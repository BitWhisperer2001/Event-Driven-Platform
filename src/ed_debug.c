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

#include "ed_debug.h"

#include <stdarg.h>
#include <stdio.h>

static ed_debug_output_t s_output_cb = (ed_debug_output_t)0;
static uint8_t s_enable = (ED_DEBUG_ENABLE == 1U) ? ED_ENABLE : ED_DISABLE;

void ed_debug_init(ed_debug_output_t output_cb) {
    s_output_cb = output_cb;
}

void ed_debug_set_enable(uint8_t enable) {
    s_enable = enable;
}

uint8_t ed_debug_is_enable(void) {
    return s_enable;
}

void ed_debug_printf(const char* fmt, ...) {
#if (ED_DEBUG_ENABLE == 1U)
    if ((s_enable == ED_DISABLE) || (s_output_cb == (ed_debug_output_t)0) || (fmt == (const char*)0)) {
        return;
    }

    char buf[ED_DEBUG_BUFFER_SIZE];

    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    s_output_cb(buf);
#else
    (void)fmt;
#endif
}