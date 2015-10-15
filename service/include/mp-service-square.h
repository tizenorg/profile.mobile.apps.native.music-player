/* 
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License"); 
* you may not use this file except in compliance with the License. 
* You may obtain a copy of the License at 
* 
* http://www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS, 
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
* See the License for the specific language governing permissions and 
* limitations under the License. 
* 
*/


#ifndef __MP_APP_CONTROL_SQUARE_H_
#define __MP_APP_CONTROL_SQUARE_H_

#include "mp-service-debug.h"

mp_square_mgr_t *mp_app_control_square_mgr_create(void);
bool mp_app_control_square_mgr_destory(mp_square_mgr_t *square_mgr);
int mp_app_control_square_mgr_update(mp_square_mgr_t *square_mgr);
int mp_app_control_square_mgr_update_diff_only(mp_square_mgr_t *square_mgr);
int mp_app_control_square_mgr_reset(mp_square_mgr_t *square_mgr);
int mp_app_control_square_mgr_records_count_get(mp_square_mgr_t *square_mgr, int *count);

#endif /* __MP_SQUARE_MGR_H_ */
