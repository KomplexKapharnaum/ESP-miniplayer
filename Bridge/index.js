// Utils
var colors = require('colors');
const Utils = require('./utils.js')
Utils.cls()

// Load Hnode library
const ESPlib = require('./esp-server.js')
var ESPserver = new ESPlib.Server();


// Event: when a new node is detected
ESPserver.on('newnode', function(node) {

  // Event: when the node goes online
  //node.on('online', function(node){ console.log('online '+this.ip, this.info) });

  // Event: when the node goes offline
  //node.on('offline', function(node){ console.log('offline '+this.ip+' '+this.info.id) });

  // Event: when the node stop
  //node.on('stop', function(node){ console.log('stop '+this.ip+' '+this.info.id) });

  // Event: when info
  //node.on('updated', function(node){ console.log('received ',this.info) });

});

function display() {
  Utils.cls()
  console.log("ESP-controller".bold)
  console.log("Broadcasting on "+ESPserver.broadcastIP)
  console.log()

  for (var i=1; i<=16; i++) {
    var nodes = ESPserver.getNodesByChannel(i)
    if (nodes.length > 0) {
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
}

setInterval(display, 500)

// Create MIDI iface
const MidiBridge = require('./midi-bridge.js')
var MIDIiface = new MidiBridge.MidiInterface(ESPserver);



// Start server
ESPserver.start();
