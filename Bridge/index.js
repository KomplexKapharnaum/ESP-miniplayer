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


var Emidi = require('easymidi');
var MidiIN = new Emidi.Input('Virtual Raw MIDI 1-0 20:0');

MidiIN.on('noteon', function (msg) {
  var chan = 'c';
  if (msg.channel < 9) chan += '0'+(msg.channel+1)
  else chan += (msg.channel+1)

  //ESPserver.broadcast('/c02/volume/'+Math.floor(msg.velocity/10))
  ESPserver.broadcast('/'+chan+'/play/'+msg.note+'.mp3')
});

MidiIN.on('noteoff', function (msg) {
  var chan = 'c';
  if (msg.channel < 9) chan += '0'+(msg.channel+1)
  else chan += (msg.channel+1)
  
  ESPserver.broadcast('/'+chan+'/stop')
});


// Start server
ESPserver.start();
