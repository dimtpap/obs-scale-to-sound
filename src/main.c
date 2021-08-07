#include <obs-module.h>

OBS_DECLARE_MODULE()

const char *SOURCE_NAME = "Scale To Sound";

struct scale_to_sound_data {
	obs_source_t *context;
};

char* get_source_name(void) {
	return SOURCE_NAME;
}

static void *create_source(obs_data_t *settings, obs_source_t *source) {
  struct scale_to_sound_data *stsf = bzalloc(sizeof(*stsf));
	stsf->context = source;
	return stsf;
}

static void destroy_source(obs_data_t *data) {
	struct scale_to_sound_data *stsf = data;
	bfree(stsf);
}

struct obs_source_info scale_to_sound = {
	.id           = "scale_to_sound",
	.type         = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name     = get_source_name,
  .create 			= create_source,
	.destroy 			= destroy_source
};

bool obs_module_load(void)
{
	obs_register_source(&scale_to_sound);
	return true;
}
