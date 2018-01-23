// Utils
const Utils = require('./utils.js')
Utils.cls()

// Load Hnode library
const ESPlib = require('./esp-server.js')
var ESPserver = new ESPlib.Server();


// Event: when a new node is detected
ESPserver.on('newnode', function(node) {

  // Event: when the node goes online
  node.on('online', function(node){ console.log('online '+this.ip, this.info) });

  // Event: when the node goes offline
  node.on('offline', function(node){ console.log('offline '+this.ip+' '+this.info.id) });

  // Event: when the node stop
  node.on('stop', function(node){ console.log('stop '+this.ip+' '+this.info.id) });

  // Event: when info
  // node.on('updated', function(node){ console.log('received ',this.info) });

});


// Create MIDI iface
const MidiBridge = require('./midi-bridge.js')
var MIDIiface = new MidiBridge.MidiInterface(ESPserver);



// Start server
ESPserver.start();
