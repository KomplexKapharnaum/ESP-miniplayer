var dgram = require('dgram');
const os = require('os');
const ip = require('ip');
// const EventEmitter = require('events');
const EventEmitter = require('eventemitter2').EventEmitter2
const crypto = require('crypto')
var debounce = require('debounce')


const Player = require('player')
const MPlayer = require('mplayer');

const OSC = require('osc')
const getPort = require('get-port');

var PORT_SERVER = 12000;          // Working UDP port

var TIME_TICK = 100;      // Watchdog timer ms
var TIME_OFFLINE = 2000;  // Offline Time
var TIME_GONE = 6000;     // Gone Time

function log(msg) {
  console.log(msg);
}

function pad(number, length) {
    var str = '' + number;
    while (str.length < length) str = '0' + str
    return str
}

class Worker extends EventEmitter {
  constructor() {
    super({wildcard:true});

    this.isRunning  = false;
    this.timer      = null;
    this.timerate   = TIME_TICK;
    this.allowRateChange = true;
  }

  start() {
    if (this.isRunning) this.stop();
    this.isRunning  = true;
    this.emit('start');
    this.next();
  }

  next() {
    var that = this;
    this.timer = setTimeout( function() {
      if (!that.isRunning) return;
      that.emit('tick');
      that.next();
    }, that.timerate);
  }

  stop()  {
    if (this.timer != null) {
      clearTimeout(this.timer);
      this.timer = null;
    }
    this.isRunning  = false;
    this.emit('stop');
  }

  setRate(tr) {
    if (!this.allowRateChange) return;
    this.timerate = Math.round(tr);
    this.emit('fps', Math.round(100000/tr)/100);
    //log('FPS: '+ Math.round(100000/tr)/100);
    //this.timerate = 50;
  }

  lockRate(tr) {
    this.allowRateChange = true;
    this.setRate(tr);
    this.allowRateChange = false;
  }

  unlockRate() {
    this.allowRateChange = true;
  }

}

class Client extends EventEmitter {
  constructor(ip, info) {
    super({wildcard:true});
    var that = this;

    this.ip = ip;
    this.info = info;

    this.noNews = 0;
    this.udp = null;
    this.infoCounter = 0;

    this.state = 'stop'
    this.loop = false;

    this.on('stop', () => {this.state = 'stop'})
    this.on('online', () => {this.state = 'online'})
    this.on('offline', () => {this.state = 'offline'})
  }

  stop() {
    if (this.udp != null) {
      this.udp.close();
      this.udp = null;
    }
    this.infoCounter = 0;
    this.emit('stop');
  }

  update(ip, info) {

    var didChange = (this.ip != ip) || (JSON.stringify(this.info) != JSON.stringify(info))
    // re-store info
    this.ip = ip;
    this.info = info

    // first info received (since last reset)
    if (this.infoCounter == 0) this.emit('online');

    // state record
    this.infoCounter += 1;
    this.noNews = 0;

    // inform data received
    this.emit('updated', didChange);
  }

  check( ticksOffline, ticksGone) {
    this.noNews += 1;

    // state control
    if (this.noNews == ticksOffline) {
      this.infoCounter = 0;
      this.emit('offline');
    }
    if (this.noNews == ticksGone) this.stop();
  }

  getSnapshot() {
    return {
      'state': this.state,
      'info': this.info,
      'ip': this.ip
    }
  }

}

class Channel extends EventEmitter {
  constructor(serv, num) {
    super({wildcard:true});
    var that = this;

    this.num = num
    this.server = serv

    this.chan = 'c';
    if (num < 9) this.chan += '0'+(num)
    else this.chan += num

    this.media = 0
    this.doLoop = true
    this.doNoteOff = true
    this.bankDir = 1
    this.volumeCh = 127
    this.velocity = 100
    this.lastSend = ""

    this.sendGain = debounce(()=> {
      this.send("/volume/"+that.gain())
      // console.log(that.gain())
    }, 100)
  }

  send(message) {
    this.server.broadcast("/"+this.chan+message)
    this.lastSend = message
    this.emit('send', message)
  }

  play(media, velocity) {
    if (velocity === undefined) velocity = 100
    this.velocity = velocity

    this.media = media
    this.send('/play/'+pad(this.bankDir, 3)+'/'+pad(this.media, 3)+'/'+this.gain())
  }

