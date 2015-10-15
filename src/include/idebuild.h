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


#ifndef __MP_IDEBUILD_H_
#define __MP_IDEBUILD_H_

typedef struct _keynode_t {
    char *keyname;           /**< Keyname for keynode */
    int type;                /**< Keynode type */
    union {
        int i;               /**< Integer type */
        int b;               /**< Bool type */
        double d;            /**< Double type */
        char *s;             /**< String type */
    } value;                 /**< Value for keynode */
    struct _keynode_t *next; /**< Next keynode */
} keynode_t;

#define MH_SCROLL_MAX 3
//int app_control_set_window(app_control_h, Ecore_X_Window);

#endif
