project(obs-scale-to-sound VERSION 0.3.0)

set(obs-scale-to-sound_SOURCES
  src/main.c)

add_library(obs-scale-to-sound MODULE ${obs-scale-to-sound_SOURCES})

target_link_libraries(obs-scale-to-sound libobs)
set_target_properties(obs-scale-to-sound PROPERTIES FOLDER "plugins")

install_obs_plugin_with_data(obs-scale-to-sound data)
