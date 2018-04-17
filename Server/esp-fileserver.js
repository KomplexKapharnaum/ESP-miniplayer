// File Server for sync
const glob = require("glob")
const fs = require("fs")
const pad = require('./utils.js').pad
var config = require('./config.js');
const resolve = require('path').resolve
var chokidar = require('chokidar');


const express = require('express')
const app = express()

var stamp = Math.floor(Date.now() / 1000)
var filecount = glob.sync(config.basepath.mp3+'/*/*.mp3').length

// Watch directory change
var changeStamp = null
chokidar.watch(resolve(config.basepath.mp3), {
    ignored: /(^|[\/\\])\../,
    depth: 3,
    ignoreInitial: true,
    awaitWriteFinish: {
      stabilityThreshold: 2000,
      pollInterval: 100
    }
  }).on('all', (event, path) => {

    console.log(event, path);
    clearTimeout(changeStamp)
    changeStamp = setTimeout(()=>{
      stamp = Math.floor(Date.now() / 1000);
      filecount = glob.sync(config.basepath.mp3+'/*/*.mp3').length
      console.log("stamp updated")
    }, 5000)
});

app.use(function(req, res, next) {
  res.header("Access-Control-Allow-Origin", "*");
  res.header("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
  next();
});


app.get('/', (req, res) => res.send('Hello World!'))

app.get('/listbank/*', (req, res) => {
  var ans = ""
  bank = parseInt(req.params[0])
  for (var note=0; note<128; note++)
  {
    ans += pad(bank,3)+" "+pad(note,3)+" "
    var path = glob.sync(config.basepath.mp3+"/"+pad(bank,3)+"/"+pad(note,3)+"*.mp3")
    if (path.length > 0) ans += pad(fs.statSync(path[0]).size,10)+" "+path[0].substring(config.basepath.mp3.length)
    else ans += pad(0,10) //+" "+"               "
    ans += "\n"
  }
  // console.log(ans)
  res.send(ans)
});

app.get('/get/*', (req, res) => {
  var path = resolve(config.basepath.mp3+"/"+req.params[0])
  // console.log(path)
  if (fs.existsSync(path)) {
    var stat = fs.statSync(path)
    res.sendFile(path)
  }
  else
  res.status(404).send("Sorry can't find that: "+path)
});

var shouldStop = false
var isRunning = false

exports.start = function() {
  shouldStop = false
  app.listen(config.filesync.port, () => {
    console.log('FileSync: server is listening on '+config.filesync.port)
    isRunning = true
    if (shouldStop) app.close(()=>{
      isRunning = false
    })
  })
}

exports.stop = function() {
  if (isRunning) app.close(()=>{
    isRunning = false
  })
  else shouldStop = true
}

exports.syncStamp = function() {
  return stamp
}

exports.fileCount = function() {
  return filecount
}
