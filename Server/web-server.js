const EventEmitter = require('events');
var express = require('express');
var config = require('./config.js');

class Server extends EventEmitter {
  constructor(espserver) {
    super();

    var that = this
    this.espserver = espserver

    this.app = express();
    this.server = require('http').createServer(this.app);
    this.io = require('socket.io')(this.server);

    this.app.use(express.static(__dirname + '/node_modules'));
    this.app.get('/', function(req, res,next) {
        res.sendFile(__dirname + '/www/index.html');
    });
    this.app.get('/style.css', function(req, res,next) {
        res.sendFile(__dirname + '/www/style.css');
    });

    this.server.listen(config.webremote.port);

    this.io.on('connection', function(client) {
      console.log('Client connected...');

      // SNAPSHOT on Connect
      client.emit('snapshot', that.espserver.getSnapshot())

      // TEST TONE
      client.on('channel.test', function(chan) {
          that.espserver.channel(chan).playtest()
      })

      // STOP
      client.on('channel.stop', function(chan) {
          that.espserver.channel(chan).stop()
      })

      // STOP
      client.on('channel.emul', function(chan) {
          that.espserver.channel(chan).switchEmulator()
      })

      // LOOP
      client.on('channel.loop', function(chan) {
          that.espserver.channel(chan).switchLoop()
      })

      // EMULATOR
      client.on('emulator.stopped', function(id) {
          var emul = that.espserver.emulator(id)
          if (emul) emul.stopped()
      })

      // SYNC
      client.on('sync', function(doSync) {
          if (doSync) that.espserver.startsync()
          else that.espserver.stopsync()
      })
    })

    // BIND CLIENT EVENTS
    this.espserver.on('client.*', function(id, data) {
      //console.log(this.event, value1, value2);
      that.io.emit('updatecli', that.espserver.getNodeById(id).getSnapshot())
    })

    // BIND CHANNEL EVENTS
    this.espserver.on('channel.*', function(id, data) {
      that.io.emit('updatechan', that.espserver.channel(id).getSnapshot(false))
    })

    // BIND EMULATOR EVENTS
    this.espserver.on('channel.emulator.action.*', function(channel, id, data) {
      that.io.emit('emulator-'+id, {event: this.event.split('action.')[1], value:data})
      // console.log('emulator-'+id, {event: this.event.split('action.')[1], value:data})
    })

    // BIND SERVER EVENTS
    this.espserver.on('syncstamp', function(stamp, count) {
      //console.log(this.event, value1, value2);
      that.io.emit('syncstamp', {stamp: stamp, filecount: count})
    })
  }
}

// Export as module
exports.Server = Server;
