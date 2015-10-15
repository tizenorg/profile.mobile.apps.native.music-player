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
#ifdef MP_COLLECT_COLOR_TIME_DEBUG
#include <time.h>
#endif

#define MP_COLOR_MASK (0xFFE0E0E0)
#define MP_TARGET_SIZE (60)
typedef struct {
	const char *path;
	int result_count;
	int find_result_index;
	int width;
	int height;
	unsigned char *img_target;
	GHashTable *hash_table;
	GList *node_list;

} CollectColor_t;

typedef unsigned char uchar;
typedef struct _rgb888 {
	uchar r;
	uchar g;
	uchar b;
} rgb888;
typedef struct _rgba8888 {
	uchar r;
	uchar g;
	uchar b;
	uchar a;
} rgba8888;
static const int RGBA_BPP = 4;
static const int RGB_BPP = 3;

int *mp_collect_color_get_RGB(int *color, int length)
{
	int* result = (int *)calloc(length*3, sizeof(int));
	mp_assert(result);

	int index = 0;
	int i = 0;
	for (; i < length; i++) {
		result[index++] = (color[i] & 0x00FF0000) >> 16;
		result[index++] = (color[i] & 0x0000FF00) >> 8;
		result[index++] = color[i] & 0x000000FF;
	}
	return result;
}

static int _mp_collect_color_get_int(int r, int g, int b)
{
	int result = (0xFF000000) | (r << 16) | (g << 8) | b;
	return result;
}


static int _mp_collect_color_get_sim_average(int color)
{
	startfunc;

	int multi = 8;
	int r = (color & 0x00FF0000) >> 16;
	int g = (color & 0x0000FF00) >> 8;
	int b = color & 0x000000FF;
	int average = (r + g + b) / 3 / 2 / multi;

	if (rand() % 2 == true)
		r = r + (rand() % average) * multi;
	else
		r = r - (rand() % average) * multi;
	if (r > 255)
		r = 255;
	else if (r < 0)
		r = 0;

	if (rand() % 2 == true)
		g = g + (rand() % average) * multi;
	else
		g = g - (rand() % average) * multi;
	if (g > 255)
		g = 255;
	else if (g < 0)
		g = 0;

	if (rand() % 2 == true)
		b = b + (rand() % average) * multi;
	else
		b = b - (rand() % average) * multi;
	if (b > 255)
		b = 255;
	else if (b < 0)
		b = 0;
	int result = _mp_collect_color_get_int(r, g, b);
	return result;
}


static int *_mp_collect_color_get_result(CollectColor_t *collect_color)
{
	startfunc;

	MP_CHECK_NULL(collect_color);
	GList *node_list = collect_color->node_list;
	MP_CHECK_NULL(node_list);

	int* result = (int *)calloc(collect_color->result_count, sizeof(int));
	mp_assert(result);

	DEBUG_TRACE("result_count %d, find_result_index %d", collect_color->result_count, collect_color->find_result_index);
	int i = 0;
	for (; i < collect_color->result_count; i++) {
		if (collect_color->find_result_index <= i) {
			result[i] = _mp_collect_color_get_sim_average(result[0]);
			continue;
		}
	CollectColorNode_t *node = g_list_nth_data(node_list, i);
	result[i] = node->key;
	DEBUG_TRACE("key=0x%x, val=%d", node->key, node->value);
}
	return result;
}

static bool _mp_collect_color_insert(int key, int value, GList *node_list)
{
	CollectColorNode_t *node = g_list_nth_data(node_list, 0);
	if (node->value < value) {
		node->key = key;
		node->value = value;
		mp_collect_color_check_tree(node_list);
		return true;
	}
	return false;
}

