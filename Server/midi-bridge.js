var isLinux = /^linux/.test(process.platform);

var Emidi = require('easymidi');

class MidiInterface {

  constructor(ESPserver) {

    var that = this
    this.ESPserver = ESPserver

    if (isLinux) this.MidiIN = new Emidi.Input('Virtual Raw MIDI 0-0 16:0');
    else this.MidiIN = new Emidi.Input('ESP-miniplayers', true);

    // console.log(Emidi.getInputs());

    // NoteON :: PLAY - STOP
    this.MidiIN.on('noteon', (msg) => {
      msg.note += 1
      msg.channel += 1

      if (msg.note == 128) that.ESPserver.channel(msg.channel).stop()    // Magic note OFF
      else that.ESPserver.channel(msg.channel).play(msg.note, msg.velocity)

      console.log(msg)
    });

    // Control Changes
    this.MidiIN.on('cc' , (msg) => {
      msg.channel += 1
      console.log(msg)

      // loop
      if (msg.controller == 1) that.ESPserver.channel(msg.channel).loop( (msg.value > 63) )

      // noteOFF enable
      if (msg.controller == 2) that.ESPserver.channel(msg.channel).noteOffStop( (msg.value < 63) )

      // volume
      else if (msg.controller == 7) that.ESPserver.channel(msg.channel).volume( msg.value )

      // cc bank
      else if (msg.controller == 118) {
        that.ESPserver.channel(msg.channel).bank( msg.value )
      }

      // stop all
      else if (msg.controller == 119) {
        that.ESPserver.broadcast('/all/stop')
        for (var i=1; i<=16; i++) that.ESPserver.channel(i).stop()
      }
    });

    // NoteOFF :: STOP
    this.MidiIN.on('noteoff', (msg) => {
      if (that.ESPserver.channel((msg.channel+1)).noteOffStop())
        that.ESPserver.channel((msg.channel+1)).stop()
    });

    // Teleco
    this.MidiIN.on('stop', function () {
      console.log('stop!')
      // that.ESPserver.broadcast('/all/stop');
      // for (var i=1; i<=16; i++) that.ESPserver.channel(i).stop()
    });

    // Program Change
    this.MidiIN.on('program', (msg) => {
      console.log(msg)
      that.ESPserver.channel((msg.channel+1)).bank(msg.number+1)
    })

  }

}


// Export as module
exports.MidiInterface = MidiInterface;
