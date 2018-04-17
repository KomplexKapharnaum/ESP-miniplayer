const Worker = require('./utils.js').Worker
const getPort = require('get-port')
const pad = require('./utils.js').pad
const OSC = require('osc')
const glob = require("glob")
var config = require('./config.js');

/*const MPlayer = require('mplayer');
MPlayer.prototype.quit = function() {
    this.player.instance.removeAllListeners('exit')
    this.player.cmd('quit');
}
MPlayer.prototype.loop = function(doLoop) {
    this.player.cmd('set_property', ['loop', (doLoop?0:-1)])
}*/

class Device extends Worker {
  constructor(channel) {
    super(1000)
    var that = this

    this.channel = channel.num
    this.id = 1000+channel.num
    this.media = ""
    this.error = ""
    this.doLoop = channel.loop()
    this.stopTrig = false

    this.udpPort = null
    this.startCount = 0

    this.on('start', function() {
      getPort().then(port => {
        // console.log("binding port", port)
        that.udpPort = new OSC.UDPPort({
            localPort: port,
            remotePort: config.espserver.port,
            remoteAddress: '127.0.0.1'
        })
        that.udpPort.open()
      })
    })

    this.on('stop', function() {
      // this.mplayer.quit()
      // console.log('exited player')
    })

    this.on('tick', function() {
      var oscmsg = {address: '/iam/esp', args:[
        that.id, 0, 0, that.channel, true, true, 0, (that.media!="")?that.media:"stop", that.error
      ]}
      if (that.udpPort != null)
        that.udpPort.send(oscmsg);
    })

    this.start()
  }

  command(cmd) {

    var path = cmd['address'].split('/')
    if (path[1] != 'esp') return
    if (path[2] != 'manual' && path[2] == this.lastPacket) return
    else this.lastPacket = path[2]
    if (path[3] != 'all' && path[3] != 'c'+pad(this.channel,2) && path[3] != this.id) return

    if (path[4] == 'stop') this.emit('action.stop')
    else if (path[4] == 'play') {
      this.media = glob.sync(config.basepath.mp3+"/"+parseInt(path[5]).pad(3)+"/"+parseInt(path[6]).pad(3)+"*.mp3").split(config.basepath.mp3)[1]
      this.emit('action.play', {media:this.media, volume:parseInt(path[7])})
    }
    else if (path[4] == 'playtest') {
      this.media = '/test.mp3'
      this.emit('action.play', {media:this.media, volume:100})
    }
    else if (path[4] == 'volume') this.emit('action.volume', parseInt(path[5])) //this.mplayer.volume(parseInt(path[5]))
    else if (path[4] == 'loop') this.emit('action.loop', (parseInt(path[5]) > 0))

  }

  stopped() {
    this.media = ""
  }

}

// Export as module
exports.Device = Device;
