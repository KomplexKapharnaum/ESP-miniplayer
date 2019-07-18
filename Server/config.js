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

config.basepath = {}
config.basepath.mp3 = '../mp3'

config.player = {}
config.player.loop = false

module.exports = config;
