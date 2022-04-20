#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>

#include "accel.h"

#define I2C_FILE "/dev/i2c-1"
#define I2C_DEV 0x1C

static void accel_initImpl(void);
static void accel_cleanupImpl(void);
static void* accel_threadFunc(void*);
int32_t accel_listenImpl(void);

enum AccelRegister {
	REG_XMSB = 0x01
	,REG_XLSB = 0x02
	,REG_YMSB = 0x03
	,REG_YLSB = 0x04
	,REG_ZMSB = 0x05
	,REG_ZLSB = 0x06
	,REG_LOOPGAURD = 7
	,REG_CTRL = 0x2A
};

enum BuffIndex {
	XMSB
	,XLSB
	,YMSB
	,YLSB
	,ZMSB
	,ZLSB
	,ACCEL_BUFFSIZE
};

static uint16_t listeners[ACCEL_MAX_LISTENERS * 2];
static uint8_t nListeners = 0;
static pthread_t thread;

void accel_init(void)
{
	accel_initImpl();
}

void accel_cleanup(void)
{
	accel_cleanupImpl();
}

int32_t accel_listen(void)
{
	return accel_listenImpl();
}

static void accel_initImpl(void)
{
	const int8_t ACCEL_ACTIVE = 0x01;
	struct i2c_smbus_ioctl_data ioctlData;
	union i2c_smbus_data smBusData;
	ioctlData.read_write = I2C_SMBUS_WRITE;
	ioctlData.size = I2C_SMBUS_BYTE_DATA;
	ioctlData.command = REG_CTRL;
	ioctlData.data = &smBusData;
	smBusData.byte = ACCEL_ACTIVE;
	int32_t fd = open(I2C_FILE, O_RDWR);
	ioctl(fd, I2C_SLAVE, I2C_DEV);
	ioctl(fd, I2C_SMBUS, &ioctlData);
	close(fd);
	pthread_create(&thread, NULL, accel_threadFunc, NULL);
}

static void accel_cleanupImpl(void)
{
}

static void* accel_threadFunc(void* arg)
{
	struct timespec req = {0, 500000000};
	struct timespec rem;
	struct i2c_smbus_ioctl_data ioctlData[6];
	union i2c_smbus_data smBusData[6];
	int8_t rawData[7];
	int16_t data[3];
	int32_t fd = open(I2C_FILE, O_RDWR);
	ioctl(fd, I2C_SLAVE, I2C_DEV);
	write(fd, 0x00, 1);

	while(true) {
		read(fd, rawData, 7);
		data[0] = (rawData[1] << 8) | (rawData[2]);
		data[1] = (rawData[3] << 8) | (rawData[4]);
		data[2] = (rawData[5] << 8) | (rawData[6]);
		data[0] = data[0] >> 4;
		data[1] = data[1] >> 4;
		data[2] = data[2] >> 4;
		// printf("x %0.2f : y %0.2f : z %0.2f\n", (double)data[0]/1024, (double)data[1]/1024, (double)data[2]/1024);
		for(int8_t i = 0; i < nListeners; i++) {
			write(listeners[(i*2) + 1], data, sizeof(*data) * 3);
		}
		nanosleep(&req, &rem);
	}
	return NULL;
}

static int32_t accel_listenImpl(void)
{
	if (nListeners < ACCEL_MAX_LISTENERS) {
		pipe(listeners + (nListeners * 2));
		return listeners[nListeners];
	}
	return 0;
}