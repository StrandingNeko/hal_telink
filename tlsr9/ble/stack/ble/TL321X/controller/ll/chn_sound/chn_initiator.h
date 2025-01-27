/******************************************************************************
 * Copyright (c) 2024 Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef STACK_BLE_CONTROLLER_LL_CHN_SOUND_CHN_INITIATOR_H_
#define STACK_BLE_CONTROLLER_LL_CHN_SOUND_CHN_INITIATOR_H_


/**
 * @brief      for user to initialize channel sounding module of initiator role.
 * @return     status, 0x00:  succeed
 *                     other: failed
 */
ble_sts_t   blc_ll_initCsInitiatorModule(void);



#endif /* STACK_BLE_CONTROLLER_LL_CHN_SOUND_CHN_INITIATOR_H_ */
