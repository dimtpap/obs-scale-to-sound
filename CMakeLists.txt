project(scale-to-sound VERSION 0.3.1)

set(scale-to-sound_SOURCES
  src/main.c)

add_library(scale-to-sound MODULE ${scale-to-sound_SOURCES})

target_link_libraries(scale-to-sound libobs)
set_target_properties(scale-to-sound PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(scale-to-sound data)
