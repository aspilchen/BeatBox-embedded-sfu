#ifndef GPIOH_H
#define GPIOH_H

/**
 * Helper to manage GPIO pins/actions.
 */

#include <stdint.h>
#include <fcntl.h>

#define GPIO_VALUE "value"
#define GPIO_EDGE "edge"

#define GPIO_IN   "in"
#define GPIO_OUT  "out"
#define GPIO_BOTH "both"

typedef uint8_t gpiopin_t;

enum gpio_ErrCode {
	GPIO_OK,
	GPIO_ERR
};

enum gpio_ErrCode gpio_export(const gpiopin_t pin);
enum gpio_ErrCode gpio_read(char* buffer, gpiopin_t pin, char* target, uint32_t size);
enum gpio_ErrCode gpio_write(gpiopin_t pin, char* filename, char* input);
int gpio_open(const gpiopin_t pin, char* filename, int oflag);
FILE* gpio_fopen(const gpiopin_t pin, char* filename, char* modes);

#endif