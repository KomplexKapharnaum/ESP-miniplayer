var isLinux = /^linux/.test(process.platform);

var Emidi = require('easymidi');

class MidiInterface {

  constructor(ESPserver) {

    var that = this
    this.ESPserver = ESPserver

    if (isLinux) {
      this.MidiIN = new Emidi.Input('Virtual Raw MIDI 0-0 16:0');
      this.MidiOUT = new Emidi.Output('Virtual Raw MIDI 0-0 16:0');
    }
    else {
      this.MidiIN = new Emidi.Input('ESP-miniplayers', true);
      this.MidiOUT = new Emidi.Output('ESP-miniplayers', true);
    }
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
      // console.log(msg)

      // loop
      if (msg.controller == 1) that.ESPserver.channel(msg.channel).loop( (msg.value > 63) )

      // noteOFF enable
      if (msg.controller == 2) that.ESPserver.channel(msg.channel).noteOffStop( (msg.value < 63) )

      // leds
      if (msg.controller == 23) that.ESPserver.channel(msg.channel).ledallwhite( msg.value*2 )
      if (msg.controller == 24) that.ESPserver.channel(msg.channel).ledallred( msg.value*2 )
      if (msg.controller == 25) that.ESPserver.channel(msg.channel).ledallgreen( msg.value*2 )
      if (msg.controller == 26) that.ESPserver.channel(msg.channel).ledallblue( msg.value*2 )

      // volume
      else if (msg.controller == 7) that.ESPserver.channel(msg.channel).volume( msg.value )

      // cc bank
      else if (msg.controller == 118) {
        that.ESPserver.channel(msg.channel).bank( msg.value )
      }

      // stop all
      else if (msg.controller == 119) {
        that.ESPserver.broadcastHiFi('/all/stop')
        for (var i=1; i<=16; i++) that.ESPserver.channel(i).stop()
      }
    });

    // NoteOFF :: STOP
    this.MidiIN.on('noteoff', (msg) => {
      if (that.ESPserver.channel((msg.channel+1)).noteOffStop())
        //that.ESPserver.channel((msg.channel+1)).stop()
        that.ESPserver.channel((msg.channel+1)).noteoff(msg.note)
    });

    // Teleco
    this.MidiIN.on('stop', function () {
      console.log('stop!')
      // that.ESPserver.broadcastHiFi('/all/stop');
      // for (var i=1; i<=16; i++) that.ESPserver.channel(i).stop()
    });

    // Program Change
    this.MidiIN.on('program', (msg) => {
      console.log('BANK CHANGE', msg.number+1, msg)
      that.ESPserver.channel((msg.channel+1)).bank(msg.number+1)
    })

  }

}


// Export as module
exports.MidiInterface = MidiInterface;
