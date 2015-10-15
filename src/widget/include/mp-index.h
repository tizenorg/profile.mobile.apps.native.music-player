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

#ifndef __MP_INDEX_H__
#define __MP_INDEX_H__

#include <Elementary.h>
#include <mp-list.h>

typedef void (*MpIndexCb)(void *data);

static char *non_latin_lan[] = {
     "ar_AE.UTF-8",
     "as_IN.UTF-8",
     "bg_BG.UTF-8",
     "bn_IN.UTF-8",
     "el_GR.UTF-8",
     "fa_IR.UTF-8",
     "gu_IN.UTF-8",
     "he_IL.UTF-8",
     "hi_IN.UTF-8",
     "hy_AM.UTF-8",
     "ja_JP.UTF-8",
     "ka_GE.UTF-8",
     "kk_KZ.UTF-8",
     "km_KH.UTF-8",
     "kn_IN.UTF-8",
     "ko_KR.UTF-8",
     "lo_LA.UTF-8",
     "mk_MK.UTF-8",
     "ml_IN.UTF-8",
     "mn_MN.UTF-8",
     "mr_IN.UTF-8",
     "ne_NP.UTF-8",
     "or_IN.UTF-8",
     "pa_IN.UTF-8",
     "ru_RU.UTF-8",
     "si_LK.UTF-8",
     "ta_IN.UTF-8",
     "te_IN.UTF-8",
     "th_TH.UTF-8",
     "uk_UA.UTF-8",
     "ur_PK.UTF-8",
     "zh_TW.UTF-8",
     NULL
};

Evas_Object *mp_index_create(Evas_Object *parent, int group_type, void *data);
void mp_index_append_item(Evas_Object *index, MpList_t *list);

#endif

