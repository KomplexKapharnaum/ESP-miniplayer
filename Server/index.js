// Config
var config = require('./config.js');

// Utils
const Utils = require('./utils.js')
Utils.cls()

// Load Hnode library
const ESPlib = require('./esp-server.js')
var ESPserver = new ESPlib.Server()

// Console display
//setInterval(()=>Utils.consoledisp(ESPserver), 500)

// Create MIDI iface
const MidiBridge = require('./midi-bridge.js')
var MIDIiface = new MidiBridge.MidiInterface(ESPserver);

// Create file sync server
const FileSync = require('./esp-fileserver.js')
FileSync.start()

// Start web interface
const WEBlib = require('./web-server.js')
var WEBserver = new WEBlib.Server(ESPserver)

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
      pathname: 'localhost:'+config.webremote.port+'/',
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
