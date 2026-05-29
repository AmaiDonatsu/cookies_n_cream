#ifndef BMP280_PROBE_H
#define BMP280_PROBE_H

bool bmp280_init(void);
bool bmp280_read_temperature_c(float *temperature_c);

#endif // BMP280_PROBE_H
