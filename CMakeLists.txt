CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(music-player C CXX)

EXEC_PROGRAM(uname ARGS -m OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)
message("Building by ${CMAKE_SYSTEM_VERSION} machine")

SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(LIBDIR "${PREFIX}/lib")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${DATA_PREFIX}/data")
SET(SHAREDDIR "${PREFIX}/shared")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons/default/small")
SET(EDJDIR "${RESDIR}/edje")
#SET(RICH_INFO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/rich_info")

message("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")
message("PREFIX ${PREFIX}")
message("BINDIR ${BINDIR}")
message("DATADIR ${DATADIR}")

#START_PUBLIC_DISABLE_FEATURE
#MP_FEATURE_ADD_TO_HOME
#MP_FEATURE_LANDSCAPE
#MP_FEATURE_CONTEXT_ENGINE
#MP_FEATURE_APP_IN_APP
#MP_3D_FEATURE
#MP_FEATURE_INNER_SETTINGS
#MP_FEATURE_DESKTOP_MODE
#MP_FEATURE_SHORTCUT
#MP_FEATURE_VOICE_CONTROL
#MP_FEATURE_LOCKSCREEN
#END_START_PUBLIC_DISABLE_FEATURE


# Build features
#ADD_DEFINITIONS("-DMP_FEATURE_APP_IN_APP")
#ADD_DEFINITIONS("-DMP_FEATURE_ALBUM_COVER_BG")
ADD_DEFINITIONS("-DMP_FEATURE_LOCKSCREEN")
ADD_DEFINITIONS("-DMP_FEATURE_PERSONAL_PAGE")
ADD_DEFINITIONS("-DMP_FEATURE_UNSUPPORT_DRM_CD_SD")
ADD_DEFINITIONS("-DMP_FEATURE_GL")
ADD_DEFINITIONS("-DMP_FEATURE_OPTIMIZATION_LAUNCH_TIME")
ADD_DEFINITIONS("-DMP_FEATURE_NO_END")
ADD_DEFINITIONS("-DGBSBUILD")

# USE VARIABLE DEFINE SERVICE MACRO
#SET(OPTION_MP_FEATURE_SUGGEST_FOR_YOU TRUE)
#IF(OPTION_MP_FEATURE_SUGGEST_FOR_YOU)
#	ADD_DEFINITIONS("-DMP_FEATURE_SUGGEST_FOR_YOU")
#	MESSAGE("### Add service process ###")
#ENDIF(OPTION_MP_FEATURE_SUGGEST_FOR_YOU)

IF (CMAKE_DISABLE_FEATURE_MMS)
	message("##Disable mms##")
	ADD_DEFINITIONS("-DMP_FEATURE_DISABLE_MMS")
#	ADD_DEFINITIONS("-DMP_FEATURE_ALBUMART_UPDATE")
ENDIF (CMAKE_DISABLE_FEATURE_MMS)
IF (CMAKE_ENABLE_FEATURE_DROPBOX)
	message("##enable dropbox##")
	ADD_DEFINITIONS("-DMP_FEATURE_ENABLE_DROPBOX")
ENDIF (CMAKE_ENABLE_FEATURE_DROPBOX)
#ADD_DEFINITIONS("-DMP_FEATURE_LANDSCAPE")

#IF (CMAKE_ENABLE_MULTIWINDOW)
#	message("##enable multiwindow##")
#	ADD_DEFINITIONS("-DMP_FEATURE_MULTIWINDOW")
#ENDIF (CMAKE_ENABLE_MULTIWINDOW)

IF (CMAKE_DEBUG_MODE)
	ADD_DEFINITIONS("-DMP_DEBUG_MODE")
#	ADD_DEFINITIONS("-DMP_FEATURE_ALBUMART_UPDATE")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
ENDIF (CMAKE_DEBUG_MODE)

