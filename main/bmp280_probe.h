#ifndef BMP280_PROBE_H
#define BMP280_PROBE_H

#include <stdbool.h>

bool bmp280_init(void);
bool bmp280_read_temperature_c(float *temperature_c);
bool bmp280_read_temperature_and_pressure(float *temperature_c, float *pressure_hpa);

#endif // BMP280_PROBE_H
