exports.cls = function () {
  return process.stdout.write('\033c');
}

exports.pad = function(number, length) {
    var str = '' + number;
    while (str.length < length) str = '0' + str
    return str
}

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
