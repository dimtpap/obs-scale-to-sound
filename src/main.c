/*
obs-scale-to-sound
Copyright (C) 2021 Dimitris Papaioannou

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs-module.h>

#define SOURCE_NAME "Scale To Sound"

#define STS_AUDSRC "STS_AUDSRC"
#define STS_MINPER "STS_MINPER"
#define STS_MAXPER "STS_MAXPER"

OBS_DECLARE_MODULE()

struct scale_to_sound_data {
	obs_source_t *context;
	obs_property_t *sources_list;

	gs_effect_t *mover;
	obs_source_t *audio_source;

	long long *min;
	long long *max;
};

char *get_source_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return SOURCE_NAME;
}


static void *filter_update(void *data, obs_data_t *settings)
{
	struct scale_to_sound_data *stsf = data;
	stsf->audio_source = obs_get_source_by_name(obs_data_get_string(settings, STS_AUDSRC));

	stsf->min = obs_data_get_int(settings, STS_MINPER);
	stsf->max = obs_data_get_int(settings, STS_MAXPER);

	return stsf;
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	struct scale_to_sound_data *stsf = bzalloc(sizeof(*stsf));
	stsf->context = source;

	obs_enter_graphics();
	stsf->mover = gs_effect_create_from_file(obs_module_file("default_move.effect"), NULL);
	obs_leave_graphics();

	return filter_update(stsf, settings);
}

static obs_data_t *filter_defaults(void *data)
{
	UNUSED_PARAMETER(data);

	obs_data_t *settings = obs_data_create();
	
	obs_data_set_default_int(settings, STS_MINPER, 90);
	obs_data_set_default_int(settings, STS_MAXPER, 100);

	return settings;
}

static bool enum_audio_sources(void *data, obs_source_t *source)
{
	struct scale_to_sound_data *stsf = data;
	uint32_t flags = obs_source_get_output_flags(source);

	if ((flags & OBS_SOURCE_AUDIO) != 0) {
		const char *name = obs_source_get_name(source);
		obs_property_list_add_string(stsf->sources_list, name, name);
	}
	return true;
}
static obs_properties_t *filter_properties(void *data)
{
	struct scale_to_sound_data *stsf = data;

	obs_properties_t *p = obs_properties_create();

	obs_property_t *sources = obs_properties_add_list(p, STS_AUDSRC, "Audio Source", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	stsf->sources_list = sources;
	obs_enum_sources(enum_audio_sources, stsf);

	obs_property_t *minper = obs_properties_add_int_slider(p, STS_MINPER, "Minimum Size", 0, 100, 1);
	obs_property_int_set_suffix(minper, "%");
	
	obs_property_t *maxper = obs_properties_add_int_slider(p, STS_MAXPER, "Maximum Size", 0, 100, 1);
	obs_property_int_set_suffix(maxper, "%");

	return p;
}

static void filter_destroy(obs_data_t *data)
{
	struct scale_to_sound_data *stsf = data;

	obs_enter_graphics();
	gs_effect_destroy(stsf->mover);
	obs_leave_graphics();

	bfree(stsf);
}

static void filter_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	struct scale_to_sound_data *stsf = data;
	obs_source_t *target = obs_filter_get_parent(stsf->context);

	uint32_t w = obs_source_get_base_width(target);
	uint32_t h = obs_source_get_base_height(target);

	//TODO: Get audio data
	uint32_t audio_w = w;
	uint32_t audio_h = h;

	obs_enter_graphics();
	obs_source_process_filter_begin(stsf->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING);

	gs_effect_t *move_effect = stsf->mover;
	gs_eparam_t *move_val = gs_effect_get_param_by_name(move_effect, "inputPos");

	//Change the position everytime so it looks like it's scaling from the center
	struct vec4 move_vec;
	vec4_set(&move_vec, (w - audio_w) / 2, (h - audio_h) / 2, 0.0f, 0.0f);

	gs_effect_set_vec4(move_val, &move_vec);

	obs_source_process_filter_end(stsf->context, move_effect, audio_w, audio_h);
	obs_leave_graphics();
}

struct obs_source_info scale_to_sound = {
	.id = "scale_to_sound",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = get_source_name,
	.video_render = filter_render,
	.get_properties = filter_properties,
	.get_defaults = filter_defaults,
	.create = filter_create,
	.update = filter_update,
	.destroy = filter_destroy};

bool obs_module_load(void)
{
	obs_register_source(&scale_to_sound);
	return true;
}