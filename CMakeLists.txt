cmake_minimum_required(VERSION 3.10)
project(scale-to-sound VERSION)

set(scale-to-sound_SOURCES
  src/scale-to-sound.c)

if(BUILD_STANDALONE)
  find_package(libobs REQUIRED)
endif()

add_library(scale-to-sound MODULE ${scale-to-sound_SOURCES})

if(BUILD_STANDALONE)
  target_link_libraries(scale-to-sound OBS::libobs)
else()
  target_link_libraries(scale-to-sound libobs)
endif()

if(BUILD_STANDALONE)
	set_target_properties(scale-to-sound PROPERTIES PREFIX "")

    install(TARGETS scale-to-sound
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/obs-plugins)
    install(FILES data/default_move.effect
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/scale-to-sound)
else()
	set_target_properties(scale-to-sound PROPERTIES FOLDER "plugins")

	install_obs_plugin_with_data(scale-to-sound data)
endif()