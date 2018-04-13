var dgram = require('dgram');
const EventEmitter = require('events');

udpSocket = dgram.createSocket('udp4');

/*payload = "/esp/007/play/BAMBAM.MP3";

udp.send(payload, 0, payload.length, 10000, "10.2.7.18", function(err, bytes) {
          if (err) {
            if (err.code == 'ENETUNREACH' || err.code == 'EADDRNOTAVAIL') {
              console.log('\nWarning: the server lost connection to the network');
            }
            else throw err;
          }
          else console.log('OK.');
          process.exit(0);
      });
      */
      
      
udpSocket.on('message', function (message, remote) {
        var ip = remote.address;
        var info = message.toString('UTF-8');
		console.log(info, ip);
    });
    

udpSocket.bind(12000);