  playtest() {
    this.velocity = 100
    this.media = 'test'
    this.send('/playtest')
  }

  stop() {
    this.media = 0
    this.send('/stop')
  }

  loop(doL) {
    if (doL !== undefined) {
      this.doLoop = doL
      if (this.doLoop) this.send("/loop/1")
      else this.send("/loop/0")
      this.emit('loop', this.doLoop)
    }
    return this.doLoop
  }

  switchLoop() {
    this.loop( !this.loop() )
  }

  noteOffStop(doO) {
    if (doO !== undefined) {
      this.doNoteOff = doO
    }
    return this.doNoteOff
  }

  bank(b) {
    if (b !== undefined) {
      this.bankDir = b
      this.emit('bank', b)
    }
    return this.bankDir
  }

  volume(v) {
    if (v !== undefined && v != this.volumeCh) {
      this.volumeCh = v
      this.sendGain();
      this.emit('volume', volume)
    }
    return this.volumeCh
  }

  gain() {
    // CONVERT 0->127 x 0->127 into 0->100
    return Math.round((this.volumeCh*this.velocity*100)/16129)
  }

  getSnapshot(withClients) {
    var snapshot = {
      'channel': this.num,
      'bank': this.bankDir,
      'loop': this.doLoop,
      'cmd': this.lastSend
    }
    if (withClients === undefined) withClients = true

    // clients info
    if (withClients) {
      snapshot['clients'] = []
      for (var node of this.server.getNodesByChannel(this.num) )
        snapshot['clients'].push(node.getSnapshot())
    }
    return snapshot
  }

}

class Server extends Worker {
  constructor() {
    super();
    var that = this;

    // Kill previous servers
    const { spawnSync} = require('child_process');
    const child = spawnSync('fuser', ['-k', PORT_SERVER+'/udp']);

    this.channels = []
    for (var i=1; i<=16; i++) {
      this.channels[i] = new Channel(this, i)
      this.channels[i].onAny( function(e,v){that.emit('channel.'+e, this.i, v) }.bind( {i: i} ))
    }

    this.clients = {};
    this.virtualDevices = [];
    this.virtualId = 1000;
    this.lastSend = "";

    this.on('start', function() {
      this.udpPort.open();
    });

    this.on('stop', function() {
      this.udpPort.close();
      for (var id in that.clients) that.clients[id].stop();
    });

    this.on('tick', function() {
      var TICK_OFFLINE = Math.round(TIME_OFFLINE/that.timerate);
      var TICK_GONE = Math.round(TIME_GONE/that.timerate);
      for (var id in that.clients) that.clients[id].check(TICK_OFFLINE, TICK_GONE);
    });


    var ifaces = os.networkInterfaces()
    this.broadcastIP = '255.255.255.255';
    for (var i in ifaces)
      for (var j in ifaces[i])
        if (ifaces[i][j]['family'] == 'IPv4' && ifaces[i][j]['address'] != '127.0.0.1' && !ifaces[i][j]['address'].startsWith('172'))
        {
          this.broadcastIP = ip.subnet(ifaces[i][j]['address'], ifaces[i][j]['netmask'])['broadcastAddress']
          // log(broadcastIP)
        }

    this.udpPort = new OSC.UDPPort({
        localAddress: "0.0.0.0",
        localPort: PORT_SERVER,
        broadcast: true,
        remotePort: 10000,
        remoteAddress: this.broadcastIP
    });

    this.udpPort.on("error", function (e) {
        //console.log('OSC Server error: ', e);
    });

    this.udpPort.on("ready", function () {
        console.log('OSC Server listening on port ' + PORT_SERVER+ ' / broadcasting on '+this.broadcastIP);
    });

    this.udpPort.on("osc", function (message, remote) {

      if (message['address'] == "/remote" ) {
        console.log('remote')
        that.broadcast(message['args'][0]);
      }

      if (message['address'] != "/iam/esp" ) return;

      // Parse info
      var ip = remote.address;
      var info = {
        id: message['args'][0],
        version: message['args'][1],
        port: message['args'][2],
        channel: message['args'][3],
        link: message['args'][4],
        sd: message['args'][5],
        media: message['args'][6],
        error: message['args'][7]
      }

      var id = info['id'];

      // Create client if new
      if (that.clients[id] == null) {
        that.clients[id] = new Client(ip, info);
        that.emit('newnode', that.clients[id]);
        that.clients[id].onAny((e,v) => {that.emit('client.'+e, id, v) })
        // console.log(ip, info)
      }

      // Update client
      that.clients[id].update(ip, info);

      // Send hello if nolink
      if (!info.link) that.broadcast("/hello");
    });

  }

