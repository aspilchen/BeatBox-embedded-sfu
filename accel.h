#ifndef ACCEL_H
#define ACCEL_H

/**
 * Did not complete this in time.
 */

#define ACCEL_MAX_LISTENERS 1

struct AccelData {
	double x;
	double y;
	double z;
};

void accel_init(void);
void accel_cleanup(void);
int32_t accel_listen(void);

#endif