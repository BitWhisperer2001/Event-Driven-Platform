#ifndef __ED_DEBUG_H__
#define __ED_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ed_config.h"

typedef void (*ed_debug_output_t)(const char* str);

void ed_debug_init(ed_debug_output_t output_cb);
void ed_debug_set_enable(uint8_t enable);
uint8_t ed_debug_is_enable(void);

void ed_debug_printf(const char* fmt, ...);

#if (ED_DEBUG_ENABLE == 1U)
#define ED_LOG(...) ed_debug_printf(__VA_ARGS__)
#else
#define ED_LOG(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ED_DEBUG_H__ */