static GList *_mp_collect_color_copy_to_array(CollectColor_t *collect_color)
{
	startfunc;

	MP_CHECK_NULL(collect_color);

	GList *node_list = NULL;
	int nodes_index = 0;
	int temp_key;
	int temp_val;
	bool nodesIsFull = false;
	GHashTableIter iter;
	gpointer key, value;

	g_hash_table_iter_init (&iter, collect_color->hash_table);
	while (g_hash_table_iter_next (&iter, &key, &value)) {
		temp_key = *((int *)key);
		temp_val = *((int *)value);
		if (nodesIsFull) {
			_mp_collect_color_insert(temp_key, temp_val, node_list);
		} else {
			CollectColorNode_t *node = (CollectColorNode_t *)calloc(1, sizeof(CollectColorNode_t));
			mp_assert(node);
			node->key = temp_key;
			node->value = temp_val;
			node_list = g_list_append(node_list, node);
			nodes_index++;
			if (nodes_index == collect_color->result_count) {
				nodesIsFull = true;
				mp_collect_color_make_tree(node_list);
			}
		}
	}

	collect_color->find_result_index = nodes_index;
	mp_collect_color_make_tree(node_list);
	return node_list;
}
static void _mp_collect_color_add_hash(GHashTable *hash_table, int pixel)
{
	MP_CHECK(hash_table);

	pixel &= MP_COLOR_MASK;

	int *ret = (int *)g_hash_table_lookup(hash_table, &pixel);
	gint *key = g_new(gint, 1);
	*key = pixel;
	gint *value = g_new(gint, 1);
	if (ret) {
		*value = *ret+1;
		g_hash_table_replace(hash_table, key, value);
	} else {
		*value = 1;
		g_hash_table_insert(hash_table, key, value);
	}
}

