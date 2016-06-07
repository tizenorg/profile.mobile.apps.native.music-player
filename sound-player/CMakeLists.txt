CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECT_NAME sound-player)
PROJECT(${PROJECT_NAME} C CXX)

SET(PACKAGE ${PROJECT_NAME})
SET(PKGNAME "${SP_PKG_NAME}")
SET(PREFIX "${INSTALL_DIR}/${PKGNAME}")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "${SP_DATA_PREFIX}/data")
SET(SHAREDDATADIR "${SP_DATA_PREFIX}/shared/data")
SET(LOCALEDIR "${RESDIR}/locale")
SET(ICONDIR "${RESDIR}/icons/default/small")
SET(EDJDIR "${RESDIR}/edje")
SET(RICH_INFO_DIR "${CMAKE_SOURCE_DIR}/src/rich_info")

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")


INCLUDE(FindPkgConfig)
pkg_check_modules(sp_pkgs REQUIRED
    elementary
    dlog
    storage
    vconf
    efl-extension
#   ecore-x
    edje
    evas
	capi-appfw-application
	capi-system-device
	capi-appfw-preference
	capi-appfw-app-manager
	capi-media-player
	capi-media-sound-manager
	capi-media-metadata-extractor
	capi-system-media-key
	capi-content-media-content
	capi-content-mime-type
	minicontrol-provider
	capi-network-bluetooth
	capi-message-port
	capi-system-system-settings
	notification
#	appcore-common

#START_PUBLIC_REMOVED_STRING
#	shortcut
#	capi-system-sensor
	libcore-context-manager
#	capi-context
#	capi-network-nfc
#	capi-telephony-sim
	#libsoup-2.4
#	voice-control-static
	capi-system-info
	capi-system-runtime-info
	capi-network-wifi
	#liballshare
	#liballshare-core
	capi-network-wifi-direct
	capi-web-url-download
	sqlite3
	db-util
	notification
	capi-telephony
#END_START_PUBLIC_REMOVED_STRING
)

INCLUDE_DIRECTORIES(
	${GLOBAL_INCLUDE}
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_SOURCE_DIR}/src/common/include
	${CMAKE_SOURCE_DIR}/src/core/include
	${CMAKE_SOURCE_DIR}/src/view/include
	${CMAKE_SOURCE_DIR}/src/widget/include

	${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

SET(SRCS
    ${CMAKE_SOURCE_DIR}/src/mp-main.c

	${CMAKE_SOURCE_DIR}/src/core/mp-file-tag-info.c
	${CMAKE_SOURCE_DIR}/src/core/mp-player-control.c
	${CMAKE_SOURCE_DIR}/src/core/mp-player-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-player-drm.c
	${CMAKE_SOURCE_DIR}/src/core/mp-setting-ctrl.c
	${CMAKE_SOURCE_DIR}/src/core/mp-playlist-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-streaming-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-lyric-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-http-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-avrcp.c
	${CMAKE_SOURCE_DIR}/src/core/mp-volume.c
	${CMAKE_SOURCE_DIR}/src/core/mp-view-mgr.c
#	${CMAKE_SOURCE_DIR}/src/core/mp-voice-control-mgr.c
	${CMAKE_SOURCE_DIR}/src/core/mp-watch-dog.c

	${CMAKE_SOURCE_DIR}/src/common/mp-item.c
	${CMAKE_SOURCE_DIR}/src/common/mp-menu.c
	${CMAKE_SOURCE_DIR}/src/common/mp-ug-launch.c
	${CMAKE_SOURCE_DIR}/src/common/mp-widget.c
	${CMAKE_SOURCE_DIR}/src/common/mp-util.c
	${CMAKE_SOURCE_DIR}/src/common/mp-file-util.c
	${CMAKE_SOURCE_DIR}/src/common/mp-play.c
	${CMAKE_SOURCE_DIR}/src/common/mp-app.c
	${CMAKE_SOURCE_DIR}/src/common/mp-media-info.c
	${CMAKE_SOURCE_DIR}/src/common/mp-common.c
#	${CMAKE_SOURCE_DIR}/src/common/mp-albumart.c

	${CMAKE_SOURCE_DIR}/src/view/mp-view.c
	${CMAKE_SOURCE_DIR}/src/view/mp-player-view.c
#	${CMAKE_SOURCE_DIR}/src/view/mp-lyric-view.c
	${CMAKE_SOURCE_DIR}/src/view/mp-detail-view.c
#	${CMAKE_SOURCE_DIR}/src/view/mp-set-as-view.c

	${CMAKE_SOURCE_DIR}/src/widget/mp-smart-event-box.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-popup.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-minicontroller.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-player-volume-widget.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-ctxpopup.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-scroll-page.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-lyric.c
	${CMAKE_SOURCE_DIR}/src/core/mp-language-mgr.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-mini-player.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-lockscreenmini.c
	${CMAKE_SOURCE_DIR}/src/core/mp-collect-color-sort.c
	${CMAKE_SOURCE_DIR}/src/core/mp-collect-color.c
#	${CMAKE_SOURCE_DIR}/src/common/mp-url-downloader.c
	${CMAKE_SOURCE_DIR}/src/widget/mp-notification.c
)


FOREACH(flag ${sp_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} ")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")

ADD_DEFINITIONS("-DMP_SOUND_PLAYER")
ADD_DEFINITIONS("-DPACKAGE=\"${PACKAGE}\"")
#ADD_DEFINITIONS("-DPREFIX=\"${PREFIX}\"")
#ADD_DEFINITIONS("-DRESDIR=\"${RESDIR}\"")
#ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
#ADD_DEFINITIONS("-DICONDIR=\"${ICONDIR}\"")
#ADD_DEFINITIONS("-DEDJDIR=\"${EDJDIR}\"")
ADD_DEFINITIONS("-D_FILE_OFFSET_BITS=64")
ADD_DEFINITIONS("-DSP_DATA_PREFIX=\"${SP_DATA_PREFIX}\"")

if("${CMAKE_SYSTEM_VERSION}" STREQUAL "arm" OR "${CMAKE_SYSTEM_VERSION}" STREQUAL "armv7l")
ADD_DEFINITIONS("-D__ARM__")
endif()

#SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -pie")
SET(CMAKE_LDFLAGS "-Wl,-zdefs" )

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${sp_pkgs_LDFLAGS} -lm)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} -L${CMAKE_SOURCE_DIR}/src/lib -lsmat)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(DIRECTORY DESTINATION ${SHAREDDATADIR})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/${PKGNAME}.png DESTINATION ${CMAKE_DESKTOP_ICON_DIR})

#START_PUBLIC_REMOVED_STRING
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${sp_pkgs_LDFLAGS} dmc-player)
#END_START_PUBLIC_REMOVED_STRING
