/**************************************************************************************/

/*
 * index.html
 */
// This string holds HTML, CSS, and Javascript for the HTML5 UI.
// The browser must support HTML5 WebSockets which is true for all modern browsers.
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<meta name = "viewport" content = "width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0">
<title>ESP8266 Sound Effects Web Trigger</title>
<style>
body { font-family: Arial, Helvetica, Sans-Serif; }
table { border: 1px solid black; }
th { border: 1px solid black; }
td { text-align: left; border: 1px solid black; }
.SFXtd { text-align: right; }
.SFXButton { font-size: 125%; background-color: #E0E0E0; border: 1px solid; }
</style>
<script>
function enableTouch(objname) {
  console.log('enableTouch', objname);
  var e = document.getElementById(objname);
  if (e) {
    e.addEventListener('touchstart', function(event) {
        event.preventDefault();
        console.log('touchstart', event);
        buttondown(e);
        }, false );
    e.addEventListener('touchend',   function(event) {
        console.log('touchend', event);
        buttonup(e);
        }, false );
  }
  else {
    console.log(objname, ' not found');
  }
}

var websock;
var WebSockOpen=0;  //0=close,1=opening,2=open

function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
  WebSockOpen=1;
  websock.onopen = function(evt) {
    console.log('websock open');
    WebSockOpen=2;
    var e = document.getElementById('webSockStatus');
    e.style.backgroundColor = 'green';
  };
  websock.onclose = function(evt) {
    console.log('websock close');
    WebSockOpen=0;
    var e = document.getElementById('webSockStatus');
    e.style.backgroundColor = 'red';
  };
  websock.onerror = function(evt) { console.log(evt); };
  websock.onmessage = function(evt) {
    var nowPlaying = document.getElementById('nowPlaying');
    if (evt.data.startsWith('nowPlaying=')) {
      nowPlaying.innerHTML = evt.data;
    }
    else {
      console.log('unknown event', evt.data);
    }
  };

  var allButtons = [
    'bSFX0',
    'bSFX1',
    'bSFX2',
    'bSFX3',
    'bSFX4',
    'bSFX5',
    'bSFX6',
    'bSFX7',
    'bSFX8',
    'bSFX9',
  ];
  for (var i = 0; i < allButtons.length; i++) {
    enableTouch(allButtons[i]);
  }
}

function buttondown(e) {
  switch (WebSockOpen) {
    case 0:
      window.location.reload();
      WebSockOpen=1;
      break;
    case 1:
    default:
      break;
    case 2:
      websock.send(e.id + '=1');
      break;
  }
}

function buttonup(e) {
  switch (WebSockOpen) {
    case 0:
      window.location.reload();
      WebSockOpen=1;
      break;
    case 1:
    default:
      break;
    case 2:
      websock.send(e.id + '=0');
      break;
  }
}
</script>
</head>
<body onload="javascript:start();">
<h2>ESP SF/X Web Trigger</h2>
<div id="nowPlaying">Now Playing</div>
<table>
  <tr>
    <td class="SFXtd"><button id="bSFX0" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">0</button></td>
    <td>Track 0</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX1" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">1</button></td>
    <td>Track 1</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX2" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">2</button></td>
    <td>Track 2</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX3" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">3</button></td>
    <td>Track 3</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX4" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">4</button></td>
    <td>Track 4</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX5" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">5</button></td>
    <td>Track 5</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX6" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">6</button></td>
    <td>Track 6</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX7" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">7</button></td>
    <td>Track 7</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX8" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">8</button></td>
    <td>Track 8</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFX9" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">9</button></td>
    <td>Track 9</td>
  </tr>
  <tr>
    <td class="SFXtd"><button id="bSFY0" type="button" class="SFXButton"
      onmousedown="buttondown(this);" onmouseup="buttonup(this);">BAM</button></td>
    <td>Track 9</td>
  </tr>
</table>
<p>
</body>
</html>
)rawliteral";
