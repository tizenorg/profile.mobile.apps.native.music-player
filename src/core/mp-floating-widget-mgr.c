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

#include "mp-floating-widget-mgr.h"

#define FLOATING_WIDGET_MAX 3

typedef struct _FwItemData {
	int content_h;
	int positon;
	mp_floaing_widget_cb cb;
	void *data;
	bool visible;
} MpFwItemData;

struct MpFWMgr_t {
	Evas_Object *genlist;
	MpFwItemData Item[FLOATING_WIDGET_MAX];
} MpFWMgr_t;


static inline void __mp_floating_widget_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpFwMgr FwMgr = data;
	int posret;
	Elm_Object_Item *item = NULL;
	int index, x, y, w;

	evas_object_geometry_get(FwMgr->genlist, &x, &y, &w, NULL);

	/* First widget */
	item = elm_genlist_at_xy_item_get(FwMgr->genlist, x, y, &posret);
	index = elm_genlist_item_index_get(item);

	/* first widget */
	if (FwMgr->Item[0].cb) {
		if (index < FwMgr->Item[0].positon) {
			/* need to hide floating obj; */
			if (FwMgr->Item[0].visible) {
				FwMgr->Item[0].cb(false, x, y, w, FwMgr->Item[0].content_h, FwMgr->Item[0].data);
				FwMgr->Item[0].visible = false;
			}
		} else {
			/* need to show floating obj; */
			if (!FwMgr->Item[0].visible) {
				FwMgr->Item[0].cb(true, x, y, w, FwMgr->Item[0].content_h, FwMgr->Item[0].data);
				FwMgr->Item[0].visible = true;
			}
		}
	}

	/* Second widget */
	if (FwMgr->Item[1].cb) {

		/* height of second object; */
		int rel_h = y + FwMgr->Item[0].content_h;

		item = elm_genlist_at_xy_item_get(FwMgr->genlist, x, rel_h, &posret);
		if (elm_genlist_item_index_get(item) < FwMgr->Item[1].positon) {
			/* need to hide floating obj; */
			if (FwMgr->Item[1].visible) {
				FwMgr->Item[1].cb(false, x, rel_h, w, FwMgr->Item[1].content_h, FwMgr->Item[1].data);
				FwMgr->Item[1].visible = false;
			}
		} else {
			/* need to show floating obj; */
			if (!FwMgr->Item[1].visible) {
				FwMgr->Item[1].cb(true, x, rel_h, w, FwMgr->Item[1].content_h, FwMgr->Item[1].data);
				FwMgr->Item[1].visible = true;
			}
		}
	}

	/* Third widget */
	if (FwMgr->Item[2].cb) {
		if (index < FwMgr->Item[2].positon) {
			/* need to hide floating obj; */
			if (FwMgr->Item[2].visible) {
				FwMgr->Item[2].cb(false, x, y, w, FwMgr->Item[2].content_h, FwMgr->Item[2].data);
				FwMgr->Item[2].visible = false;
			}
		} else {
			/* need to show floating obj; */
			if (!FwMgr->Item[2].visible) {
				FwMgr->Item[2].cb(true, x, y, w, FwMgr->Item[2].content_h, FwMgr->Item[2].data);
				FwMgr->Item[2].visible = true;
			}
		}
	}


}

MpFwMgr mp_floating_widget_mgr_create(Evas_Object *genlist)
{
	MP_CHECK_NULL(genlist);
	MpFwMgr FwMgr = calloc(1, sizeof(MpFWMgr_t));
	MP_CHECK_NULL(FwMgr);

	DEBUG_TRACE("size: %d", sizeof(MpFWMgr_t));

	FwMgr->genlist = genlist;
	evas_object_smart_callback_add(genlist, "scroll", __mp_floating_widget_cb, FwMgr);
	evas_object_smart_callback_add(genlist, "realized", __mp_floating_widget_cb, FwMgr);

	return FwMgr;
}

void mp_floating_widget_mgr_destroy(MpFwMgr FwMgr)
{
	MP_CHECK(FwMgr);

	int index = 0;
	for (; index < FLOATING_WIDGET_MAX; index++) {
		mp_floating_widget_mgr_widget_deleted(FwMgr, index);
	}

	free(FwMgr);
}
/**
*	position - position in list
*	index - index between floating widgets
**/
void mp_floating_widget_callback_add(MpFwMgr FwMgr,
                                     int content_h, int position, int index, mp_floaing_widget_cb cb , void *data)
{
	MP_CHECK(FwMgr);
	if (index >= FLOATING_WIDGET_MAX) {
		ERROR_TRACE("Only 2 items are supported");
		return;
	}

	FwMgr->Item[index].positon = position;
	FwMgr->Item[index].content_h = content_h;
	FwMgr->Item[index].cb = cb;
	FwMgr->Item[index].data = data;

}

void mp_floating_widget_mgr_widget_deleted(MpFwMgr FwMgr, int index)
{
	MP_CHECK(FwMgr);
	if (index >= FLOATING_WIDGET_MAX) {
		ERROR_TRACE("Only 2 items are supported");
		return;
	}

	int x, y, w;
	evas_object_geometry_get(FwMgr->genlist, &x, &y, &w, NULL);
	/* DEBUG_TRACE("Genlist x[%d] y[%d]", x, y); */

	if (FwMgr->Item[index].cb) {
		FwMgr->Item[index].cb(false, x, y, w, FwMgr->Item[index].content_h, FwMgr->Item[index].data);
	}
	FwMgr->Item[index].visible = false;
}

bool mp_floating_widget_mgr_visible_get(MpFwMgr FwMgr, int index)
{
	MP_CHECK_FALSE(FwMgr);
	if (index >= FLOATING_WIDGET_MAX) {
		ERROR_TRACE("Only 2 items are supported");
		return false;
	}
	return FwMgr->Item[index].visible;
}

