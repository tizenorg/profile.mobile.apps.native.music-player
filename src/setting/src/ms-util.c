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

#include <sound_manager.h>
#include "ms-util.h"
#include "mp-vconf-private-keys.h"
#include "mp-common.h"
#include "mp-widget.h"
#include <runtime_info.h>

Eina_Bool
ms_util_is_earjack_connected(void)
{
	int ear_jack;
	if (runtime_info_get_value_int(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS, &ear_jack))
	{
		DEBUG_TRACE("Failed to get ear jack status");
		return FALSE;
	}

	if (ear_jack == RUNTIME_INFO_AUDIO_JACK_STATUS_UNCONNECTED)
		return FALSE;
	else
		return TRUE;
}

bool
ms_util_is_sound_device_connected(void)
{
	//Replaced for _prod dependency
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
	WARN_TRACE("Enter sound_manager_get_active_device");
	int ret;
	if (!(ret= sound_manager_get_current_device_list(g_device_mask,&g_device_list)))
		return true;
	else
		return false;
	//Replaced for _prod dependency end
}

static inline const char *_ms_util_get_text_domain(const char *string_id)
{
	const char *domain = "music-player";

	return domain;
}

void ms_util_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* text)
{
	const char *domain = _ms_util_get_text_domain(text);
	elm_object_domain_translatable_part_text_set(obj, part, domain, text);
}

void ms_util_domain_translatable_text_set(Evas_Object *obj, const char* text)
{
	const char *domain = _ms_util_get_text_domain(text);
	elm_object_domain_translatable_text_set(obj, domain, text);
}

void ms_util_object_item_translate_set(Elm_Object_Item *item, const char *ID)
{
	MP_CHECK(ID);
	MP_CHECK(item);
	const char *domain = _ms_util_get_text_domain(ID);
	elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
}
