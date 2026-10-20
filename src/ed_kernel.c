/**
 *     /$$$$$$$  /$$$$$$$$  /$$$$$$  /$$$$$$  /$$$$$$  /$$   /$$ /$$$$$$$$ /$$$$$$$        /$$$$$$$  /$$     /$$
 *    | $$__  $$| $$_____/ /$$__  $$|_  $$_/ /$$__  $$| $$$ | $$| $$_____/| $$__  $$      | $$__  $$|  $$   /$$/
 *    | $$  \ $$| $$      | $$  \__/  | $$  | $$  \__/| $$$$| $$| $$      | $$  \ $$      | $$  \ $$ \  $$ /$$/ 
 *    | $$  | $$| $$$$$   |  $$$$$$   | $$  | $$ /$$$$| $$ $$ $$| $$$$$   | $$  | $$      | $$$$$$$   \  $$$$/  
 *    | $$  | $$| $$__/    \____  $$  | $$  | $$|_  $$| $$  $$$$| $$__/   | $$  | $$      | $$__  $$   \  $$/   
 *    | $$  | $$| $$       /$$  \ $$  | $$  | $$  \ $$| $$\  $$$| $$      | $$  | $$      | $$  \ $$    | $$    
 *    | $$$$$$$/| $$$$$$$$|  $$$$$$/ /$$$$$$|  $$$$$$/| $$ \  $$| $$$$$$$$| $$$$$$$/      | $$$$$$$/    | $$    
 *    |_______/ |________/ \______/ |______/ \______/ |__/  \__/|________/|_______/       |_______/     |__/    
 *                                                                                                              
 *                                                                                                              
 *                                                                                                              
 *                 /$$      /$$ /$$$$$$ /$$   /$$ /$$   /$$        /$$$$$$  /$$   /$$ /$$$$$$                   
 *                | $$$    /$$$|_  $$_/| $$$ | $$| $$  | $$       /$$__  $$| $$  | $$|_  $$_/                   
 *                | $$$$  /$$$$  | $$  | $$$$| $$| $$  | $$      | $$  \__/| $$  | $$  | $$                     
 *                | $$ $$/$$ $$  | $$  | $$ $$ $$| $$$$$$$$      | $$      | $$$$$$$$  | $$                     
 *                | $$  $$$| $$  | $$  | $$  $$$$| $$__  $$      | $$      | $$__  $$  | $$                     
 *                | $$\  $ | $$  | $$  | $$\  $$$| $$  | $$      | $$    $$| $$  | $$  | $$                     
 *                | $$ \/  | $$ /$$$$$$| $$ \  $$| $$  | $$      |  $$$$$$/| $$  | $$ /$$$$$$                   
 *                |__/     |__/|______/|__/  \__/|__/  |__/       \______/ |__/  |__/|______/                   
 *                                                                                                              
 *                                                                                                              
 *                                                                                                              
 */

#include "ed_kernel.h"

#include <string.h>

#include "ed_debug.h"

#define ED_NULL_INDEX           (0xFFU)

typedef struct {
    uint8_t q_head;
    uint8_t q_tail;
    uint8_t prio_mask;
} ed_runq_t;

static ed_task_desc_t s_tasks[ED_TASK_MAX];
static uint8_t s_task_count;

static ed_polling_desc_t s_polling[ED_TASK_MAX];
static uint8_t s_polling_count;

static ed_runq_t s_runq[ED_PRIORITY_MAX];
static uint8_t s_ready_bitmap;
static ed_task_id_t s_current_task;

static ed_port_hook_t s_port_enter_critical = (ed_port_hook_t)0;
static ed_port_hook_t s_port_exit_critical = (ed_port_hook_t)0;

void ed_enter_critical(void) {
    if (s_port_enter_critical != (ed_port_hook_t)0) {
        s_port_enter_critical();
    }
}

void ed_exit_critical(void) {
    if (s_port_exit_critical != (ed_port_hook_t)0) {
        s_port_exit_critical();
    }
}

void ed_port_init(ed_port_hook_t enter_critical, ed_port_hook_t exit_critical) {
    s_port_enter_critical = enter_critical;
    s_port_exit_critical = exit_critical;
}

static int8_t highest_prio_ready(uint8_t bitmap) {
    if (bitmap == 0U) {
        return -1;
    }

    for (int8_t p = (int8_t)ED_PRIORITY_MAX - 1; p >= 0; p--) {
        if ((bitmap & (1U << p)) != 0U) {
            return p;
        }
    }

    return -1;
}

