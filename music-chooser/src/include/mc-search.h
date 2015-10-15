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


#ifndef __MC_SEARCH_H_
#define __MC_SEARCH_H_
#include <Elementary.h>

Evas_Object *mc_search_create_new(Evas_Object * parent, Evas_Smart_Cb change_cb, void *change_cb_data, Evas_Smart_Cb cancel_cb, void *cancel_cb_data, Evas_Smart_Cb focus_cb, void *focus_cb_data, Evas_Smart_Cb unfocus_cb, void *unfocus_cb_data);
void mc_search_hide_imf_pannel(Evas_Object * search);
void mc_search_show_imf_pannel(Evas_Object * search);
Evas_Object *mc_search_entry_get(Evas_Object *search);
char *mc_search_text_get(Evas_Object *search);
void mc_search_text_set(Evas_Object *search, const char *text);

#endif //__MP_SEARCH_H_
