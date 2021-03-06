var dgram = require('dgram');
const os = require('os');
const ip = require('ip');
// const EventEmitter = require('events');
const EventEmitter = require('eventemitter2').EventEmitter2
const crypto = require('crypto')
const glob = require("glob")
const debounce = require('debounce')

var config = require('./config.js');

const ESPemulator = require('./esp-emulator.js')
const Worker = require('./utils.js').Worker
const pad = require('./utils.js').pad

const OSC = require('osc')

var TIME_OFFLINE = 3500;  // Offline Time
var TIME_GONE = 5000;     // Gone Time
var TIME_SYNC = 3000      // Sync alert Time

function log(msg) {
  console.log(msg);
}

function oscType(a) {
  t = (typeof a)[0]
  if (t == 'n') t = 'i'
  return t
}



class Client extends EventEmitter {
  constructor(ip, info, serv) {
    super({wildcard:true});
    var that = this;

    this.server = serv
    this.ip = ip;
    this.info = info;

    this.noNews = 0;
    this.udp = null;
    this.infoCounter = 0;

    this.state = 'stop'
    this.loop = false;

    this.on('stop', () => {
      this.state = 'stop'
      this.emit('updated', true)
    })
    this.on('online', () => {
      this.state = 'online'
      this.emit('updated', true)
    })
    this.on('offline', () => {
      this.state = 'offline'
      this.emit('updated', true)
    })
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

  send(message, args, useMqtt) {
    var path = "/e"+this.info['id']+message

    if (args === true) this.lastHash = this.server.broadcastHiFi(path)
    else if (useMqtt) this.lastHash = this.server.broadcastHiFi(path, args)
    else this.lastHash = this.server.broadcastLoFi(path, args)
    this.lastSend = [path, args]
    this.emit('send', [path, args])
  }

  playtest() {
    this.send('/audio/noteon', [0, 3, 20], true)
  }

  stopPlayback() {
    this.send('/audio/stop', true)
  }

  reset() {
    this.send("/reset")
    this.send("/reset", undefined, true)
    this.emit('reset')
  }

  shutdown() {
    this.send("/shutdown")
    this.send("/shutdown", undefined, true)
    this.emit('shutdown')
  }

  setChannel(chan) {
    this.send("/channel", [parseInt(chan)], true)
  }

}

class Channel extends EventEmitter {
  constructor(serv, num) {
    super({wildcard:true});
    var that = this;

    this.num = num
    this.server = serv

    this.chan = 'c';
    // if (num <= 9) this.chan += '0'+(num)
    // else
    this.chan += num

    this.note = 0
    this.doLoop = config.player.loop
    this.doNoteOff = true
    this.bankDir = 1
    this.volumeCh = 127
    if (this.num == 16) this.volumeCh = 64
    this.velocity = 100
    this.leds = [0,0,0]
    this.lastSend = ""
    this.lastHash = ""

    this.emulator = null

    /*this.sendGain = debounce(()=> {
      this.send("/volume/"+that.gain())
      // console.log(that.gain())
    }, 100)*/
  }

  send(message, args, useMqtt) {
    var path;
    if (this.num < 16) path = "/"+this.chan+message
    else path = "/all"+message

    if (args === true) this.lastHash = this.server.broadcastHiFi(path)
    else if (useMqtt) this.lastHash = this.server.broadcastHiFi(path, args)
    else this.lastHash = this.server.broadcastLoFi(path, args)

    this.lastSend = [path, args]
    this.emit('send', [path, args])
    console.log([path, args])
  }

  ledall(red, green, blue) {
    this.leds = [red, green, blue]
    this.sendledall()
  }
  ledallwhite(val) {
    this.leds = [val, val, Math.round(val*80/100)]
    this.sendledall()
  }
  ledallred(val) {
    this.leds[0] = val
    this.sendledall()
  }
  ledallgreen(val) {
    this.leds[1] = val
    this.sendledall()
  }
  ledallblue(val) {
    this.leds[2] = val
    this.sendledall()
  }
  sendledall() {
    this.send('/leds/all', [this.leds[0],this.leds[1],this.leds[2]], true);
  }

  play(note, velocity) {
    if (velocity === undefined) velocity = 100
    this.velocity = velocity
    this.note = note
    this.send('/audio/noteon', [this.bankDir, this.note, this.gain()], true)
  }

