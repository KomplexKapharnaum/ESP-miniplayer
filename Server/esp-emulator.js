const Worker = require('./utils.js').Worker
const getPort = require('get-port')
const pad = require('./utils.js').pad
const OSC = require('osc')
var config = require('./config.js');
const MPlayer = require('mplayer');
MPlayer.prototype.quit = function() {
    this.player.cmd('quit');
}
MPlayer.prototype.loop = function(doLoop) {
    this.player.cmd('set_property', ['loop', (doLoop?0:-1)])
}

class Device extends Worker {
  constructor(id, channel, doLoop) {
    super(1000)
    var that = this

    this.id = id
    this.channel = channel
    this.media = ""
    this.error = ""
    this.doLoop = doLoop
    this.stopTrig = false

    this.udpPort = null
    this.player = null
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

      that.player = new MPlayer();
      that.player.on('status', (st)=>that.status=st);
      that.player.loop(that.doLoop)
      that.player.on('stop', () => {
        that.startCount -= 1
        if (that.startCount == 0) that.media = ""
      });

    })

    this.on('stop', function() {
      this.udpPort.close()
      this.player.quit()
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

    if (path[4] == 'stop') this.audiostop()
    else if (path[4] == 'play') this.audioplay(
          glob.sync(config.basepath.mp3+"/"+parseInt(path[5]).pad(3)+"/"+parseInt(path[6]).pad(3)+"*.mp3"),
          parseInt(path[7]))
    else if (path[4] == 'playtest') this.audioplay('/test.mp3', 100)
    else if (path[4] == 'volume') this.player.volume(parseInt(path[5]))
    else if (path[4] == 'loop') {
      this.doLoop = parseInt(path[5]) > 0
      this.player.loop(this.doLoop)
    }

  }

  audioplay(media, volume, skipStop) {
    this.media = media
    this.player.openFile('../mp3'+media);
    this.player.volume(volume)
    this.player.loop(this.doLoop)
    this.player.play()
    this.startCount += 1
  }

  audiostop() {
    this.player.stop()
    this.media = ""
  }

}

// Export as module
exports.Device = Device;
