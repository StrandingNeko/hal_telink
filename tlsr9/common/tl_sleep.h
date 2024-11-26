/******************************************************************************
 * Copyright (c) 2023-2024 Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#ifndef __TL_SLEEP_H
#define __TL_SLEEP_H

bool tl_suspend(uint32_t wake_stimer_tick);
#if (CONFIG_SOC_SERIES_RISCV_TELINK_B9X_RETENTION || \
CONFIG_SOC_SERIES_RISCV_TELINK_TLX_RETENTION)
bool tl_deep_sleep(uint32_t wake_stimer_tick);
#endif /* (CONFIG_SOC_SERIES_RISCV_TELINK_B9X_RETENTION || */
/* CONFIG_SOC_SERIES_RISCV_TELINK_TLX_RETENTION) */

#endif /* __TL_SLEEP_H */