  noteoff(note) {
    this.send('/audio/noteoff', [this.note], true)
  }

  playtest() {
    this.send('/audio/noteon', [0, 3, 20], true)
  }

  stop() {
    this.note = 0
    this.send('/audio/stop', true)
  }

  loop(doL) {
    if (doL !== undefined) {
      this.doLoop = doL
      if (this.doLoop) this.send("/audio/loop", [1], true)
      else this.send("/audio/loop", [0], true)
      this.emit('loop', this.doLoop)
    }
    return this.doLoop
  }

  switchLoop() {
    this.loop( !this.loop() )
  }

  sendGain() {
    if (this.num < 16) this.send("/audio/volume", [this.gain()])
    else {
      for (var ch in this.server.channels)
        if (this.server.channels[ch].num < 16) this.server.channels[ch].sendGain()
      }
  }

  noteOffStop(doO) {
    if (doO !== undefined) {
      this.doNoteOff = doO
    }
    return this.doNoteOff
  }

  reset() {
    this.send("/reset")
    this.send("/reset", undefined, true)
    this.emit('reset')
  }

  shutdown() {
    this.send("/shutdown")
    this.send("/shutdown", undefined, true)
    this.emit('shutdown')
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
      this.emit('volume', v)
    }
    return this.volumeCh
  }

  gain() {
    // CONVERT 0->127 (volume channel CC7) x 0->127 (velocity note) x 0->127 (master volume cc7 ch16) into 0->100
    return Math.round((this.volumeCh*this.velocity*this.server.master().volumeCh*100)/(2048383))
  }