static void _mp_collect_color_run_detail(CollectColor_t *collect_color)
{
	startfunc;

	MP_CHECK(collect_color);

	int key = 0;
	int i = 0;
	int j = 0;
	int src_idx = 0;
	unsigned char *src = (unsigned char *)collect_color->img_target;
	for (i = 0; i < collect_color->height; i++) {
		for (j = 0; j < collect_color->width; j++) {
			src_idx = (i * collect_color->width+j) * 3;
			key = _mp_collect_color_get_int(*(src+src_idx), *(src+src_idx+1), *(src+src_idx+2));
			_mp_collect_color_add_hash(collect_color->hash_table, key);
		}
	}
}
static int _mp_collect_color_image_util_resize(unsigned char *dest, int *dest_width , int *dest_height, const unsigned char *src, int src_width, int src_height, const image_util_colorspace_e colorspace)
{
	if (!dest || !dest_width || !dest_height || !src)
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	int src_w = src_width;
	int src_h = src_height;
	int dest_w = *dest_width;
	int dest_h = *dest_height;
	if ((IMAGE_UTIL_COLORSPACE_RGB888 != colorspace && IMAGE_UTIL_COLORSPACE_RGBA8888 != colorspace) || src_w <= 0 || src_h <= 0 || dest_w <= 0 || dest_h <= 0) 775
		return IMAGE_UTIL_ERROR_INVALID_PARAMETER;
	const unsigned int bpp = (IMAGE_UTIL_COLORSPACE_RGBA8888 == colorspace ? RGBA_BPP : RGB_BPP);
	unsigned int src_stride = bpp * src_w;
	unsigned int dest_stride = bpp * dest_w;
	int h = 0, w = 0;
	float t, u, coef;
	t = u = coef = 0.0;
	float c1, c2, c3, c4;
	c1 = c2 = c3 = c4 = 0.0;
	u_int32_t red, green, blue, alpha;
	red = green = blue = alpha = 0;
	int i = 0, j = 0;
	for (j = 0; j < dest_h; j++) {
		coef = (float) (j) / (float) (dest_h - 1) * (src_h - 1);
		h = (int) floor(coef);
		if (h < 0) {
			h = 0;
		} else {
			if (h >= src_h - 1) {
				h = src_h - 2;
			}
		}
		u = coef - h;
		for (i = 0; i < dest_w; i++) {
			coef = (float) (i) / (float) (dest_w - 1) * (src_w - 1);
			w = (int) floor(coef);
			if (w < 0) {
				w = 0;
			} else {
				if (w >= src_w - 1) {
					w = src_w - 2;
				}
			}
			t = coef - w;
			c1 = (1 - t) * (1 - u);
			c2 = t * (1 - u);
			c3 = t * u;
			c4 = (1 - t) * u;
			if (IMAGE_UTIL_COLORSPACE_RGBA8888 == colorspace) {
				rgba8888 pixel1 = *((rgba8888 *) (src + h * src_stride + bpp * w));
				rgba8888 pixel2 = *((rgba8888 *) (src + h * src_stride + bpp * (w + 1)));
				rgba8888 pixel3 = *((rgba8888 *) (src + (h + 1) * src_stride + bpp * (w + 1)));
				rgba8888 pixel4 = *((rgba8888 *) (src + (h + 1) * src_stride + bpp * w));
				red = pixel1.r * c1 + pixel2.r * c2 + pixel3.r * c3 + pixel4.r * c4;
				green = pixel1.g * c1 + pixel2.g * c2 + pixel3.g * c3 + pixel4.g * c4;
				blue =  pixel1.b * c1 + pixel2.b * c2 + pixel3.b * c3 + pixel4.b * c4;
				alpha = pixel1.a * c1 + pixel2.a * c2 + pixel3.a * c3 + pixel4.a * c4;
				rgba8888 *pixel_res = (rgba8888 *)(dest + bpp * i + j * dest_stride);
				pixel_res->r = red;
				pixel_res->g = green;
				pixel_res->b = blue;
				pixel_res->a = alpha;
			} else /* (IMAGE_UTIL_COLORSPACE_RGB888 == colorspace)  853 */{
				rgb888 pixel1 = *((rgb888 *) (src + h * src_stride + bpp * w));
				rgb888 pixel2 = *((rgb888 *) (src + h * src_stride + bpp * (w + 1)));
				rgb888 pixel3 = *((rgb888 *) (src + (h + 1) * src_stride + bpp * (w + 1)));
				rgb888 pixel4 = *((rgb888 *) (src + (h + 1) * src_stride + bpp * w));
				red = pixel1.r * c1 + pixel2.r * c2 + pixel3.r * c3 + pixel4.r * c4;
				green = pixel1.g * c1 + pixel2.g * c2 + pixel3.g * c3 + pixel4.g * c4;
				blue =  pixel1.b * c1 + pixel2.b * c2 + pixel3.b * c3 + pixel4.b * c4;
				rgb888 *pixel_res = (rgb888 *)(dest + bpp * i + j * dest_stride);
				pixel_res->r = red;
				pixel_res->g = green;
				pixel_res->b = blue;
			}
		}
	}
	return IMAGE_UTIL_ERROR_NONE;
}

static void _mp_collect_color_decode_image(CollectColor_t *collect_color)
{
	startfunc;

	MP_CHECK(collect_color);

	int width = 0;
	int height = 0;
	int resized_width = MP_TARGET_SIZE;
	int resized_height = MP_TARGET_SIZE;
	unsigned int size_decode = 0;
	unsigned char *img_source = 0;
	const image_util_colorspace_e colorspace = IMAGE_UTIL_COLORSPACE_RGB888;

	image_util_decode_jpeg(collect_color->path, colorspace, &img_source, &width, &height, &size_decode);
	DEBUG_TRACE("image_util_decode_jpeg path %s, orig width %d, orig height %d", collect_color->path, width, height);

	image_util_calculate_buffer_size(resized_width, resized_height, colorspace, &size_decode);
	collect_color->img_target = malloc(size_decode);

	int ret = _mp_collect_color_image_util_resize(collect_color->img_target, &resized_width, &resized_height,
		img_source, width, height, colorspace);
	DEBUG_TRACE("_mp_collect_color_image_util_resize ret %d", ret);
	SAFE_FREE(img_source);
	collect_color->width = resized_width;
	collect_color->height = resized_height;
	DEBUG_TRACE("resize width %d, resize height %d, size_decode %d", collect_color->width, collect_color->height, size_decode);
}

