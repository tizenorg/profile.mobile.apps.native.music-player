SET(MUSIC_PLAYER_WIDGET music-player-widget)

SET(SRCS
	src/mp-widget-main.c
	src/mp-widget-create.c
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

message("CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")

INCLUDE(FindPkgConfig)
pkg_check_modules(app_pkgs REQUIRED
	capi-appfw-application
	vconf
	elementary
	edje
	evas
	appcore-efl
	#ecore-x
	dlog
	widget
	widget_service
	widget_provider_app
	widget_provider
	capi-message-port
	notification
	capi-telephony
	capi-appfw-widget-application
)

FOREACH(flag ${app_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

ADD_DEFINITIONS(${app_pkgs_CFLAGS})
FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
    ADD_DEFINITIONS("-DTARGET")
    MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DLOG_TAG=\"WIDGET_APP\"")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/include")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie -Wl,-z,noexecstack")

ADD_EXECUTABLE(${MUSIC_PLAYER_WIDGET} ${SRCS})
TARGET_LINK_LIBRARIES(${MUSIC_PLAYER_WIDGET} ${app_pkgs_LDFLAGS} "-pie -lm")

#edj compile

INSTALL(TARGETS ${MUSIC_PLAYER_WIDGET} DESTINATION ${BINDIR})

#install application HOME directory
INSTALL(DIRECTORY DESTINATION ${DATADIR})
ADD_SUBDIRECTORY(res)
