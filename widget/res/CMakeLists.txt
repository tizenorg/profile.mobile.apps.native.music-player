# install edj
ADD_CUSTOM_TARGET(music_widget.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/images ${EDJE_CFLAGS}
		${CMAKE_CURRENT_SOURCE_DIR}/music_widget.edc music_widget.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/music_widget.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} music_widget.edj)
INSTALL(FILES music_widget.edj DESTINATION ${RESDIR})