SET(COMMON_SRCS
	src/mp-main.c

	src/core/mp-file-tag-info.c
	src/core/mp-player-control.c
	src/core/mp-player-mgr.c
	src/core/mp-player-drm.c
	src/core/mp-setting-ctrl.c
	src/core/mp-streaming-mgr.c
	src/core/mp-music-view-mgr.c
	src/core/mp-lyric-mgr.c
	src/core/mp-avrcp.c
	src/core/mp-volume.c
	src/core/mp-http-mgr.c
	src/core/mp-language-mgr.c
	src/core/mp-playlist-mgr.c
	src/core/mp-view-mgr.c
#	src/core/mp-cloud.c
	src/core/mp-floating-widget-mgr.c
	src/core/mp-watch-dog.c
	src/common/mp-common.c
	src/common/mp-edit-callback.c
	src/common/mp-item.c
	src/common/mp-menu.c
	src/common/mp-ug-launch.c
	src/common/mp-widget.c
	src/common/mp-util.c
	src/common/mp-play.c
	src/common/mp-app.c
	src/common/mp-media-info.c
	src/common/mp-ta.c
#	src/common/mp-recommend.c
#	src/common/mp-albumart.c
#	src/common/mp-db-client.c
	src/common/mp-file-util.c
#	src/view/mp-lyric-view.c
#	src/view/mp-mw-lyric-view.c
	src/widget/mp-lyric.c
	src/widget/mp-search.c
	src/widget/mp-smart-event-box.c
	src/widget/mp-popup.c
	src/widget/mp-minicontroller.c
	src/widget/mp-lockscreenmini.c
	src/widget/mp-ctxpopup.c
	src/widget/mp-player-volume-widget.c
	src/widget/mp-now-playing.c
	src/widget/mp-index.c
	src/widget/mp-scroll-page.c
	src/widget/mp-edit-playlist.c

	src/view/mp-all-view.c
	src/view/mp-add-track-view.c
	src/view/mp-select-track-view.c
	src/view/mp-create-playlist-view.c
	src/view/mp-album-detail-view.c
	src/view/mp-artist-detail-view.c
	src/view/mp-playlist-detail-view.c
	src/view/mp-folder-view.c
	src/view/mp-folder-detail-view.c
	src/view/mp-player-view.c
	src/view/mp-search-view.c
	src/view/mp-set-as-view.c
	src/view/mp-now-playing-list-view.c
#	src/view/mp-square-playlist-view.c
	src/view/mp-list-view.c
	src/view/mp-view.c
	src/view/mp-edit-view.c
	src/view/mp-genre-detail-view.c
#	src/view/mp-square-layout.c

	src/widget/mp-list.c
	src/widget/mp-track-list.c
	src/widget/mp-search-list.c
	src/widget/mp-playlist-list.c
	src/widget/mp-album-list.c
	src/widget/mp-genre-list.c
	src/widget/mp-album-detail-list.c
	src/widget/mp-artist-list.c
	src/widget/mp-artist-detail-list.c
	src/widget/mp-folder-list.c
	src/widget/mp-now-playing-list.c
#	src/widget/mp-square-list.c


#START_PUBLIC_REMOVED_STRING
#	src/core/mp-wifidirect-mgr.c
#	src/core/mp-music-view-mgr.c
#	src/core/mp-s-beam.c
#	src/core/mp-context.c
#	src/core/mp-voice-control-mgr.c
#	src/core/mp-crypto.c
	src/core/mp-collect-color-sort.c
	src/core/mp-collect-color.c
	src/view/mp-setting-view.c
	src/view/mp-detail-view.c
#	src/view/mp-album-booklet-view.c
#	src/view/mp-artist-booklet-view.c
	src/widget/mp-mini-player.c
	src/widget/mp-notification.c
#	src/view/mp-square-view-new.c
#	src/view/mp-make-offline-view.c
	src/widget/mp-notification.c
#END_START_PUBLIC_REMOVED_STRING
)


SET(COMMON_INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}/inc
	src/include
	src/common/include
	src/core/include
	src/list/include
	src/view/include
	src/widget/include
#START_PUBLIC_REMOVED_STRING
#	src/dali/include
	src/dmc-player/include
	src/setting/src/include
	#src/rich_info/src/include
	#src/rich_info/src/core/include
	src/lib/include
#END_START_PUBLIC_REMOVED_STRING
)

INCLUDE_DIRECTORIES(${COMMON_INCLUDE})

#SET(SRCS ${COMMON_SRCS} ${RICH_INFO_SRCS})
SET(SRCS ${COMMON_SRCS})

SET(REQUIRED_PKGS
	elementary
	cairo
	dlog
	vconf
	libxml-2.0
	#ecore-x
	efl-extension
	edje
	evas
	capi-appfw-application
	capi-appfw-preference
	capi-appfw-app-manager
	capi-media-sound-manager
	capi-telephony
	capi-media-player
	capi-media-metadata-extractor
	capi-content-media-content
	capi-content-mime-type
	minicontrol-provider
	capi-network-bluetooth
	capi-message-port
	capi-system-system-settings
	capi-system-media-key
	storage
#START_PUBLIC_REMOVED_STRING
	capi-web-url-download
	notification
	libcore-context-manager
	capi-network-wifi-direct
	capi-system-info
	capi-system-runtime-info
	capi-network-wifi
	capi-system-device
	db-util
#END_START_PUBLIC_REMOVED_STRING
)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${REQUIRED_PKGS})

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)
#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -finstrument-functions")
#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -DMUSIC_DEBUG")
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -DMACHINE_$ENV{MACHINE} ")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC  -fvisibility=hidden")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -Wall -g -fpie")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -Xassembler -W -fPIC")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DSHAREDDIR=\"${PREFIX}/shared\"")
ADD_DEFINITIONS("-DPKG_NAME=\"${PKG_NAME}\"")
ADD_DEFINITIONS("-DDATA_PREFIX=\"${DATA_PREFIX}\"")
ADD_DEFINITIONS("-DFACTORYFS=\"$ENV{FACTORYFS}\"")
ADD_DEFINITIONS("-DDESKTOP_ICON=\"${CMAKE_DESKTOP_ICON_DIR}/${PKG_NAME}.png\"")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})

