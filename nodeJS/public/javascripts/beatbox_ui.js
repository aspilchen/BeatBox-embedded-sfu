const WEB_OP_RESPONSE   = 'response';
const WEB_OP_SET_MODE   = 'setMode';
const WEB_OP_PLAY_SOUND   = 'playSound';
const WEB_OP_WEB_RESPONSE = 'response';
const WEB_OP_SET_TEMPO    = 'setTempo';
const WEB_OP_GET_TEMPO    = 'getTempo';
const WEB_OP_SET_VOLUME   = 'setVolume';
const WEB_OP_GET_VOLUME   = 'getVolume';
const WEB_OP_GET_MODE = 'getMode';

const MODE_NAMES = ['Rock #1', 'Personal', 'None'];

const MODE_ROCK1 = 0;
const MODE_ROCK2 = 1;
const MODE_NONE = 2;

const VOLUME_MIN = 0;
const VOLUME_MAX = 100;

const TEMPO_MIN = 40;
const TEMPO_MAX = 300;

var socket = io.connect();

function setMode(mode) {
	socket.emit(WEB_OP_SET_MODE, mode);
}

function displayVolume(volume) {
	$('#volumeid').val(volume);
}

function isValidVolume(volume) {
	return VOLUME_MIN <= volume && volume <= VOLUME_MAX;
}

function setVolume(direction) {
	const VOLUME_CHANGE = 5;
	var volume = parseInt($('#volumeid').val());
	if(direction == 'down') {
		volume -= VOLUME_CHANGE;
	} else if(direction == 'up') {
		volume += VOLUME_CHANGE;
	}
	if(isValidVolume(volume)) {
		socket.emit(WEB_OP_SET_VOLUME, volume);
		displayVolume(volume);
	}
}

function displayTempo(tempo) {
	$('#tempoid').val(tempo);
}

function isValidTempo(tempo) {
	return TEMPO_MIN <= tempo && tempo <= TEMPO_MAX;
}

function setTempo(direction) {
	const TEMPO_CHANGE = 5;
	var tempo = parseInt($('#tempoid').val());
	if(direction == 'down') {
		tempo -= TEMPO_CHANGE;
	} else if(direction == 'up') {
		tempo += TEMPO_CHANGE;
	}
	if(isValidTempo(tempo)) {
		socket.emit(WEB_OP_SET_TEMPO, tempo);
		displayTempo(tempo);
	}
}

function displayMode(mode) {
	$('#modeid').text(mode);
}

$(document).ready(function () {



	$('#modeNone').click(function() {
		setMode(MODE_NONE);
		return false;
	})

	$('#modeRock1').click(function() {
		setMode(MODE_ROCK1);
		return false;
	})

	$('#modeRock2').click(function() {
		setMode(MODE_ROCK2);
		return false;
	})

	$('#volumeDown').click(function() {
		setVolume('down');
		return false;
	});

	$('#volumeUp').click(function() {
		setVolume('up');
		return false;
	});

	$('#tempoDown').click(function() {
		setTempo('down');
		return false;
	})

	$('#tempoUp').click(function() {
		setTempo('up');
		return false;
	})

	$('#hat').click(function() {
		socket.emit(WEB_OP_PLAY_SOUND, 'hat');
		return false;
	})

	$('#snare').click(function() {
		socket.emit(WEB_OP_PLAY_SOUND, 'snare');
		return false;
	})

	$('#bass').click(function() {
		socket.emit(WEB_OP_PLAY_SOUND, 'bass');
		return false;
	})

	socket.on(WEB_OP_RESPONSE, function(data) {
		if(data[0] == WEB_OP_GET_TEMPO && isValidTempo(data[1])) {
			displayTempo(data[1]);
		} else if(data[0] == WEB_OP_GET_VOLUME && isValidVolume(data[1])) {
			displayVolume(data[1]);
		} else if(data[0] == WEB_OP_GET_MODE) {
			displayMode(data[1]);
		}
	})

	socket.emit(WEB_OP_GET_VOLUME);
	socket.emit(WEB_OP_GET_TEMPO);
	socket.emit(WEB_OP_GET_MODE);


})