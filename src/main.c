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

OBS_DECLARE_MODULE()

struct scale_to_sound_data {
	obs_source_t *context;
	gs_effect_t *mover;
};

char *get_source_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return SOURCE_NAME;
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	struct scale_to_sound_data *stsf = bzalloc(sizeof(*stsf));
	stsf->context = source;

	obs_enter_graphics();
	stsf->mover = gs_effect_create_from_file(obs_module_file("default_move.effect"), NULL);
	obs_leave_graphics();

	return stsf;
}

static obs_data_t *filter_defaults(void *data)
{
	UNUSED_PARAMETER(data);

	obs_data_t *settings = obs_data_create();
	
	//TODO: Create the defaults

	return settings;
}

static obs_properties_t *filter_properties(void *data)
{
	UNUSED_PARAMETER(data);

	obs_properties_t *p = obs_properties_create();
	
	//TODO: Create the properties
	/* Audio Source - Source to get audio data from
	   Minimum size to scale to (%)
		 Maximum size to scale to (%)

		 Maybes:
		 Minimum/Maximum audio level
	*/

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
	//.get_properties = filter_properties,
	//.get_defaults = filter_defaults,
	.create = filter_create,
	.destroy = filter_destroy
};

bool obs_module_load(void)
{
	obs_register_source(&scale_to_sound);
	return true;
}