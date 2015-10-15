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

#ifdef MP_ENABLE_INOTIFY
/* #define MP_WATCH_FLAGS IN_CREATE | IN_DELETE | IN_CLOSE_WRITE | IN_MODIFY */
#define MP_WATCH_FLAGS IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_CLOSE_WRITE | IN_MOVED_TO

#define MP_EVENT_SIZE  (sizeof (struct inotify_event))
/** reasonable guess as to size of 1024 events */
#define MP_EVENT_BUF_LEN (1024 * (MP_EVENT_SIZE + 16))
#define INOTI_FOLDER_COUNT_MAX 1024

typedef struct _mp_inotify_t {
	int fd;
	GList *wd_list;
	unsigned int prev_event;
	pthread_t monitor;
	mp_inotify_cb callback;
	void *u_data;
} mp_inotify_t;

static pthread_mutex_t mp_noti_lock;
static mp_inotify_t *g_handle;

static void
_mp_app_inotify_handle_free(void)
{
	pthread_mutex_destroy(&mp_noti_lock);

	if (g_handle) {
		if (g_handle->fd >= 0) {
			close(g_handle->fd);
			g_handle->fd = -1;
		}
		g_free(g_handle);
		g_handle = NULL;
	}

	return;
}

static mp_inotify_t *
_mp_app_inotify_handle_init(void)
{
	_mp_app_inotify_handle_free();
	g_handle = g_new0(mp_inotify_t, 1);

	if (g_handle) {
		g_handle->fd = -1;
		pthread_mutex_init(&mp_noti_lock, NULL);
	}

	return g_handle;
}

static void
_mp_app_inotify_thread_clean_up(void *data)
{
	pthread_mutex_t *lock = (pthread_mutex_t *) data;
	DEBUG_TRACE("Thread cancel Clean_up function");
	if (lock) {
		pthread_mutex_unlock(lock);
	}
	return;
}


static gpointer
_mp_app_inotify_watch_thread(gpointer user_data)
{
	mp_inotify_t *handle = (mp_inotify_t *) user_data;
	int oldtype = 0;

	mp_retvm_if (handle == NULL, NULL, "handle is NULL");
	DEBUG_TRACE("Create _mp_app_inotify_watch_thread!!! ");

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldtype);

	while (1) {
		ssize_t len = 0;
		ssize_t i = 0;
		char event_buff[MP_EVENT_BUF_LEN] = { 0, };

		if (handle->fd < 0) {
			ERROR_TRACE("fd is not a vaild one");
			pthread_exit(NULL);
		}

		len = read(handle->fd, event_buff, sizeof(event_buff) - 1);
		if (len <= 0 || len > sizeof(event_buff) - 1) {

		}

		while (i < len) {
			struct inotify_event *pevent = (struct inotify_event *)&event_buff[i];
			mp_inotify_event s_event = MP_INOTI_NONE;

			if (pevent->len && strncmp(pevent->name, ".", 1) == 0) {
				s_event = MP_INOTI_NONE;
			} else if (pevent->mask & IN_ISDIR)	/* directory */{
				/*
				if (pevent->mask & IN_DELETE_SELF)
				s_event = MP_INOTI_DELETE_SELF;

				if (pevent->mask & IN_MOVE_SELF)
				s_event = MP_INOTI_MOVE_SELF;

				if (pevent->mask & IN_CREATE)
				s_event = MP_INOTI_CREATE;

				if (pevent->mask & IN_DELETE)
				s_event = MP_INOTI_DELETE;

				if (pevent->mask & IN_MOVED_FROM)
				s_event = MP_INOTI_MOVE_OUT;

				if (pevent->mask & IN_MOVED_TO)
				s_event = MP_INOTI_MOVE_IN;
				 */
			} else	/* file */{
				if (pevent->mask & IN_CREATE) {
					s_event = MP_INOTI_NONE;
					handle->prev_event = IN_CREATE;
				}

				if (pevent->mask & IN_CLOSE_WRITE) {
					if (handle->prev_event == IN_CREATE) {
						s_event = MP_INOTI_CREATE;
					}
					handle->prev_event = MP_INOTI_NONE;
				}

				if (pevent->mask & IN_DELETE)
					s_event = MP_INOTI_DELETE;

				if (pevent->mask & IN_MODIFY) {
					s_event = MP_INOTI_MODIFY;
				}

				if (pevent->mask & IN_MOVED_TO) {
					s_event = MP_INOTI_MOVE_OUT;
				}
			}

			if (s_event != MP_INOTI_NONE) {
				pthread_cleanup_push(_mp_app_inotify_thread_clean_up, (void *)&mp_noti_lock);
				pthread_mutex_lock(&mp_noti_lock);
				if (handle->callback) {
					handle->callback(s_event, (pevent->len) ? pevent->name : NULL, handle->u_data);
				}
				pthread_mutex_unlock(&mp_noti_lock);
				pthread_cleanup_pop(0);
			}

			i += sizeof(struct inotify_event) + pevent->len;

			if (i >= MP_EVENT_BUF_LEN)
				break;
		}
	}

	DEBUG_TRACE("end _mp_app_inotify_watch_thread!!! ");

	return NULL;
}

