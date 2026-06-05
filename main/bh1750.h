#ifndef BH1750_H
#define BH1750_H

#include <stdbool.h>

bool bh1750_init(void);
bool bh1750_read_lux(float *lux);

#endif // BH1750_H
