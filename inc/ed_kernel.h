#ifndef __ED_KERNEL_H__
#define __ED_KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "ed_event.h"
#include "ed_timer.h"

typedef uint8_t ed_priority_t;
typedef void (*ed_task_handler_t)(ed_event_t* event);
typedef void (*ed_polling_handler_t)(void);
typedef void (*ed_port_hook_t)(void);

typedef struct {
    ed_task_id_t id;
    ed_priority_t prio;
    ed_task_handler_t handler;
} ed_task_desc_t;

typedef struct {
    uint8_t enable;
    ed_polling_handler_t handler;
} ed_polling_desc_t;

void ed_kernel_init(void);

/* portability hooks for critical section */
void ed_port_init(ed_port_hook_t enter_critical, ed_port_hook_t exit_critical);

bool ed_task_create(ed_task_desc_t task);
bool ed_polling_create(ed_polling_desc_t polling);

bool ed_post(ed_task_id_t des, ed_signal_t sig);
bool ed_post_data(ed_task_id_t des, ed_signal_t sig, const uint8_t* data, uint8_t len);

void ed_kernel_step(void);
void ed_kernel_run(void);

uint8_t ed_kernel_ready_mask(void);

void ed_enter_critical(void);
void ed_exit_critical(void);

#ifdef __cplusplus
}
#endif

#endif /* __ED_KERNEL_H__ */