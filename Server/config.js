var config = {};

config.filesync = {}
config.filesync.port = 3742;

config.webremote = {}
config.webremote.port = 8088;

config.oscremote = {}
config.oscremote.port = 3753;

config.espserver = {}
config.espserver.portin = 1819;
config.espserver.portout = 1818;

config.espserver.osc2mqtt = {}
config.espserver.osc2mqtt.port  = 9037;
config.espserver.osc2mqtt.server = '2.0.0.1';

config.basepath = {}
config.basepath.mp3 = '../mp3'

config.player = {}
config.player.loop = false

module.exports = config;
