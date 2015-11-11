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


#ifndef	__MP_SMART_EVENT_BOX_H_
#define	__MP_SMART_EVENT_BOX_H_

typedef enum {
	MP_EVENT_CLICK,
	MP_EVENT_LEFT,
	MP_EVENT_RIGHT,
	MP_EVENT_MAX,
} MpEventCallback_e;

Evas_Object *mp_smart_event_box_add(Evas_Object * parent);
void mp_smart_event_box_callback_add(Evas_Object *event_box, MpEventCallback_e event, void (*event_cb)(void *), void *user_data);

#endif //__MP_SMART_EVENT_BOX_H_
