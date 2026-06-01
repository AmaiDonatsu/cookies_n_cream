#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <stdbool.h>
#include <stddef.h>

bool time_sync_wait_for_clock(void);
bool time_sync_get_iso8601(char *buffer, size_t buffer_len);

#endif // TIME_SYNC_H
