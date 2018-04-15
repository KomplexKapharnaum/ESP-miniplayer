// File Server for sync
const glob = require("glob")
const fs = require("fs")
const http = require('http')
const pad = require('./utils.js').pad

var config = require('./config.js');

Number.prototype.pad = function(size) {
    var s = String(this);
    while (s.length < (size || 2)) {s = "0" + s;}
    return s;
}

const requestHandler = (request, response) => {
  //console.log(request.url)

  if (request.url == "/file") {

	var path = config.basepath.mp3;

  	request.on('data', function (data) {
		path += data;
		if (path.length > 1e6)
		    request.connection.destroy();
	});

	request.on('end', function () {
		if (fs.existsSync(path)) {
			var stat = fs.statSync(path);

			response.writeHead(200, {
				'Content-Type': 'audio/mpeg',
				'Content-Length': stat.size
			});

			var readStream = fs.createReadStream(path);

			console.log("Serving "+path);
			// We replaced all the event handlers with a simple call to readStream.pipe()
			readStream.pipe(response);
		}
		else response.end();

	});

  }

  else if (request.url == "/info") {

  	var body = '';

    request.on('data', function (data) {
      body += data;
      if (body.length > 1e6)
          request.connection.destroy();
    });

    request.on('end', function () {
      var bank = parseInt(body.split('/')[0])
      var note = parseInt(body.split('/')[1])

      // search file
      glob(config.basepath.mp3+"/"+bank.pad(3)+"/"+note.pad(3)+"*.mp3", function (er, files) {

        // answer
        var ans = ""+bank.pad(3)+" "+note.pad(3)

        // file exist
        if (files.length > 0) {
        	ans += " "+fs.statSync(files[0]).size.pad(10)
        	ans += " "+files[0].substring(config.basepath.mp3.length)
        }
        else ans += " "+(0).pad(10)

        console.log(ans)
        response.end(ans)
    })


	});

  }

  else if (request.url == "/listfiles") {

  	var bank = '';

    request.on('data', function (data) {
      bank += data;
      if (bank.length > 1e6)
          request.connection.destroy();
    });

    request.on('end', function () {
        var ans = ""
        bank = parseInt(bank);

        for (var note=0; note<128; note++)
        {
          ans += bank.pad(3)+" "+note.pad(3)+" "
          var path = glob.sync(config.basepath.mp3+"/"+bank.pad(3)+"/"+note.pad(3)+"*.mp3")
          if (path.length > 0) ans += fs.statSync(path[0]).size.pad(10)+" "+path[0].substring(config.basepath.mp3.length)
          else ans += (0).pad(10) //+" "+"               "
          ans += "\n"
        }

        // console.log(ans)
        response.end(ans)
    })


	}

}

const server = http.createServer(requestHandler)

exports.start = function() {
    server.listen(config.filesync.port, (err) => {
      if (err) {
        return console.log('FileSync: something bad happened', err)
      }
      console.log(`FileSync: server is listening on ${config.filesync.port}`)
    })
  }
