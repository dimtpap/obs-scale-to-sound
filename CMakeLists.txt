cmake_minimum_required(VERSION 3.10)
project(scale-to-sound VERSION 1.2.0)

set(scale-to-sound_SOURCES
  src/main.c)

if(BUILD_STANDALONE)
  find_package(LibObs REQUIRED)
endif()

add_library(scale-to-sound MODULE ${scale-to-sound_SOURCES})
target_link_libraries(scale-to-sound libobs)

if(BUILD_STANDALONE)
	set_target_properties(scale-to-sound PROPERTIES PREFIX "")
else()
	set_target_properties(scale-to-sound PROPERTIES FOLDER "plugins")

	install_obs_plugin_with_data(scale-to-sound data)
endif()