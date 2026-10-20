#ifndef __ED_TIMER_H__
#define __ED_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "ed_event.h"

typedef enum {
    ED_TIMER_ONE_SHOT = 0,
    ED_TIMER_PERIODIC,
} ed_timer_mode_t;

void ed_timer_init(void);

bool ed_timer_start(ed_task_id_t des, ed_signal_t sig, uint32_t timeout_ms, ed_timer_mode_t mode);
bool ed_timer_stop(ed_task_id_t des, ed_signal_t sig);

void ed_timer_tick_isr(uint32_t elapsed_ms);
void ed_timer_process(void);

/* process up to a budgeted number of expirations to avoid long jitter */
uint8_t ed_timer_process_budget(uint8_t budget);
uint8_t ed_timer_has_pending_tick(void);

uint8_t ed_timer_free_count(void);

#ifdef __cplusplus
}
#endif

#endif /* __ED_TIMER_H__ */