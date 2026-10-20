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
 */

#include "ed_event.h"

#include <string.h>

#define ED_EVENT_NULL_INDEX   (0xFFU)

static ed_event_t s_pool[ED_EVENT_POOL_SIZE];
static uint8_t s_free_head;
static uint8_t s_used_count;

uint8_t ed_event_index_of(const ed_event_t* event)
{
    if ((event < &s_pool[0]) || (event > &s_pool[ED_EVENT_POOL_SIZE - 1U])){
        return ED_EVENT_NULL_INDEX;
    }
    return (uint8_t)(event - &s_pool[0]);
}

ed_event_t* ed_event_from_index(uint8_t index) {
    if (index >= ED_EVENT_POOL_SIZE) {
        return (ed_event_t*)0;
    }
    return &s_pool[index];
}

void ed_event_init(void){
    for (uint8_t i = 0; i < ED_EVENT_POOL_SIZE; i++) {
        s_pool[i].next = (i == (ED_EVENT_POOL_SIZE - 1U)) ? ED_EVENT_NULL_INDEX : (uint8_t)(i + 1U);
        s_pool[i].len = 0;
    }

    s_free_head = 0;
    s_used_count = 0;
}

ed_event_t* ed_event_alloc(void)
{
    if (s_free_head == ED_EVENT_NULL_INDEX){
        return (ed_event_t*)0;
    }

    // get address event from index
    ed_event_t* event = ed_event_from_index(s_free_head);
    s_free_head = event->next;

    event->next = ED_EVENT_NULL_INDEX;
    event->type = ED_EVENT_TYPE_PURE;
    event->src = 0;
    event->des = 0;
    event->sig = 0;
    event->len = 0;

    s_used_count++;

    return event;
}

void ed_event_free(ed_event_t* event) {
    if (event == (ed_event_t*)0) {
        return;
    }

    uint8_t index = ed_event_index_of(event);    // get index event in s_pool array
    if (index == ED_EVENT_NULL_INDEX) {
        return;
    }

    event->next = s_free_head;
    event->len = 0;
    s_free_head = index;

    if (s_used_count > 0U) {
        s_used_count--;
    }
}

bool ed_event_set_payload(ed_event_t* event, const uint8_t* data, uint8_t len) {
    if ((event == (ed_event_t*)0) || (data == (const uint8_t*)0) || (len > ED_EVENT_PAYLOAD_MAX)) {
        return false;
    }

    memcpy(event->payload, data, len);
    event->type = ED_EVENT_TYPE_FIXED;
    event->len = len;

    return true;
}

// Get free slot number in s_pool
uint8_t ed_event_free_count(void) {
    return (uint8_t)(ED_EVENT_POOL_SIZE - s_used_count);
}