void _mp_collect_color_table_value_destory(gpointer data)
{
	SAFE_FREE(data);
}

void _mp_collect_color_table_key_destory(gpointer data)
{
	SAFE_FREE(data);
}

static GHashTable *_mp_collect_color_table_init()
{
	GHashTable *hash_table = NULL;

	hash_table = g_hash_table_new_full(g_int_hash, g_int_equal, _mp_collect_color_table_key_destory, _mp_collect_color_table_value_destory);
	return hash_table;
}

static CollectColor_t *_mp_collect_color_create()
{
	startfunc;

	CollectColor_t *collect_color = (CollectColor_t *)calloc(1, sizeof(CollectColor_t));
	mp_assert(collect_color);

	collect_color->hash_table = _mp_collect_color_table_init();
	MP_CHECK_NULL(collect_color->hash_table);
	g_hash_table_remove_all(collect_color->hash_table);
	srand(time(0));

	return collect_color;
}

static void _mp_collect_color_list_free_cb(gpointer data)
{
	CollectColorNode_t *node = (CollectColorNode_t *)data;

	SAFE_FREE(node);
}

static void _mp_collect_color_destroy(CollectColor_t *collect_color)
{
	startfunc;

	MP_CHECK(collect_color);

	if (collect_color->hash_table) {
		g_hash_table_remove_all(collect_color->hash_table);
		g_hash_table_destroy(collect_color->hash_table);
	}
	SAFE_FREE(collect_color->img_target);
	if (collect_color->node_list) {
		g_list_free_full(collect_color->node_list, _mp_collect_color_list_free_cb);
	}
	SAFE_FREE(collect_color);
}

#ifdef MP_COLLECT_COLOR_TIME_DEBUG
static int _mp_collect_color_get_time()
{
	struct timeval t;
	unsigned int tval = 0;
	gettimeofday(&t, NULL);

	tval =  t.tv_usec;/* t.tv_sec * 1000000L + */
	return tval;
}
#endif

int *mp_collect_color_set_image(const char *path, int resultCount)
{
	startfunc;
	if (resultCount < 1)
		return NULL;
#ifdef MP_COLLECT_COLOR_TIME_DEBUG
	int time_1 = _mp_collect_color_get_time();
#endif
	CollectColor_t *collect_color = _mp_collect_color_create();
	MP_CHECK_NULL(collect_color);

	collect_color->find_result_index = 0;
	collect_color->path = path;
	collect_color->result_count = resultCount;

	_mp_collect_color_decode_image(collect_color);
#ifdef MP_COLLECT_COLOR_TIME_DEBUG
	time_t time_2 = _mp_collect_color_get_time();
	DEBUG_TRACE("time_2-time_1 %ld, time_2 %ld, time_1 %ld", time_2-time_1, time_2, time_1);
#endif
	_mp_collect_color_run_detail(collect_color);
#ifdef MP_COLLECT_COLOR_TIME_DEBUG
	time_t time_3 = _mp_collect_color_get_time();
	DEBUG_TRACE("time_3-time_1 %ld", time_3-time_1);
#endif
	collect_color->node_list = _mp_collect_color_copy_to_array(collect_color);
	mp_collect_color_sort(collect_color->node_list);

	int *colors = _mp_collect_color_get_result(collect_color);
	_mp_collect_color_destroy(collect_color);
#ifdef MP_COLLECT_COLOR_TIME_DEBUG
	time_t time_4 = _mp_collect_color_get_time();
	DEBUG_TRACE("time_4-time_1 %ld", time_4-time_1);
#endif
	return colors;
}

#endif
