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
#ifndef SCAN_H_
#define SCAN_H_

typedef enum{
    SCAN_STRATEGY_0 = 0, /*!< The number of scans corresponds to the number of connections,
                                  that is to say, after the number of connections reaches the
                                  maximum allowed number, scan cannot be started. >!*/
    SCAN_STRATEGY_1 = 1, /*!< When 1m is configured, if a master establishes a link,
                                  scan continues to work and scan can be turned on and off normally by the scan enable API---
                                  "blc_ll_setExtScanEnable" or "blc_ll_setScanEnable". >!*/
}scan_en_strtg_t;


void        blc_ll_configScanEnableStrategy (scan_en_strtg_t scanStrategy);


#endif /* SCAN_H_ */
