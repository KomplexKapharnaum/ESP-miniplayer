
exports.pad = function(number, length) {
    var str = '' + number;
    while (str.length < length) str = '0' + str
    return str
}

//////////////////////

const EventEmitter = require('eventemitter2').EventEmitter2

class Worker extends EventEmitter {
  constructor(timerate) {
    super({wildcard:true});

    this.isRunning  = false;
    this.timer      = null;
    this.timerate   = timerate;
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

exports.Worker = Worker

///////////////////////

function cls() {
  process.stdout.write('\033c');
}

exports.cls = cls

/////////////////////////
var colors = require('colors')

function display(ESPserver) {
  cls()
  console.log("ESP-controller".bold)
  var br = "Broadcasting on "+ESPserver.broadcastIP
  if (ESPserver.broadcastIP.startsWith('2.0')) console.log(br.green)
  else console.log(br.red)
  console.log()

  var noDevices = true
  for (var i=1; i<=16; i++) {
    var nodes = ESPserver.getNodesByChannel(i)
    if (nodes.length > 0) {
      noDevices = false
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

  if (noDevices) {
    console.log("No device detected...".bold)
    console.log("you might be on the wrong network !".red)
    console.log("If you do change the network, please restart the plugin.")
  }
  else console.log('Last send: ',ESPserver.lastSend)
}

exports.consoledisp = display