Ecore_Timer *_g_inotyfy_timer = NULL;

static Eina_Bool
_mp_app_inotify_timer_cb(void *data)
{
	TIMER_TRACE();
	bool b_invalid_playing_file = false;
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_FALSE(ad);

	if (ad->edit_in_progress) {
		DEBUG_TRACE("editing in progress. not refresh list...");
		return false;
	}

	DEBUG_TRACE("update view");

	mp_plst_item *current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);

	if (current_item) {
		if (mp_util_is_streaming(current_item->uri)) {
			mp_debug("http uri path");
		} else if (!g_file_test(current_item->uri, G_FILE_TEST_EXISTS)) {
			mp_play_stop_and_updateview(ad, FALSE);
			b_invalid_playing_file = true;
		}
	}

#ifndef MP_SOUND_PLAYER
	mp_library_update_view(ad);
#endif
	_g_inotyfy_timer = NULL;
	return EINA_FALSE;
}

static void
_mp_app_inotify_cb(mp_inotify_event event, char *path, void *data)
{
	DEBUG_TRACE("file operation occured...");

	struct appdata *ad = (struct appdata *)data;

	MP_CHECK(path);

	ecore_pipe_write(ad->inotify_pipe, path, strlen(path));
}

void
_mp_app_inotify_add_recursive_watch(const char *path, void *ad)
{

	DIR *dp = NULL;
	struct dirent *entry = NULL;
	char *sub_path = NULL;
	sub_path = strdup(path);
	if (mp_app_inotify_add_watch(sub_path, _mp_app_inotify_cb, ad) < 0) {
		IF_FREE(sub_path);
		return;
	}

	dp = opendir(sub_path);
	if (dp == NULL)
		return;

	while ((entry = (struct dirent *)readdir(dp)) != NULL) {
		if (entry->d_name[0] == '.')
			continue;

		IF_FREE(sub_path);
		sub_path = g_strdup_printf("%s/%s", path, entry->d_name);
		if (entry->d_type == DT_DIR)
			_mp_app_inotify_add_recursive_watch(sub_path, ad);
	}
	IF_FREE(sub_path);

	closedir(dp);

}

static void
_mp_app_pipe_cb(void *data, void *path, unsigned int nbyte)
{
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	DEBUG_TRACE("%s modified..", path);
	mp_retm_if (ad->app_is_foreground, "Do not refresh list");

	if (_g_inotyfy_timer)
		ecore_timer_del(_g_inotyfy_timer);
	_g_inotyfy_timer = ecore_timer_add(0.5, _mp_app_inotify_timer_cb, data);


}

static void
_mp_add_inofity_refresh_watch(struct appdata *ad)
{
	mp_inotify_t *handle = NULL;
	handle = g_handle;

	MP_CHECK(handle);

	GList *wd_list = handle->wd_list;
	while (wd_list) {
		if (wd_list->data >= 0)
			mp_app_inotify_rm_watch((int)wd_list->data);
		wd_list = g_list_delete_link(wd_list, wd_list);
	}

	_mp_app_inotify_add_recursive_watch(MP_MMC_ROOT_PATH, ad);
	_mp_app_inotify_add_recursive_watch(MP_PHONE_ROOT_PATH, ad);

}

