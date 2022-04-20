#include <stdint.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "udp.h"
#include "drum_machine.h"
#include "UI.h"

#define OP_BUFFSIZE 1
#define ARG_BUFFSIZE 8

enum ui_UDPOpCode {
	SET_BEAT
	,SET_VOLUME
	,SET_TEMPO
	,PLAY_SOUND
	,GET_CURR_BEAT
	,GET_VOLUME
	,GET_TEMPO
	,N_OPS
	,RESPONSE
};

static void ui_udpInitImpl(void);
static void ui_udpCleanupImpl(void);
static void* ui_udpThreadFunc(void*);
static uint8_t ui_udpParseMsg(enum ui_UDPOpCode *opBuffer, char *argBuffer, char *message);
static uint8_t ui_udpTokenizeMsg(char **opBuffer, char **argBuffer, char *message);
static void ui_udpSetBeat(char*, struct sockaddr_in*);
static void ui_udpSetVolume(char*, struct sockaddr_in*);
static void ui_udpSetTempo(char*, struct sockaddr_in*);
static void ui_udpPlaySound(char*, struct sockaddr_in*);
static void ui_updGetCurrBeat(char*, struct sockaddr_in*);
static void ui_updGetVolume(char*, struct sockaddr_in*);
static void ui_updGetTempo(char*, struct sockaddr_in*);


// static const char* OP_STRINGS[] = {
// 	"setBeat"
// 	,"setVolume"
// 	,"setTempo"
// 	,"playSound"
// };

static const uint32_t PORT = 1337;

static void (*actionFuncs[]) (char*, struct sockaddr_in*) = {
	ui_udpSetBeat
	,ui_udpSetVolume
	,ui_udpSetTempo
	,ui_udpPlaySound
	,ui_updGetCurrBeat
	,ui_updGetVolume
	,ui_updGetTempo
};

static int32_t socketFD;
static pthread_t thread;

void ui_udpInit(void)
{
	ui_udpInitImpl();
}

void ui_udpCleanup(void)
{
	ui_udpCleanupImpl();
}

static void ui_udpInitImpl(void)
{
	socketFD = udp_openSocket(PORT);
	pthread_create(&thread, NULL, ui_udpThreadFunc, NULL);
}

static void ui_udpCleanupImpl(void)
{
	pthread_cancel(thread);
	udp_closeSocket(socketFD);
}

static void* ui_udpThreadFunc(void* arg)
{
	enum ui_UDPOpCode opBuff[OP_BUFFSIZE];
	char argBuff[ARG_BUFFSIZE];
	char buffer[MSG_MAX_LEN];
	struct sockaddr_in sockAddr;
	int32_t msgLen;
	while(true) {
		msgLen = udp_receive(buffer, socketFD, MSG_MAX_LEN, &sockAddr);
		ui_udpParseMsg(opBuff, argBuff, buffer);
		printf("received %d\n", msgLen);
		printf("%s\n", buffer);
		if(0 <= opBuff[0] && opBuff[0] < N_OPS) {
			actionFuncs[opBuff[0]](argBuff, &sockAddr);
		}
	}
	return NULL;
}

static uint8_t ui_udpParseMsg(enum ui_UDPOpCode *opBuffer, char *argBuffer, char *message)
{
	char *opTok;
	char *argTok;
	uint8_t nToks = ui_udpTokenizeMsg(&opTok, &argTok, message);
	if(nToks > 0) {
		*opBuffer = atoi(opTok);
	}
	if(nToks > 1) {
		memcpy(argBuffer, argTok, ARG_BUFFSIZE);
	}
	return nToks;
}

static uint8_t ui_udpTokenizeMsg(char **opBuffer, char **argBuffer, char *message)
{
	char delim[2] = " ";
	*opBuffer = strtok(message, delim);
	if(!(*opBuffer)) {
		return 0;
	}
	*argBuffer = strtok(NULL, delim);
	if(!(*argBuffer)) {
		return 1;
	}
	return 2;
}

static void ui_udpSetBeat(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t id = atoi(arg);
	ui_setRythm(id);
}

static void ui_udpSetVolume(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t volume = atoi(arg);
	ui_setVolume(volume);
}

static void ui_udpSetTempo(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t tempo = atoi(arg);
	ui_setTempo(tempo);
}

static void ui_udpPlaySound(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t id = atoi(arg);
	drum_playWave(id);
}

static void ui_updGetCurrBeat(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t id = drum_getCurrRythm();
	const char *beatName = drum_getRythmName(id);
	char buffer[MSG_MAX_LEN];
	snprintf(buffer, MSG_MAX_LEN-1, "%d %s", GET_CURR_BEAT, beatName);
	printf("sending %s\n", buffer);
	udp_send(socketFD, buffer, sockAddr);
}

static void ui_updGetVolume(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t volume = drum_getVolume();
	char buffer[MSG_MAX_LEN];
	snprintf(buffer, MSG_MAX_LEN-1, "%d %d", GET_VOLUME, volume);
	printf("sending %s\n", buffer);
	udp_send(socketFD, buffer, sockAddr);
}

static void ui_updGetTempo(char *arg, struct sockaddr_in *sockAddr)
{
	uint32_t tempo = drum_getTempo();
	char buffer[MSG_MAX_LEN];
	snprintf(buffer, MSG_MAX_LEN-1, "%d %d", GET_TEMPO, tempo);
	printf("sending %s\n", buffer);
	udp_send(socketFD, buffer, sockAddr);
}