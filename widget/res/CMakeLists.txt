# install edj
ADD_CUSTOM_TARGET(musicwidget.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/music_widget.edc musicwidget.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/music_widget.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} musicwidget.edj)
INSTALL(FILES musicwidget.edj DESTINATION ${EDJDIR})
