CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(music-player-service C)

#LINK_DIRECTORIES(${CMAKE_BINARY_DIR})
#LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/server)

SET(DAEMON music-player-service)

INCLUDE(FindPkgConfig)
pkg_check_modules(music-player-app_control_pkgs REQUIRED
	capi-appfw-service-application
	dlog
	elementary
	storage
)

FOREACH(flag ${music-player-app_control_pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common/include
)

SET(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mp-service-main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mp-service-square.c
    ${CMAKE_SOURCE_DIR}/src/common/mp-media-info.c
)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
SET(music-player-app_control_pkgs_LDFLAGS "${pkgs_LDFLAGS} ${music-player-app_control_pkgs_LDFLAGS}")

ADD_EXECUTABLE(${DAEMON} ${SRCS})
SET_TARGET_PROPERTIES(${DAEMON} PROPERTIES COMPILE_FLAGS ${EXTRA_CFLAGS})
TARGET_LINK_LIBRARIES(${DAEMON} ${music-player-app_control_pkgs_LDFLAGS})

TARGET_LINK_LIBRARIES(${DAEMON} -L${CMAKE_SOURCE_DIR}/src/lib -l_MusicSquare_TIZEN)


INSTALL(TARGETS ${DAEMON} DESTINATION ${BINDIR})