void ed_kernel_init(void) {
    ed_event_init();
    ed_timer_init();

    memset(s_tasks, 0, sizeof(s_tasks));
    memset(s_polling, 0, sizeof(s_polling));

    s_task_count = 0;
    s_polling_count = 0;
    s_ready_bitmap = 0;
    s_current_task = 0;

    for (uint8_t i = 0; i < ED_PRIORITY_MAX; i++) {
        s_runq[i].q_head = ED_NULL_INDEX;
        s_runq[i].q_tail = ED_NULL_INDEX;
        s_runq[i].prio_mask = (uint8_t)(1U << i);
    }

    ED_LOG("[ed] kernel init done\n");
}

bool ed_task_create(ed_task_desc_t task) {
    if ((task.handler == (ed_task_handler_t)0) || (task.prio >= ED_PRIORITY_MAX)) {
        return false;
    }

    if ((task.id >= ED_TASK_MAX) || (s_task_count >= ED_TASK_MAX)) {
        return false;
    }

    if (s_tasks[task.id].handler != (ed_task_handler_t)0) {
        return false;
    }

    s_tasks[task.id] = task;
    s_task_count++;

    ED_LOG("[ed] task created id=%u prio=%u\n", task.id, task.prio);

    return true;
}

bool ed_polling_create(ed_polling_desc_t polling) {
    if ((polling.handler == (ed_polling_handler_t)0) || (s_polling_count >= ED_TASK_MAX)) {
        return false;
    }

    s_polling[s_polling_count++] = polling;
    return true;
}

static bool post_internal(ed_task_id_t des, ed_signal_t sig, const uint8_t* data, uint8_t len)
{
    if (des >= ED_TASK_MAX || s_tasks[des].handler == (ed_task_handler_t)0) {
        return false;
    }

    ed_event_t* event = ed_event_alloc();
    if (event == (ed_event_t*)0) {
        ED_LOG("[ed] drop event des=%u sig=%u (pool exhausted)\n", des, sig);
        return false;
    }

    event->src = s_current_task;
    event->des = des;
    event->sig = sig;

    if (data != (const uint8_t*)0) {
        if (!ed_event_set_payload(event, data, len)) {
            ed_event_free(event);
            return false;
        }
    }

    ed_enter_critical();

    uint8_t prio = s_tasks[des].prio;
    ed_runq_t* q = &s_runq[prio];
    uint8_t idx = ed_event_index_of(event);

    event->next = ED_NULL_INDEX;

    if (q->q_tail == ED_NULL_INDEX) {
        q->q_head = idx;
        q->q_tail = idx;
        s_ready_bitmap |= q->prio_mask;
    }
    else {
        ed_event_t* tail = ed_event_from_index(q->q_tail);
        if (tail != (ed_event_t*)0) {
            tail->next = idx;
            q->q_tail = idx;
        } else {
            ed_exit_critical();
            ed_event_free(event);
            return false;
        }
    }

    ed_exit_critical();

    return true;
}

bool ed_post(ed_task_id_t des, ed_signal_t sig)
{
    return post_internal(des, sig, (const uint8_t*)0, 0);
}

bool ed_post_data(ed_task_id_t des, ed_signal_t sig, const uint8_t* data, uint8_t len)
{
    return post_internal(des, sig, data, len);
}

static void run_scheduler_once(void)
{
    int8_t prio = highest_prio_ready(s_ready_bitmap);
    if (prio < 0) {
        return;
    }

    ed_runq_t* q = &s_runq[prio];

    ed_enter_critical();

    uint8_t event_idx = q->q_head;
    ed_event_t* event = ed_event_from_index(event_idx);

    if (event == (ed_event_t*)0) {
        q->q_head = ED_NULL_INDEX;
        q->q_tail = ED_NULL_INDEX;
        s_ready_bitmap &= (uint8_t)~q->prio_mask;
        ed_exit_critical();
        return;
    }

    q->q_head = event->next;

    if (q->q_head == ED_NULL_INDEX) {
        q->q_tail = ED_NULL_INDEX;
        s_ready_bitmap &= (uint8_t)~q->prio_mask;
    }

    ed_exit_critical();

    s_current_task = event->des;
    s_tasks[event->des].handler(event);
    ed_event_free(event);
}

void ed_kernel_step(void) {
    /* service timer every step to avoid starvation under heavy event load */
    if (ed_timer_has_pending_tick() == ED_ENABLE) {
        (void)ed_timer_process_budget(ED_TIMER_SERVICE_BUDGET_PER_STEP);
    }

    run_scheduler_once();

    if (s_ready_bitmap == 0U) {
        (void)ed_timer_process_budget(ED_TIMER_SERVICE_BUDGET_PER_STEP);

        for (uint8_t i = 0; i < s_polling_count; i++) {
            if (s_polling[i].enable == ED_ENABLE) {
                s_polling[i].handler();
            }
        }
    }
}

void ed_kernel_run(void) {
    for (;;) {
        ed_kernel_step();
    }
}

uint8_t ed_kernel_ready_mask(void) {
    return s_ready_bitmap;
}