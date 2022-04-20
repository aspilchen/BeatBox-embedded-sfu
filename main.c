#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include "drum_machine.h"
#include "UI.h"

int main() {
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	printf("Initializing\n");
	ui_init();
	drum_init();
	printf("Ready\n");

	pthread_mutex_lock(&lock);
	pthread_mutex_lock(&lock);
	return 0;
}