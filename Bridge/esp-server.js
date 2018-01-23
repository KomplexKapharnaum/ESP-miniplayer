var dgram = require('dgram');
const os = require('os');
const ip = require('ip');
const EventEmitter = require('events');

const OSC = require('osc');

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
    super();

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
    super();
    var that = this;

    this.ip = ip;
    this.info = info;

    this.noNews = 0;
    this.udp = null;
    this.infoCounter = 0;

    this.media = 0;
    this.loop = false;
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

    // re-store info
    this.ip = ip;
    this.info = info

    // first info received (since last reset)
    if (this.infoCounter == 0) this.emit('online');

    // state record
    this.infoCounter += 1;
    this.noNews = 0;

    // inform data received
    this.emit('updated');
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

  send() {
    var that = this;
    if (this.udp == null) this.udp = dgram.createSocket('udp4');
    if (this.payload != null)
      this.udp.send(this.payload, 0, this.payload.length, this.port, this.ip, function(err, bytes) {
          if (err) {
            if (err.code == 'ENETUNREACH' || err.code == 'EADDRNOTAVAIL') {
              console.log('\nWarning: the server lost connection to the network');
              that.stop();
            }
            else throw err;
          }
          else that.emit('sent', this.payload);
      });
  }

}

class Channel {
  constructor(serv, num) {
    this.num = num
    this.server = serv

    this.chan = 'c';
    if (num < 9) this.chan += '0'+(num)
    else this.chan += num

    this.media = 0
    this.volume = 100
    this.doLoop = false
    this.bankDir = 0
  }

  send(message) {
    this.server.broadcast("/"+this.chan+message)
  }

  play(media, volume) {
    if (volume === undefined) volume = 100

    this.volume = volume
    this.media = media
    this.send('/play/'+pad(this.bankDir, 3)+'/'+pad(this.media, 3)+'/'+this.volume)
  }

  stop() {
    this.media = 0
    this.send('/stop')
  }

  loop(doL) {
    this.doLoop = doL
    if (this.doLoop) this.send("/loop/1")
    else this.send("/loop/0")
  }

  bank(b) {
    this.bankDir = b
  }

}

class Server extends Worker {
  constructor() {
    super();
    var that = this;

    this.channels = []
    for (var i=1; i<=16; i++) this.channels[i] = new Channel(this, i)

    this.clients = {};
    this.countCmd = 0;

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
    var broadcastIP = '255.255.255.255';
    for (var i in ifaces)
      for (var j in ifaces[i])
        if (ifaces[i][j]['family'] == 'IPv4' && ifaces[i][j]['address'] != '127.0.0.1' && !ifaces[i][j]['address'].startsWith('172'))
        {
          broadcastIP = ip.subnet(ifaces[i][j]['address'], ifaces[i][j]['netmask'])['broadcastAddress']
          // log(broadcastIP)
        }

    this.udpPort = new OSC.UDPPort({
        localAddress: "0.0.0.0",
        localPort: PORT_SERVER,
        broadcast: true,
        remotePort: 10000,
        remoteAddress: broadcastIP
    });

    this.udpPort.on("error", function (e) {
        //console.log('OSC Server error: ', e);
    });

    this.udpPort.on("ready", function () {
        console.log('OSC Server listening on port ' + PORT_SERVER+ ' / broadcasting on '+broadcastIP);
    });

    this.udpPort.on("osc", function (message, remote) {
      if (message['address'] != "/iam/esp" ) return;

      // Parse info
      var ip = remote.address;
      var info = {
        id: message['args'][0],
        port: message['args'][1],
        channel: message['args'][2],
        link: message['args'][3],
        media: message['args'][4]
      }

      var id = info['id'];

      // Create client if new
      if (that.clients[id] == null) {
        that.clients[id] = new Client(ip, info);
        that.emit('newnode', that.clients[id]);
        // console.log(ip, info)
      }

      // Update client
      that.clients[id].update(ip, info);
    });

  }

  broadcast(message) {
    this.countCmd+=1

    var oscmsg = {address: '/esp/'+this.countCmd+message}
    this.udpPort.send(oscmsg);
    this.udpPort.send(oscmsg);
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 5)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 10)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 15)
    setTimeout(() => {  this.udpPort.send(oscmsg); }, 40)

    // console.log(oscmsg)
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

}

// Export as module
exports.Server = Server;
