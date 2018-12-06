const OSC = require('osc')
var config = require('./config.js');

class OscInterface {

  constructor(ESPserver, MIDIiface) {

    var that = this
    this.ESPserver = ESPserver
    this.MIDIiface = MIDIiface

    this.udpPort = new OSC.UDPPort({
        localAddress: "0.0.0.0",
        localPort: config.oscremote.port
    });

    this.udpPort.on("error", function (e) {
        console.log('OSC Bridge error: ', e);
    });

    this.udpPort.on("ready", function () {
        console.log('OSC Bridge listening on port ' + config.oscremote.port);
    });

    this.udpPort.on("osc", function (message, remote) {

      console.log("osc bridge received:", message['address'], message['args'], "from", remote.address, remote.port)

      if (message['address'] == "/gegenrpm") {
        if (that.MIDIiface) that.MIDIiface.MidiOUT.send('cc', { controller: 23, value: Math.min(message['args'][0]*2, 127), channel: 15 })
        that.ESPserver.rpm(message['args'][0])
      }

    });

    this.udpPort.open();
  }
}


// Export as module
exports.OscInterface = OscInterface;
