"use strict";
/*
 * Respond to commands over a websocket to relay UDP commands to a local program
 */

var socketio = require('socket.io');
var io;

var dgram = require('dgram');
const { futimesSync } = require('fs');
const PORT = 1337;
const HOST = '192.168.7.2';


const CLIENT_OP_SET_MODE       = 0;
const CLIENT_OP_SET_VOLUME     = 1;
const CLIENT_OP_SET_TEMPO      = 2;
const CLIENT_OP_PLAY_SOUND     = 3;
const CLIENT_OP_GET_MODE = 4;
const CLIENT_OP_GET_VOLUME     = 5;
const CLIENT_OP_GET_TEMPO      = 6;
const CLIENT_OP_RESPONSE = 8;

const WEB_OP_RESPONSE   = 'response';
const WEB_OP_SET_MODE   = 'setMode';
const WEB_OP_PLAY_SOUND   = 'playSound';
const WEB_OP_WEB_RESPONSE = 'response';
const WEB_OP_SET_TEMPO    = 'setTempo';
const WEB_OP_GET_TEMPO    = 'getTempo';
const WEB_OP_SET_VOLUME   = 'setVolume';
const WEB_OP_GET_VOLUME   = 'getVolume';
const WEB_OP_GET_MODE = 'getMode';

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	var client = dgram.createSocket('udp4');

	socket.on(WEB_OP_SET_MODE, function(data) {
		updForward(client, CLIENT_OP_SET_MODE + ' ' + data);
	});
	socket.on(WEB_OP_SET_VOLUME, function(data) {
		updForward(client, CLIENT_OP_SET_VOLUME + ' ' + data);
	});
	socket.on(WEB_OP_SET_TEMPO, function(data) {
		updForward(client, CLIENT_OP_SET_TEMPO + ' ' + data);
	});
	socket.on(WEB_OP_GET_TEMPO, function() {
		updForward(client, CLIENT_OP_GET_TEMPO);
		// socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_TEMPO, 120]);
	});
	socket.on(WEB_OP_GET_VOLUME, function() {
		updForward(client, CLIENT_OP_GET_VOLUME);
		// socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_VOLUME, 80]);
	});
	socket.on(WEB_OP_GET_MODE, function() {
		updForward(client, CLIENT_OP_GET_MODE);
		// socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_MODE, 'Rock1']);
	})
	socket.on(WEB_OP_PLAY_SOUND, function(data) {
		const SOUNDMAP = {'bass': 0, 'hat': 1, 'snare': 2};
		updForward(client, CLIENT_OP_PLAY_SOUND + ' ' + SOUNDMAP[data]);
	})


	client.on('listening', function() {
		console.log("UDP client listening on port 1337");

		client.on('message', function(message) {
			var responseType;
			var tokens = message.toString().split(' ');
			switch(parseInt(tokens[0])) {
				case CLIENT_OP_GET_VOLUME:
					socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_VOLUME, tokens[1]]);
					break;
				case CLIENT_OP_GET_TEMPO:
					socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_TEMPO, tokens[1]]);
					break;
				case CLIENT_OP_GET_MODE:
					socket.emit(WEB_OP_RESPONSE, [WEB_OP_GET_MODE, tokens[1]]);
					break;
				default:
			}
		})

	});
};


function updForward(client, message) {
	let msg = message.toString();
	console.log("sending " + msg);
	client.send(msg, 0, msg.length, PORT, HOST, function(err, bytes) {
		if (err) {
			console.log('UDP message sent to ' + HOST +':'+ PORT);
		}
	});
}