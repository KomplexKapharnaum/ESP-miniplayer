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
//var MidiIN = new Emidi.Input('ESP-miniplayers', true);

var MidiIN = new Emidi.Input('Virtual Raw MIDI 0-0 16:0');
// console.log(Emidi.getInputs());

MidiIN.on('noteon', function (msg) {
  if (msg.note == 1) ESPserver.channel((msg.channel+1)).stop()    // Magic note OFF
  else ESPserver.channel((msg.channel+1)).play(msg.note)
  console.log(msg)
});

MidiIN.on('cc' , function (msg) {

  // loop
  if (msg.controller == 1) ESPserver.channel((msg.channel+1)).loop( (msg.value > 63) )

});

MidiIN.on('noteoff', function (msg) {
  //ESPserver.channel((msg.channel+1)).stop()
});

MidiIN.on('stop', function () {
  console.log('stop!')
  ESPserver.broadcast('/all/stop');
  for (var i=1; i<=16; i++) ESPserver.channel(i).stop()
});


// Start server
ESPserver.start();
