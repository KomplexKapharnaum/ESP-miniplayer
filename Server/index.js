// Config
var config = require('./config.js');

// Utils
const Utils = require('./utils.js')
Utils.cls()

// Create file sync server
const FileSync = require('./esp-fileserver.js')
FileSync.start()

// Load Hnode library
const ESPlib = require('./esp-server.js')
var ESPserver = new ESPlib.Server(FileSync)

// Create MIDI iface
var isLinux = /^linux/.test(process.platform);
if (!isLinux) {
  const MidiBridge = require('./midi-bridge.js')
  var MIDIiface = new MidiBridge.MidiInterface(ESPserver);
}
// Console display
//setInterval(()=>Utils.consoledisp(ESPserver), 500)

// Start web interface
const WEBlib = require('./web-server.js')
var WEBserver = new WEBlib.Server(ESPserver)

// Start OSC interface
const OSCbridge = require('./osc-bridge.js')
var OSCiface = new OSCbridge.OscInterface(ESPserver, MIDIiface)

// Electron interface
const {app, BrowserWindow} = require('electron')
const path = require('path')
const url = require('url')
let win
function createWindow () {
    // Create the browser window.
    win = new BrowserWindow({width: 1680, height: 600})

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
