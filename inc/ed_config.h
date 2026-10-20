#ifndef __ED_CONFIG_H__
#define __ED_CONFIG_H__

#include <stdint.h>

#define ED_ENABLE                          (1U)
#define ED_DISABLE                         (0U)

/* kernel sizing */
#define ED_TASK_MAX                        (16U)
#define ED_PRIORITY_MAX                    (8U)

/* event sizing */
#define ED_EVENT_POOL_SIZE                 (64U)
#define ED_EVENT_PAYLOAD_MAX               (32U)

/* timer sizing */
#define ED_TIMER_POOL_SIZE                 (32U)

/* scheduler behavior */
#define ED_TIMER_SERVICE_BUDGET_PER_STEP   (8U)

/* debug/log options (set 0 for release firmware) */
#define ED_DEBUG_ENABLE                    (0U)
#define ED_DEBUG_BUFFER_SIZE               (128U)

#endif /* __ED_CONFIG_H__ */