  getSnapshot(withClients) {
    var snapshot = {
      'channel': this.num,
      'bank': this.bankDir,
      'loop': this.doLoop,
      'cmd': this.lastSend,
      'emul': (this.emulator != null)
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

  switchEmulator() {
    if (this.emulator == null) {
      var that = this;
      this.emulator = new ESPemulator.Device(this)
      this.emulator.onAny((e,v) => {
        that.emit('emulator.'+e, that.emulator.id, v)
      })
    }
    else {
      this.emulator.stop()
      this.emulator = null
    }
    this.emit('emul', (this.emulator == null))
  }

}

class Server extends Worker {
  constructor(syncserver) {
    super(100);
    var that = this;

    this.syncserver = syncserver
    this.doSync = false
    this.tickSync = 0

    // Kill previous servers
    const { spawnSync} = require('child_process');
    spawnSync('fuser', ['-k', config.espserver.portin+'/udp']);
    spawnSync('fuser', ['-k', config.webremote.port+'/tcp']);
    spawnSync('fuser', ['-k', config.oscremote.port+'/udp']);

    this.channels = []
    for (var i=1; i<=16; i++) {
      this.channels[i] = new Channel(this, i)
      this.channels[i].onAny( function(e,v,w){that.emit('channel.'+e, this.i, v, w) }.bind( {i: i} ))
    }

    this.clients = {};
    this.lastSend = "";

    this.on('start', function() {
      this.udpPort.open();
    });

    this.on('stop', function() {
      this.udpPort.close();
      for (var cl in that.clients) that.clients[cl].stop();
      for (var ch in that.channels) if (that.channels[ch].emulator) that.channels[ch].emulator.stop();
    });

    this.on('tick', function() {
      var TICK_OFFLINE = Math.round(TIME_OFFLINE/that.timerate);
      var TICK_GONE = Math.round(TIME_GONE/that.timerate);
      for (var id in that.clients) that.clients[id].check(TICK_OFFLINE, TICK_GONE);

      if (that.doSync && that.syncserver) {
        that.tickSync += 1
        var TICK_SYNC = Math.round(TIME_SYNC/that.timerate);
        if (that.tickSync > TICK_SYNC) {
          that.tickSync = 0
          that.broadcastLoFi('/all/sync', [that.syncserver.syncStamp()])
          that.emit('syncstamp', that.syncserver.syncStamp(), that.syncserver.fileCount())
        }
      }
    });


    var ifaces = os.networkInterfaces()
    this.broadcastIP = '255.255.255.255';
    for (var i in ifaces)
      for (var j in ifaces[i])
        if (ifaces[i][j]['family'] == 'IPv4' && ifaces[i][j]['address'] != '127.0.0.1' && !ifaces[i][j]['address'].startsWith('172'))
        {
          this.ip = ifaces[i][j]['address']
          this.broadcastIP = ip.subnet(ifaces[i][j]['address'], ifaces[i][j]['netmask'])['broadcastAddress']
          // log(broadcastIP)
        }

    this.udpPort = new OSC.UDPPort({
        localAddress: "0.0.0.0",
        localPort: config.espserver.portin,
        broadcast: true,
        remotePort: config.espserver.portout,
        remoteAddress: this.broadcastIP
    });

    this.udpPort.on("error", function (e) {
        //console.log('OSC Server error: ', e);
    });

    this.udpPort.on("ready", function () {
        console.log('OSC Server listening on port ' + config.espserver.portin+ ' / broadcasting on '+this.broadcastIP);
    });

    this.udpPort.on("osc", function (message, remote) {

      // console.log(message)

      if (message['address'] == "/remote" ) {
        console.log('remote')
        that.broadcastLoFi(message['args'][0]);
      }

      if (message['address'] != "/status" ) return;

      // Parse info
      var ip = remote.address;
      var info = {
        id: message['args'][0],
        channel: message['args'][1],
        version: message['args'][2],
        bssid: message['args'][3],
        ip: message['args'][4],
        rssi: message['args'][5],
        link: message['args'][6],
        battery: message['args'][7],
        sd: message['args'][8],
        media: message['args'][9],
        error: message['args'][10],
        bank: message['args'][11],
        sync: message['args'][12],
        syncerror: message['args'][13]
      }

      var id = info['id'];

      // Create client if new
      if (that.clients[id] == null) {
        that.clients[id] = new Client(ip, info, that);
        that.emit('newnode', that.clients[id]);
        that.clients[id].onAny((e,v) => {that.emit('client.'+e, id, v) })
        // console.log(ip, info)
      }

      // Update client
      that.clients[id].update(ip, info);

      // Send ping if nolink
      if (!info.link) that.broadcastLoFi("/ping");
    });
  }

  broadcastLoFi(message, args) {
    return this.oscBroadcaster(message, args, false)
  }

  broadcastHiFi(message, args) {
    return this.oscBroadcaster(message, args, true)
  }

  oscBroadcaster(message, args, useMqtt) {

    var hash = crypto.createHash('sha1').update(message+'-'+(new Date()).getTime()).digest('hex').substring(0,10);

    // ONE SEND
    var oscmsg = {address: message}
    if (args !== undefined) {
      oscmsg['args'] = []
      for (var a of args) {
        oscmsg['args'].push({
          type: oscType(a),
          value: a
        })
      }
    }

    if (useMqtt) {
      this.udpPort.send(oscmsg, config.espserver.osc2mqtt.server, config.espserver.osc2mqtt.port);
      console.log('send HiFi (udp via mqtt)', oscmsg)
    }
    else {
      this.udpPort.send(oscmsg);
      console.log('send LoFi (udp broadcast)', oscmsg)
    }

    for (var ch in this.channels)
      if (this.channels[ch].emulator) this.channels[ch].emulator.command(oscmsg)

    this.lastSend = oscmsg

    return hash
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

  emulator(id) {
    for (var ch in this.channels)
      if (this.channels[ch].emulator && this.channels[ch].emulator.id == id)
        return this.channels[ch].emulator
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
    snapshot['server']['ip'] = this.ip
    snapshot['server']['broadcastIP'] = this.broadcastIP
    snapshot['server']['fileCount'] = this.syncserver.fileCount()

    // channels info
    for (var i=0; i<16; i++)
      snapshot['channels'][i] = this.channel(i+1).getSnapshot()

    return snapshot
  }

  startsync() {
    this.doSync = true
  }

  stopsync() {
    this.doSync = false
  }

  ledall(red, green, blue) {
    this.master().ledall(red, green, blue)
  }
  ledallwhite(white) {
    this.master().ledallwhite(white)
  }
  ledallred(red) {
    this.master().ledallred(red)
  }
  ledallgreen(green) {
    this.master().ledallgreen(green)
  }
  ledallblue(blue) {
    this.master().ledallblue(blue)
  }
  rpm(rpm) {
    console.log("server rpm ",rpm)
    this.emit('rpm', rpm);
  }

  master() {
    return this.channel(16)
  }
}

// Export as module
exports.Server = Server;
