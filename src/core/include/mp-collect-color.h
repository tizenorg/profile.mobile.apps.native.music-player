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

#ifdef MP_FEATURE_ALBUM_COVER_BG

#ifndef __MP_COLLECT_COLOR_H__
#define __MP_COLLECT_COLOR_H__

#include "music.h"

/**
 * @brief collect specified count highest frequency colors of specified image file.
 * @param path [IN] path of image file.
 * @param resultCount [IN] color count need to collect.
 * @return rgb of colors, need to free by caller.
 */
int* mp_collect_color_set_image(const char *path, int resultCount);
int* mp_collect_color_get_RGB(int* color, int length);

#endif

#endif
