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


#include "music-chooser.h"
#include "mc-common.h"

Evas_Object *
mc_widget_no_content_add(Evas_Object *parent, NoContentType_e type)
{
	MP_CHECK_NULL(parent);
	Evas_Object *nocontents = NULL;
	nocontents = elm_layout_add(parent);
	elm_layout_theme_set(nocontents, "layout", "nocontents", "default");

	Evas_Object *icon = elm_image_add(nocontents);
	char *shared_path = app_get_shared_resource_path();
	char nocontent_mm[1024] = {0};
	snprintf(nocontent_mm, 1024, "%s%s/%s", shared_path, "shared_images", NOCONTENT_MULTIMEDIA);
	free(shared_path);
	elm_image_file_set(icon, nocontent_mm, NULL);
	elm_object_part_content_set(nocontents, "nocontents.image", icon);

	const char *ids;
	if (type == NO_CONTENT_PLAYLIST) {
		ids = MC_TEXT_NO_PLAYLIST;
	} else if (type == NO_CONTENT_ALBUM) {
		ids = MC_TEXT_NO_ALBUM;
	} else if (type == NO_CONTENT_ARTIST) {
		ids = MC_TEXT_NO_ARTIST;
	} else {
		ids = MC_TEXT_NO_SONGS;
	}

	//elm_object_text_set(nocontents, GET_STR(ids));
	mc_common_obj_domain_text_translate(nocontents, ids);
	elm_object_focus_allow_set(nocontents, EINA_TRUE);

	return nocontents;
}

static void
_mc_win_del(void *data, Evas_Object * obj, void *event)
{
	elm_exit();
}


Evas_Object *
mc_create_win(const char *name)
{
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request", _mc_win_del, NULL);
		elm_win_screen_size_get(eo, NULL, NULL, &w, &h);
		evas_object_resize(eo, w, h);
		//set indicator as transparent
		elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(eo, ELM_WIN_INDICATOR_TRANSPARENT);
		elm_win_conformant_set(eo, EINA_TRUE);
	}
	return eo;
}

Evas_Object *mc_widget_create_title_icon_btn(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data)
{
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/title_icon");

	ic = elm_image_add(parent);
	elm_image_file_set(ic, file, group);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	evas_object_show(btn);
	return btn;
}

Evas_Object *mc_widget_create_naviframe_toolbar(Elm_Object_Item *navi_it)
{
	Evas_Object *toolbar = elm_object_item_part_content_unset(navi_it, "toolbar");
	if (toolbar) {
		evas_object_del(toolbar);
		toolbar = NULL;
	}

	toolbar = elm_toolbar_add(elm_object_item_widget_get(navi_it));
	MP_CHECK_NULL(toolbar);

	elm_object_style_set(toolbar, "tabbar");
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_object_item_part_content_set(navi_it, "toolbar", toolbar);

	return toolbar;
}

Elm_Object_Item *mc_widget_create_toolbar_item_btn(Evas_Object *parent, const char *style, const char *text, Evas_Smart_Cb func, void *data)
{
	Evas_Object *toolbar = parent;
	MP_CHECK_NULL(toolbar);

	Elm_Object_Item *toolbar_item = NULL;
	//Evas_Object *toolbar_obj_item = NULL;

	toolbar_item = elm_toolbar_item_append(toolbar, NULL, text, func, data);
	//toolbar_obj_item = elm_toolbar_item_object_get(toolbar_item);
	return toolbar_item;
}
static inline const char *_mc_get_text_domain(const char *string_id)
{
	const char *domain = DOMAIN_NAME;

	return domain;
}

Evas_Object* mc_widget_create_navi_left_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{

	if (!pParent || !pNaviItem) {
		ERROR_TRACE("parent is NULL.");
		return NULL;
	}
	Evas_Object *pLeftbtn = NULL;
	pLeftbtn = elm_button_add(pParent);
	elm_object_style_set(pLeftbtn, "naviframe/title_left");
	evas_object_smart_callback_add(pLeftbtn, "clicked", pFunc, pUserData);
	const char *domain = _mc_get_text_domain(STR_MP_NAVI_CANCEL);
	elm_object_domain_translatable_text_set(pLeftbtn, domain, STR_MP_NAVI_CANCEL);

	if (!pLeftbtn) {
		ERROR_TRACE("[ERR] Fail to create pLeftbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_left_btn", pLeftbtn);

	evas_object_show(pLeftbtn);

	return pLeftbtn;
}

Evas_Object* mc_widget_create_navi_right_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{

	if (!pParent || !pNaviItem) {
		ERROR_TRACE("parent is NULL.");
		return NULL;
	}

	Evas_Object *pRightbtn = NULL;

	pRightbtn = elm_button_add(pParent);
	elm_object_style_set(pRightbtn, "naviframe/title_right");
	evas_object_smart_callback_add(pRightbtn, "clicked", pFunc, pUserData);
	const char *domain = _mc_get_text_domain(STR_MP_NAVI_DONE);
	elm_object_domain_translatable_text_set(pRightbtn, domain, STR_MP_NAVI_DONE);

	if (!pRightbtn) {
		ERROR_TRACE("[ERR] Fail to create pRightbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_right_btn", pRightbtn);
	elm_object_disabled_set(pRightbtn, EINA_TRUE);

	evas_object_show(pRightbtn);

	return pRightbtn;
}

