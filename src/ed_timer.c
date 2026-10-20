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

#include "ed_timer.h"

#include "ed_kernel.h"
#include "ed_debug.h"

#define ED_TIMER_NULL_INDEX     (0xFFU)

typedef struct {
    uint8_t next;

    ed_task_id_t des;
    ed_signal_t sig;

    uint32_t remaining;
    uint32_t period;
} ed_timer_node_t;

static ed_timer_node_t s_timer_pool[ED_TIMER_POOL_SIZE];
static uint8_t s_timer_free_head;
static uint8_t s_timer_used_count;

/* Delta-list head: node.remaining is delta from previous node. */
static uint8_t s_active_head;

static volatile uint32_t s_elapsed_acc;

static uint8_t timer_alloc(void) {
    if (s_timer_free_head == ED_TIMER_NULL_INDEX) {
        return ED_TIMER_NULL_INDEX;
    }

    uint8_t index = s_timer_free_head;
    s_timer_free_head = s_timer_pool[index].next;
    s_timer_used_count++;
    return index;
}

static void timer_free(uint8_t index) {
    s_timer_pool[index].next = s_timer_free_head;
    s_timer_free_head = index;

    if (s_timer_used_count > 0U) {
        s_timer_used_count--;
    }
}

static bool timer_detach_existing(ed_task_id_t des, ed_signal_t sig)
{
    uint8_t prev = ED_TIMER_NULL_INDEX;
    uint8_t cur = s_active_head;

    while (cur != ED_TIMER_NULL_INDEX) {
        if ((s_timer_pool[cur].des == des) && (s_timer_pool[cur].sig == sig)) {
            uint8_t next = s_timer_pool[cur].next;

            if (next != ED_TIMER_NULL_INDEX) {
                s_timer_pool[next].remaining += s_timer_pool[cur].remaining;
            }

            if (prev == ED_TIMER_NULL_INDEX) {
                s_active_head = next;
            } else {
                s_timer_pool[prev].next = next;
            }

            timer_free(cur);
            return true;
        }

        prev = cur;
        cur = s_timer_pool[cur].next;
    }

    return false;
}

static void timer_insert_sorted(uint8_t node_idx, uint32_t timeout_ms) {
    uint8_t prev = ED_TIMER_NULL_INDEX;
    uint8_t cur = s_active_head;

    while ((cur != ED_TIMER_NULL_INDEX) && (timeout_ms >= s_timer_pool[cur].remaining)) {
        timeout_ms -= s_timer_pool[cur].remaining;
        prev = cur;
        cur = s_timer_pool[cur].next;
    }

    s_timer_pool[node_idx].remaining = timeout_ms;
    s_timer_pool[node_idx].next = cur;

    if (cur != ED_TIMER_NULL_INDEX) {
        s_timer_pool[cur].remaining -= timeout_ms;
    }

    if (prev == ED_TIMER_NULL_INDEX) {
        s_active_head = node_idx;
    } else {
        s_timer_pool[prev].next = node_idx;
    }
}

void ed_timer_init(void) {
    for (uint8_t i = 0; i < ED_TIMER_POOL_SIZE; i++) {
        s_timer_pool[i].next = (i == (ED_TIMER_POOL_SIZE - 1U)) ? ED_TIMER_NULL_INDEX : (uint8_t)(i + 1U);
        s_timer_pool[i].remaining = 0;
        s_timer_pool[i].period = 0;
    }

    s_timer_free_head = 0;
    s_timer_used_count = 0;
    s_active_head = ED_TIMER_NULL_INDEX;
    s_elapsed_acc = 0;
}

bool ed_timer_start(ed_task_id_t des, ed_signal_t sig, uint32_t timeout_ms, ed_timer_mode_t mode) {
    if (timeout_ms == 0U) {
        timeout_ms = 1U;
    }

    ed_enter_critical();

    (void)timer_detach_existing(des, sig);

    uint8_t idx = timer_alloc();
    if (idx == ED_TIMER_NULL_INDEX) {
        ed_exit_critical();
        ED_LOG("[ed] timer alloc failed des=%u sig=%u\n", des, sig);
        return false;
    }

    s_timer_pool[idx].des = des;
    s_timer_pool[idx].sig = sig;
    s_timer_pool[idx].period = (mode == ED_TIMER_PERIODIC) ? timeout_ms : 0U;

    timer_insert_sorted(idx, timeout_ms);

    ed_exit_critical();

    return true;
}

bool ed_timer_stop(ed_task_id_t des, ed_signal_t sig) {
    bool ret;

    ed_enter_critical();
    ret = timer_detach_existing(des, sig);
    ed_exit_critical();

    return ret;
}

void ed_timer_tick_isr(uint32_t elapsed_ms) {
    
    /*
     * Only account time while at least one timer is active.
     *
     * If ticks are accumulated while the timer list is empty, the next timer
     * created after a long idle gap can expire immediately because it inherits
     * stale elapsed time. This mirrors typical MCU timer-service semantics:
     * elapsed time starts counting when a software timer is armed.
     */

    if (s_active_head != ED_TIMER_NULL_INDEX) {
        s_elapsed_acc += elapsed_ms;
    }
}

uint8_t ed_timer_has_pending_tick(void) {
    return (s_elapsed_acc > 0U) ? ED_ENABLE : ED_DISABLE;
}

uint8_t ed_timer_process_budget(uint8_t budget) {
    if (budget == 0U) {
        return 0U;
    }

    ed_enter_critical();
    uint32_t elapsed = s_elapsed_acc;
    s_elapsed_acc = 0;
    ed_exit_critical();

    uint8_t processed = 0;

    while ((elapsed > 0U) && (s_active_head != ED_TIMER_NULL_INDEX) && (processed < budget)) {
        uint8_t head = s_active_head;

        if (elapsed < s_timer_pool[head].remaining) {
            s_timer_pool[head].remaining -= elapsed;
            elapsed = 0U;
            break;
        }

        elapsed -= s_timer_pool[head].remaining;

        s_active_head = s_timer_pool[head].next;
        (void)ed_post(s_timer_pool[head].des, s_timer_pool[head].sig);
        processed++;

        if (s_timer_pool[head].period > 0U) {
            uint32_t period = s_timer_pool[head].period;
            ed_task_id_t des = s_timer_pool[head].des;
            ed_signal_t sig = s_timer_pool[head].sig;
            timer_free(head);

            uint8_t idx = timer_alloc();
            if (idx == ED_TIMER_NULL_INDEX) {
                ED_LOG("[ed] periodic timer re-alloc failed des=%u sig=%u\n", des, sig);
                continue;
            }

            s_timer_pool[idx].des = des;
            s_timer_pool[idx].sig = sig;
            s_timer_pool[idx].period = period;
            timer_insert_sorted(idx, period);
        } else {
            timer_free(head);
        }
    }

    if ((elapsed > 0U) && (s_active_head != ED_TIMER_NULL_INDEX)) {
        ed_enter_critical();
        s_elapsed_acc += elapsed;
        ed_exit_critical();
    }

    return processed;
}

void ed_timer_process(void) {
    while (ed_timer_process_budget(ED_TIMER_SERVICE_BUDGET_PER_STEP) == ED_TIMER_SERVICE_BUDGET_PER_STEP) {
        /* keep draining with bounded chunks */
    }
}

uint8_t ed_timer_free_count(void) {
    return (uint8_t)(ED_TIMER_POOL_SIZE - s_timer_used_count);
}