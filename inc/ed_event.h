#ifndef __ED_EVENT_H__
#define __ED_EVENT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "ed_config.h"

typedef uint8_t ed_task_id_t;
typedef uint8_t ed_signal_t;

typedef enum {
    ED_EVENT_TYPE_PURE = 0,     // signal without data
    ED_EVENT_TYPE_FIXED,        // signal with data
} ed_event_type_t;

typedef struct ed_event_t {
    uint8_t next;
    uint8_t type;

    ed_task_id_t src;
    ed_task_id_t des;
    ed_signal_t sig;

    uint8_t len;
    uint8_t payload[ED_EVENT_PAYLOAD_MAX];
} ed_event_t;

void ed_event_init(void);

ed_event_t* ed_event_alloc(void);
void ed_event_free(ed_event_t* event);

bool ed_event_set_payload(ed_event_t* event, const uint8_t* data, uint8_t len);

uint8_t ed_event_free_count(void);

/* index helpers for scheduler queues (no cross-module global access) */
uint8_t ed_event_index_of(const ed_event_t* event);
ed_event_t* ed_event_from_index(uint8_t index);

#ifdef __cplusplus
}
#endif

#endif /* __ED_EVENT_H__ */
