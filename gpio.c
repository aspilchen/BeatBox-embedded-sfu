#include <stdio.h>
#include <time.h>

#include "gpio.h"

#define GPIO_MAX_PATHSIZE 64


// If b is false, returns error code from containing function.
#define SOFT_ASSERT(b) {if(!(b)) {return GPIO_ERR;}}

static enum gpio_ErrCode gpio_exportImpl(const gpiopin_t pin);
static enum gpio_ErrCode gpio_readImpl(char* buffer, gpiopin_t pin, char* filename, uint32_t size);
static enum gpio_ErrCode gpio_writeImpl(gpiopin_t pin, char* filename, char* input);
static int gpio_openImpl(const gpiopin_t pin, char* filename, int oflag);
static FILE* gpio_fopenImpl(const gpiopin_t pin, char* filename, char* modes);
static void gpio_generatePathImpl(char* buffer, gpiopin_t pin, char* filename, const uint32_t bufferSize);


/*****************************************/
/********** interface functions **********/

enum gpio_ErrCode gpio_export(const gpiopin_t pin)
{
	return gpio_exportImpl(pin);
}

enum gpio_ErrCode gpio_read(char* buffer, gpiopin_t pin, char* filename, uint32_t size)
{
	return gpio_readImpl(buffer, pin, filename, size);
}

enum gpio_ErrCode gpio_write(gpiopin_t pin, char* filename, char* input)
{
	return gpio_writeImpl(pin, filename, input);
}

int gpio_open(const gpiopin_t pin, char* filename, int oflag)
{
	return gpio_openImpl(pin, filename, oflag);
}

FILE* gpio_fopen(const gpiopin_t pin, char* filename, char* modes)
{
	return gpio_fopenImpl(pin, filename, modes);
}

void gpio_generatePath(char* buffer, const gpiopin_t pin, char* filename, const uint32_t bufferSize) {
	gpio_generatePathImpl(buffer, pin, filename, bufferSize);
}

/**************************************/
/********** static functions **********/

static enum gpio_ErrCode gpio_exportImpl(const gpiopin_t pin)
{
	const uint64_t MILLISEC_IN_NANOSEC = 100000000;
	struct timespec req = {0, MILLISEC_IN_NANOSEC * 3};
	struct timespec rem;
	FILE *f = fopen("/sys/class/gpio/export", "w");
	SOFT_ASSERT(f);
	fprintf(f, "%d", pin);
	fclose(f);
	nanosleep(&req, &rem); // Wait for export to complete
	return GPIO_OK;
}


static enum gpio_ErrCode gpio_readImpl(char* buffer, gpiopin_t pin, char* filename, uint32_t size)
{
	FILE *f = gpio_fopenImpl(pin, filename, "r");
	SOFT_ASSERT(f);
	fgets(buffer, size, f);
	fclose(f);
	return GPIO_OK;
}

static enum gpio_ErrCode gpio_writeImpl(gpiopin_t pin, char* filename, char* input)
{
	FILE *f = gpio_fopenImpl(pin, filename, "w");
	SOFT_ASSERT(f);
	fprintf(f, "%s", input);
	fclose(f);
	return GPIO_OK;
}

static int gpio_openImpl(const gpiopin_t pin, char* filename, int oflag)
{
	char path[GPIO_MAX_PATHSIZE];
	gpio_generatePath(path, pin, filename, GPIO_MAX_PATHSIZE);
	int fd = open(path, oflag);
	return fd;
}

static FILE* gpio_fopenImpl(const gpiopin_t pin, char* filename, char* modes)
{
	char path[GPIO_MAX_PATHSIZE];
	gpio_generatePath(path, pin, filename, GPIO_MAX_PATHSIZE);
	FILE *f = fopen(path, modes);
	return f;
}

static void gpio_generatePathImpl(char* buffer, const gpiopin_t pin, char* filename, const uint32_t bufferSize)
{
	#define GPIO_PATH "/sys/class/gpio/gpio"
	snprintf(buffer, bufferSize, "%s%d/%s", GPIO_PATH, pin, filename);
}