MESSAGE("### CMAKE_SYSTEM_VERSION :: ${CMAKE_SYSTEM_VERSION} ###")
#START_PUBLIC_REMOVED_STRING
if("${CMAKE_SYSTEM_VERSION}" STREQUAL "arm" OR "${CMAKE_SYSTEM_VERSION}" STREQUAL "armv7l")
ADD_DEFINITIONS("-D__ARM__")
#ADD_DEFINITIONS("-DMP_3D_FEATURE")
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lstdc++ -lm -ldl)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} -ldali-adaptor -ldali-core -ldali-toolkit dali)

else()
#END_START_PUBLIC_REMOVED_STRING
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm -ldl)
#START_PUBLIC_REMOVED_STRING
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${CMAKE_SOURCE_DIR}/src/lib -lMusicSquare_Tizen_SL)
endif()
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${CMAKE_SOURCE_DIR}/src/lib -lsmat)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES INSTALL_RPATH ${LIBDIR})
#END_START_PUBLIC_REMOVED_STRING

ADD_CUSTOM_TARGET(mp-custom-winset-style.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player -sd ${CMAKE_CURRENT_SOURCE_DIR}/images/sounds
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-custom-winset-style.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-custom-winset-style.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-custom-winset-style.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-custom-winset-style.edj)

ADD_CUSTOM_TARGET(mp-library.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-library.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-library.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-library.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-library.edj)

ADD_CUSTOM_TARGET(music.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player -sd ${CMAKE_CURRENT_SOURCE_DIR}/images/sounds
		${CMAKE_SOURCE_DIR}/res/edje/layout/music.edc ${CMAKE_BINARY_DIR}/res/edje/layout/music.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/music.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} music.edj)

ADD_CUSTOM_TARGET(mp-minicontroller.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player -sd ${CMAKE_CURRENT_SOURCE_DIR}/images/sounds
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-minicontroller.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-minicontroller.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-minicontroller.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-minicontroller.edj)

ADD_CUSTOM_TARGET(mp-lockscreenmini.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player -sd ${CMAKE_CURRENT_SOURCE_DIR}/images/sounds
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-lockscreenmini.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-lockscreenmini.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-lockscreenmini.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-lockscreenmini.edj)

ADD_CUSTOM_TARGET(mp-images.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-images.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-images.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-images.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-images.edj)

ADD_CUSTOM_TARGET(mp-fake-effect.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images/music_player
		${CMAKE_SOURCE_DIR}/res/edje/layout/mp-fake-effect.edc ${CMAKE_BINARY_DIR}/res/edje/layout/mp-fake-effect.edj
		DEPENDS ${CMAKE_SOURCE_DIR}/res/edje/layout/mp-fake-effect.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} mp-fake-effect.edj)

ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
ADD_DEFINITIONS("-DDATADIR=\"${DATADIR}\"")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-DLIBDIR=\"${LIBDIR}\"")


INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(DIRECTORY DESTINATION ${DATA_PREFIX}/shared/data)
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-custom-winset-style.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/music.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-library.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-minicontroller.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-lockscreenmini.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-images.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-library.edj DESTINATION ${EDJDIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/mp-fake-effect.edj DESTINATION /usr/apps/${PKG_NAME}/shared/res)

# install desktop file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/${PKG_NAME}.png DESTINATION ${CMAKE_DESKTOP_ICON_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/preview_music_4x2.png DESTINATION ${CMAKE_DESKTOP_ICON_DIR})
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/shared/res/target_images/music_player DESTINATION ${RESDIR}/images)
INSTALL(FILES  ${CMAKE_SOURCE_DIR}/data/portrait.jpg DESTINATION ${SHAREDDIR}/res)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/shared/res/shared_images DESTINATION ${SHAREDDIR}/res)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/changeable_GUI DESTINATION ${SHAREDDIR}/res)

#install xml file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PKG_NAME}.xml DESTINATION ${DESKTOP_DIR}/packages)

# Share APP mo files with music chooser
SET(MC_MUSIC_PROJECT_NAME org.tizen.music-chooser)

# i18n
ADD_SUBDIRECTORY(po)

# Sound Player
ADD_SUBDIRECTORY(sound_player)

#START_PUBLIC_REMOVED_STRING
# music-chooser
ADD_SUBDIRECTORY(music-chooser)

# setting
ADD_SUBDIRECTORY(src/setting)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${CMAKE_BINARY_DIR}/src/setting -lsetting-music-player)

#install smack rule file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/${PKG_NAME}.efl DESTINATION  /etc/smack/accesses.d)
#END_START_PUBLIC_REMOVED_STRING

# music-player widget
ADD_SUBDIRECTORY(widget)

# log dump script
INSTALL(FILES ${CMAKE_SOURCE_DIR}/dump_${PKG_NAME}.sh DESTINATION ${CMAKE_LOG_DUMP_SCRIPT_DIR})