int
mp_app_inotify_init(void *data)
{

	struct appdata *ad = data;

	mp_inotify_t *handle = NULL;
	handle = _mp_app_inotify_handle_init();
	mp_retvm_if (handle == NULL, -1, "fail to _mp_app_inotify_handle_init()");

	handle->fd = inotify_init();

	if (handle->fd < 0) {
		switch (errno) {
		case EMFILE:
			ERROR_TRACE("The user limit on the total number of inotify instances has been reached.\n");
			break;
		case ENFILE:
			ERROR_TRACE("The system limit on the total number of file descriptors has been reached.\n");
			break;
		case ENOMEM:
			ERROR_TRACE("Insufficient kernel memory is available.\n");
			break;
		default:
			ERROR_TRACE("Fail to inotify_init(), Unknown error.\n");
			break;
		}
		return -1;
	}
	pthread_create(&handle->monitor, NULL, _mp_app_inotify_watch_thread, handle);

	_mp_app_inotify_add_recursive_watch(MP_MMC_ROOT_PATH, ad);
	_mp_app_inotify_add_recursive_watch(MP_PHONE_ROOT_PATH, ad);

	ad->inotify_pipe = ecore_pipe_add(_mp_app_pipe_cb, (const void *)ad);

	return 0;
}

int
mp_app_inotify_add_watch(const char *path, mp_inotify_cb callback, void *user_data)
{
	mp_inotify_t *handle = NULL;
	GList *wd_list;
	int wd;

	handle = g_handle;
	MP_CHECK_VAL(handle, -1);

	pthread_mutex_lock(&mp_noti_lock);

	wd_list = handle->wd_list;
	wd = inotify_add_watch(handle->fd, path, MP_WATCH_FLAGS);
	if (wd < 0) {
		switch (errno) {
		case EACCES:
			ERROR_TRACE("Read access to the given file is not permitted.\n");
			break;
		case EBADF:
			ERROR_TRACE("The given file descriptor is not valid.\n");
			handle->fd = -1;
			break;
		case EFAULT:
			ERROR_TRACE("pathname points outside of the process's accessible address space.\n");
			break;
		case EINVAL:
			ERROR_TRACE
				("The given event mask contains no legal events; or fd is not an inotify file descriptor.\n");
			break;
		case ENOMEM:
			ERROR_TRACE("Insufficient kernel memory is available.\n");
			break;
		case ENOSPC:
			ERROR_TRACE
				("The user limit on the total number of inotify watches was reached or the kernel failed to allocate a needed resource.\n");
			break;
		default:
			ERROR_TRACE("Fail to mp_inotify_add_watch(), Unknown error.\n");
			break;
		}
		pthread_mutex_unlock(&mp_noti_lock);
		return -1;
	}

	wd_list = g_list_append(wd_list, (gpointer)wd);
	if (!wd_list) {
		DEBUG_TRACE("g_list_append failed");
		pthread_mutex_unlock(&mp_noti_lock);
		return -1;
	}

	handle->callback = callback;
	handle->u_data = user_data;
	pthread_mutex_unlock(&mp_noti_lock);

	return 0;
}

int
mp_app_inotify_rm_watch(int wd)
{
	int ret = -1;
	mp_inotify_t *handle = NULL;

	handle = g_handle;
	mp_retvm_if (handle == NULL, -1, "handle is NULL");

	if (handle->fd < 0 || wd < 0) {
		WARN_TRACE
			("inotify is not initialized or has no watching dir - fd [%d] wd [%d]",
			 handle->fd, wd);
		return 0;
	}

	pthread_mutex_lock(&mp_noti_lock);

	ret = inotify_rm_watch(handle->fd, wd);
	if (ret < 0) {
		switch (errno) {
		case EBADF:
			ERROR_TRACE("fd is not a valid file descriptor\n");
			handle->fd = -1;
			break;
		case EINVAL:
			ERROR_TRACE("The watch descriptor wd is not valid; or fd is not an inotify file descriptor.\n");
			break;
		default:
			ERROR_TRACE("Fail to mp_inotify_add_watch(), Unknown error.\n");
			break;
		}
		pthread_mutex_unlock(&mp_noti_lock);
		return -1;
	}
	pthread_mutex_unlock(&mp_noti_lock);

	return 0;
}

static void
_mp_app_inotify_wd_list_destroy(gpointer data)
{
	mp_app_inotify_rm_watch((int)data);
}

void
mp_app_inotify_finalize(struct appdata *ad)
{
	mp_inotify_t *handle = NULL;
	handle = g_handle;

	mp_retm_if (handle == NULL, "handle is NULL");

	if (ad->inotify_pipe) {
		ecore_pipe_del(ad->inotify_pipe);
		ad->inotify_pipe = NULL;
	}

	g_list_free_full(handle->wd_list, _mp_app_inotify_wd_list_destroy);
	handle->wd_list = NULL;

	pthread_cancel(handle->monitor);
	pthread_join(handle->monitor, NULL);

	_mp_app_inotify_handle_free();

	return;
}
#endif

