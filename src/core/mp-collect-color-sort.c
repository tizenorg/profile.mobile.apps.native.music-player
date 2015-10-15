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

#include "mp-collect-color-sort.h"

static void _mp_collect_color_swap(CollectColorNode_t *node_1, CollectColorNode_t *node_2)
{
	int temp;

	temp = node_1->key;
	node_1->key = node_2->key;
	node_2->key = temp;

	temp = node_1->value;
	node_1->value = node_2->value;
	node_2->value = temp;
}

static bool _mp_collect_color_sift_down(GList *node_list, int start, int end)
{
	MP_CHECK_FALSE(node_list);

	int root = start;
	int child;
	int swap;

	while (root * 2 + 1 <= end) {
		child = root * 2 + 1;
		swap = root;
		CollectColorNode_t *node_1 = g_list_nth_data(node_list, swap);
		CollectColorNode_t *node_2 = g_list_nth_data(node_list, child);
		if (node_1->value > node_2->value) {
			swap = child;
		}
		CollectColorNode_t *node_3 = g_list_nth_data(node_list, swap);
		CollectColorNode_t *node_4 = g_list_nth_data(node_list, child+1);
		if (child+1 <= end && node_3->value > node_4->value) {
			swap = child + 1;
		}
		if (swap != root) {
			CollectColorNode_t *node_5 = g_list_nth_data(node_list, swap);
			CollectColorNode_t *node_6 = g_list_nth_data(node_list, root);
			_mp_collect_color_swap(node_5, node_6);

			root = swap;
		} else
			return true;
	}
	return false;
}

void mp_collect_color_sort(GList *node_list)
{
	startfunc;

	MP_CHECK(node_list);

	int length = g_list_length(node_list);
	int end = length - 1;
	while (end > 0) {
		_mp_collect_color_swap((CollectColorNode_t *)(g_list_nth_data(node_list, 0)), (CollectColorNode_t *)(g_list_nth_data(node_list, end)));
		end--;
		_mp_collect_color_sift_down(node_list, 0, end);
	}
}

void mp_collect_color_make_tree(GList *node_list)
{
	startfunc;

	MP_CHECK(node_list);

	int length = g_list_length(node_list);
	int start = (length - 2) / 2;
	while (start >= 0) {
		_mp_collect_color_sift_down(node_list, start, length - 1);
		start--;
	}
}

void mp_collect_color_check_tree(GList *node_list)
{
	MP_CHECK(node_list);

	_mp_collect_color_sift_down(node_list, 0, g_list_length(node_list)-1);
}

#endif