  broadcast(message) {

    // Hash message with Time
    var hash = crypto.createHash('sha1').update(message+'-'+(new Date()).getTime()).digest('hex').substring(0,10);

    var oscmsg = {address: '/esp/'+hash+message}
    this.udpPort.send(oscmsg);
    this.udpPort.send(oscmsg);
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 5)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 10)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 15)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 40)

    for (var vDev of this.virtualDevices) vDev.command(oscmsg)

    this.lastSend = oscmsg
  }

  getNodeByIP(ip) {
    for (var id in this.clients)
      if (this.clients[id].ip == ip) return this.clients[id];
  }

  getNodeById(id) {
    return this.clients[id];
  }

  getNodesByChannel(ch) {
    var nodes = [];
    for (var id in this.clients)
      if (this.clients[id].info.channel == ch) nodes.push(this.clients[id]);
    return nodes;
  }

  channel(ch) {
    return this.channels[ch];
  }

  getAllNodes() {
    var nodes = [];
    for (var id in this.clients) nodes.push(this.clients[id]);
    return nodes;
  }

  getSnapshot() {
    // ON CONNECT: send Snapshot
    var snapshot = {'server': {}, 'channels':[]}

    // server info
    snapshot['server']['broadcastIP'] = this.broadcastIP

    // channels info
    for (var i=0; i<16; i++)
      snapshot['channels'][i] = this.channel(i+1).getSnapshot()

    return snapshot
  }

  createVirtualDevice(channel) {
    this.virtualId += 1
    this.virtualDevices.push( new Device( this.virtualId, channel) )
  }
}

class Device extends Worker {
  constructor(id, channel) {
    super()
    var that = this

    this.id = id
    this.channel = channel
    this.media = ""
    this.error = ""
    this.doLoop = true
    this.stopTrig = false

    this.udpPort = null
    this.player = new MPlayer();
    this.player.on('stop', () => {
      if (that.stopTrig) that.stopTrig = false
      else if (that.doLoop) that.audioplay(that.media);
    });

    this.on('start', function() {
      getPort().then(port => {
        // console.log("binding port", port)
        that.udpPort = new OSC.UDPPort({
            localPort: port,
            remotePort: PORT_SERVER,
            remoteAddress: '127.0.0.1'
        })
        that.udpPort.open()
      })
    })

    this.on('stop', function() {
      this.udpPort.close()
    })

    this.on('tick', function() {
      var oscmsg = {address: '/iam/esp', args:[
        that.id, 0, 0, that.channel, true, true, (that.media!="")?that.media:"stop", that.error
      ]}
      that.udpPort.send(oscmsg);
    })

    this.setRate(1000)
    this.start()
  }

  command(cmd) {

    var path = cmd['address'].split('/')
    if (path[1] != 'esp') return
    if (path[2] != 'manual' && path[2] == this.lastPacket) return
    else this.lastPacket = path[2]
    if (path[3] != 'all' && path[3] != 'c'+pad(this.channel,2) && path[3] != this.id) return

    if (path[4] == 'stop') this.audiostop()
    else if (path[4] == 'play') this.audioplay('/test.mp3', parseInt(path[7]))
    else if (path[4] == 'playtest') this.audioplay('/test.mp3', 100)
    else if (path[4] == 'volume') this.player.volume(parseInt(path[5]))
    else if (path[4] == 'loop') this.doLoop = parseInt(path[5]) > 0

  }

  audioplay(media, volume) {
    this.audiostop()
    this.media = media
    this.player.openFile('mp3'+media);
    this.player.volume(volume)
    this.player.play()
  }

  audiostop() {
    this.stopTrig = true
    this.player.stop()
    this.media = ""
  }

}

// Export as module
exports.Server = Server;
