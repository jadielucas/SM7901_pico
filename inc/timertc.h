#ifndef TIMERTC_H
#define TIMERTC_H

#include "hardware/rtc.h"
#include <time.h>

void my_rtc_set_from_sntp(uint32_t epoch_seconds, uint32_t epoch_microseconds);
void init_and_sync_rtc();

#endif