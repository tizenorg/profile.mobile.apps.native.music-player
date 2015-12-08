CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECT_NAME music-chooser)
PROJECT(${PROJECT_NAME} C CXX)

SET(SRCS
	src/music-chooser.c
	src/mc-select-view.c
	src/mc-track-list.c
	src/mc-voice-clip-list.c
	src/mc-group-play-list.c
	src/mc-group-list.c
	src/mc-library-view.c
	src/mc-common.c
	src/mc-widget.c
	src/mc-list-play.c
	src/mc-index.c
	src/mc-search-view.c
	src/mc-search-list.c
	src/mc-search.c
	${CMAKE_SOURCE_DIR}/src/common/mp-media-info.c
	${CMAKE_SOURCE_DIR}/src/common/mp-file-util.c
)

SET(COMMON_INCLUDE
	${CMAKE_SOURCE_DIR}/inc
	${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

INCLUDE_DIRECTORIES(${COMMON_INCLUDE})

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	efl-extension
	dlog
	vconf
	capi-appfw-application
	capi-appfw-preference
	capi-appfw-app-manager
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} ${flag}")
ENDFOREACH(flag)

message("MC_PREFIX is ${MC_PREFIX}")
message("CMAKE_BINARY_DIR is ${CMAKE_BINARY_DIR}")

SET(EXTRA_CXXFLAGS "${EXTRA_CXXFLAGS} -fvisibility=hidden -Wall")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXXFLAGS}")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -Wno-format-extra-args")
SET(CMAKE_LDFLAGS "-Wl,-zdefs" )
ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DMC_MUSIC_PLAYER")
ADD_DEFINITIONS("-DMC_ENABLE_TAB")
#ADD_DEFINITIONS("-DMC_AUTO_RECOMMENDED")

#ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRCS})
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} -lm)
#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.1.0)
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

SET(BINDIR "${MC_PREFIX}/bin")

# set extra defines
SET(IMAGES_DIR "${MC_PREFIX}/shared/trusted/music-chooser")
SET(MC_EDJ_NAME "${MC_PREFIX}/shared/trusted/music-chooser.edj")
ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")
ADD_DEFINITIONS("-DIMAGES_DIR=\"${IMAGES_DIR}\"")
ADD_DEFINITIONS("-DMC_EDJ_NAME=\"${MC_EDJ_NAME}\"")
ADD_CUSTOM_TARGET(music-chooser.edj
		COMMAND edje_cc -no-save -id ${CMAKE_CURRENT_SOURCE_DIR}/images
		${CMAKE_CURRENT_SOURCE_DIR}/layout/music-chooser.edc
		${CMAKE_BINARY_DIR}/res/edje/layout/music-chooser.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/layout/music-chooser.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} music-chooser.edj)

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${BINDIR})
INSTALL(DIRECTORY DESTINATION ${DATADIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/res/edje/layout/music-chooser.edj DESTINATION ${MC_PREFIX}/shared/trusted)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/org.tizen.music-chooser.xml DESTINATION ${DESKTOP_DIR}/packages)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/music-chooser.png DESTINATION /usr/share/icons/default/small)
