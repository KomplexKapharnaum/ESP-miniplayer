// Utils
var colors = require('colors')
const Utils = require('./utils.js')
Utils.cls()

// Load Hnode library
const ESPlib = require('./esp-server.js')
var ESPserver = new ESPlib.Server()


function display() {
  Utils.cls()
  console.log("ESP-controller".bold)
  var br = "Broadcasting on "+ESPserver.broadcastIP
  if (ESPserver.broadcastIP.startsWith('2.0')) console.log(br.green)
  else console.log(br.red)
  console.log()

  var noDevices = true
  for (var i=1; i<=16; i++) {
    var nodes = ESPserver.getNodesByChannel(i)
    if (nodes.length > 0) {
      noDevices = false
      var c = ("Channel "+i).bold+"\n"
      c += " "+("bank: "+ESPserver.channel(i).bankDir).blue
      c += " / "+("loop: "+ESPserver.channel(i).doLoop).cyan
      console.log(c)


      for (var k in nodes) {
        var n = " "+nodes[k].ip+" - "+nodes[k].state;

        if (nodes[k].state == "online") n = n.green
        else if (nodes[k].state == "offline") n = n.yellow
        else n = n.red

        n += " - version: "+nodes[k].info.version
        n += " - link: "+nodes[k].info.link
        n += " - sd: "+(nodes[k].info.sd ? "OK": "ERROR")
        n += " - media: "+nodes[k].info.media
        if (nodes[k].info.error != "") n += " - error: "+nodes[k].info.error



        console.log(n)
      }
      console.log()
    }
  }

  if (noDevices) {
    console.log("No device detected...".bold)
    console.log("you might be on the wrong network !".red)
    console.log("If you do change the network, please restart the plugin.")
  }
  else console.log('Last send: ',ESPserver.lastSend)
}

//setInterval(display, 500)

// Create MIDI iface
const MidiBridge = require('./midi-bridge.js')
var MIDIiface = new MidiBridge.MidiInterface(ESPserver);



// Start web interface
const WEBlib = require('./web-server.js')
var WEBserver = new WEBlib.Server(8088, ESPserver)

// Electron interface
const {app, BrowserWindow} = require('electron')
const path = require('path')
const url = require('url')
let win
function createWindow () {
    // Create the browser window.
    win = new BrowserWindow({width: 1590, height: 550})

    // and load the index.html of the app.
    win.loadURL(url.format({
      pathname: 'localhost:8088/',
      protocol: 'http:',
      slashes: true
    }))

    // Open the DevTools.
    //win.webContents.openDevTools()

    // Emitted when the window is closed.
    win.on('closed', () => {
      // Dereference the window object, usually you would store windows
      // in an array if your app supports multi windows, this is the time
      // when you should delete the corresponding element.
      win = null
    })
  }

  app.on('ready', createWindow)
  app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
      ESPserver.stop();
      app.quit()
      process.exit()
    }
  })
  app.on('activate', () => {
    if (win === null) createWindow()
  })

//const opn = require('opn');
//opn('http://localhost:8088/');


// Start server
ESPserver.start();
