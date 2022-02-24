/*
obs-scale-to-sound
Copyright (C) 2021-2022 Dimitris Papaioannou

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
#include <util/platform.h>

#define SOURCE_NAME "Scale To Sound"

#define STS_AUDSRC "STS_AUDSRC"
#define STS_MINLVL "STS_MIN_LVL"
#define STS_MAXLVL "STS_MAXLVL"
#define STS_MINPER "STS_MINPER"
#define STS_MAXPER "STS_MAXPER"
#define STS_INVSCL "STS_INVSCL"
#define STS_SMOOTH "STS_SMOOTH"
#define STS_SCALEW "STS_SCALEW"
#define STS_SCALEH "STS_SCALEH"
#define STS_XPOSAL "STS_XPOSAL"
#define STS_YPOSAL "STS_YPOSAL"

OBS_DECLARE_MODULE()
const char *get_source_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return SOURCE_NAME;
}

enum positional_alignment {
	BEGINNING = 0,
	CENTER = 1,
	END = 2
};

struct scale_to_sound_data {
	obs_source_t *context;
	obs_source_t *target;

	obs_property_t *sources_list;
	obs_weak_source_t *audio_source;
	double minimum_audio_level;
	double maximum_audio_level;
	double audio_range;
	bool invert;
	long long min;
	long long max;
	double smooth;
	bool scale_w;
	bool scale_h;
	enum positional_alignment x_pos_alignment;
	enum positional_alignment y_pos_alignment;

	uint32_t src_w;
	uint32_t src_h;

	long long min_w;
	long long min_h;
	long long max_w;
	long long max_h;

	uint64_t last_update_ts;
	double audio_level;

	gs_effect_t *mover;
};

static void calculate_audio_level(void *param, obs_source_t *source, const struct audio_data *data, bool muted)
{
	UNUSED_PARAMETER(source);

	struct scale_to_sound_data *stsf = param;

	stsf->last_update_ts = os_gettime_ns();

	double min_audio_level = stsf->minimum_audio_level;

	if(muted) {
		stsf->audio_level = min_audio_level;
		return;
	}

	//Taken from libobs/obs-audio-controls.c volmeter_process_magnitude and slightly modified
	size_t nr_samples = data->frames;

	float *samples = (float *)data->data[0];
	if (!samples) {
		stsf->audio_level = min_audio_level;
		return;
	}
	float sum = 0.0;
	for (size_t i = 0; i < nr_samples; i++) {
		float sample = samples[i];
		sum += sample * sample;
	}

	double audio_level = (double)obs_mul_to_db(sqrtf(sum / nr_samples));

	double smooth = stsf->smooth;
	if(smooth < 1) {
		if(stsf->audio_level < min_audio_level) stsf->audio_level = min_audio_level;

		if(stsf->audio_level > audio_level) stsf->audio_level -= smooth;
		else if(stsf->audio_level < audio_level) stsf->audio_level += smooth;
	}
	else stsf->audio_level = audio_level >= min_audio_level ? audio_level : min_audio_level;
}

static void audio_source_destroy(void *data, calldata_t *call_data) {
	struct scale_to_sound_data *stsf = data;

	obs_weak_source_release(stsf->audio_source);
	stsf->audio_source = NULL;

	stsf->scale_h = false;
	stsf->scale_w = false;
}

static void *filter_create(obs_data_t *settings, obs_source_t *source)
{
	UNUSED_PARAMETER(settings);

	struct scale_to_sound_data *stsf = bzalloc(sizeof(*stsf));
	stsf->context = source;
	
	char *effect_file = obs_module_file("default_move.effect");
	obs_enter_graphics();
	stsf->mover = gs_effect_create_from_file(effect_file, NULL);
	obs_leave_graphics();
	bfree(effect_file);

	return stsf;
}

static void filter_update(void *data, obs_data_t *settings)
{
	struct scale_to_sound_data *stsf = data;

	obs_source_t *target = obs_filter_get_target(stsf->context);
	stsf->target = target;

	long long min = obs_data_get_int(settings, STS_MINPER);
	long long max = obs_data_get_int(settings, STS_MAXPER);

	uint32_t w = obs_source_get_base_width(target);
	uint32_t h = obs_source_get_base_height(target);

	stsf->src_w = w;
	stsf->src_h = h;

	if (max <= min) {
		obs_data_set_int(settings, STS_MAXPER, min + 1);
		stsf->max = min + 1;
	}
	else {
		stsf->max = max;
	}
	stsf->min = min;

	stsf->invert = obs_data_get_bool(settings, STS_INVSCL);

	stsf->smooth = 1 - obs_data_get_double(settings, STS_SMOOTH);

	stsf->scale_w = obs_data_get_bool(settings, STS_SCALEW);
	stsf->scale_h = obs_data_get_bool(settings, STS_SCALEH);

	stsf->min_w = w * min / 100;
	stsf->min_h = h * min / 100;
	stsf->max_w = w * max / 100;
	stsf->max_h = h * max / 100;

	stsf->x_pos_alignment = obs_data_get_int(settings, STS_XPOSAL);
	stsf->y_pos_alignment = obs_data_get_int(settings, STS_YPOSAL);

	stsf->minimum_audio_level = obs_data_get_double(settings, STS_MINLVL);
	double maximum_audio_level = obs_data_get_double(settings, STS_MAXLVL);
	if (maximum_audio_level > stsf->minimum_audio_level) {
		stsf->maximum_audio_level = maximum_audio_level;
		stsf->audio_range = stsf->maximum_audio_level - stsf->minimum_audio_level;
	}
	else {
		obs_data_set_double(settings, STS_MAXLVL, stsf->minimum_audio_level + 0.5f);
		stsf->maximum_audio_level = stsf->minimum_audio_level + 0.5f;
		stsf->audio_range = 0.5f;
	}

	obs_source_t *new_target = obs_get_source_by_name(obs_data_get_string(settings, STS_AUDSRC));

	obs_source_t *current_target = NULL;
	if (stsf->audio_source)
		current_target = obs_weak_source_get_source(stsf->audio_source);

	if (current_target != new_target) {
		signal_handler_t *sig_handler;
		if (current_target) {
			sig_handler = obs_source_get_signal_handler(current_target);
			signal_handler_disconnect(sig_handler, "destroy", audio_source_destroy, stsf);

			obs_source_remove_audio_capture_callback(current_target, calculate_audio_level, stsf);

			obs_weak_source_release(stsf->audio_source);
		}

		sig_handler = obs_source_get_signal_handler(new_target);
		signal_handler_connect(sig_handler, "destroy", audio_source_destroy, stsf);

		obs_source_add_audio_capture_callback(new_target, calculate_audio_level, stsf);

		stsf->audio_source = obs_source_get_weak_source(new_target);
	}

	obs_source_release(new_target);
	
	if (current_target) obs_source_release(current_target);
}

static void filter_load(void *data, obs_data_t *settings)
{
	filter_update(data, settings);
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

	obs_property_t *minlvl = obs_properties_add_float_slider(p, STS_MINLVL, "Audio Threshold", -100, -0.5, 0.5);
	obs_property_float_set_suffix(minlvl, "dB");

	obs_property_t *maxlvl = obs_properties_add_float_slider(p, STS_MAXLVL, "Audio Ceiling", -99.5, 0, 0.5);
	obs_property_float_set_suffix(maxlvl, "dB");

	obs_property_t *minper = obs_properties_add_int_slider(p, STS_MINPER, "Minimum Size", 0, 99, 1);
	obs_property_int_set_suffix(minper, "%");

	obs_property_t *maxper = obs_properties_add_int_slider(p, STS_MAXPER, "Maximum Size", 1, 100, 1);
	obs_property_int_set_suffix(maxper, "%");

	obs_properties_add_bool(p, STS_INVSCL, "Inverse Scaling");

	obs_properties_add_float_slider(p, STS_SMOOTH, "Smoothing", 0, 0.99, 0.01);

	obs_properties_add_bool(p, STS_SCALEW, "Scale Width");
	obs_properties_add_bool(p, STS_SCALEH, "Scale Height");

	obs_property_t *x_pos_alignment = obs_properties_add_list(p, STS_XPOSAL, "X Positional Alignment", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(x_pos_alignment, "Left", BEGINNING);
	obs_property_list_add_int(x_pos_alignment, "Center", CENTER);
	obs_property_list_add_int(x_pos_alignment, "Right", END);

	obs_property_t *y_pos_alignment = obs_properties_add_list(p, STS_YPOSAL, "Y Positional Alignment", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);
	obs_property_list_add_int(y_pos_alignment, "Top", BEGINNING);
	obs_property_list_add_int(y_pos_alignment, "Center", CENTER);
	obs_property_list_add_int(y_pos_alignment, "Bottom", END);

	return p;
}

static void filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, STS_MINLVL, -40);
	obs_data_set_default_double(settings, STS_MAXLVL, 0);

	obs_data_set_default_int(settings, STS_MINPER, 90);
	obs_data_set_default_int(settings, STS_MAXPER, 100);

	obs_data_set_default_bool(settings, STS_INVSCL, false);

	obs_data_set_default_double(settings, STS_SMOOTH, 0);

	obs_data_set_default_bool(settings, STS_SCALEW, true);
	obs_data_set_default_bool(settings, STS_SCALEH, true);

	obs_data_set_default_int(settings, STS_XPOSAL, CENTER);
	obs_data_set_default_int(settings, STS_YPOSAL, CENTER);
}

static void filter_destroy(void *data)
{
	struct scale_to_sound_data *stsf = data;

	if (stsf->audio_source) {
		obs_source_t *current_target = obs_weak_source_get_source(stsf->audio_source);

		signal_handler_t *sig_handler = obs_source_get_signal_handler(current_target);
		signal_handler_disconnect(sig_handler, "destroy", audio_source_destroy, stsf);

		obs_source_remove_audio_capture_callback(current_target, calculate_audio_level, stsf);

		obs_source_release(current_target);
		obs_weak_source_release(stsf->audio_source);
	}

	obs_enter_graphics();
	gs_effect_destroy(stsf->mover);
	obs_leave_graphics();

	bfree(stsf);
}

static void target_update(void *data, float seconds)
{
	UNUSED_PARAMETER(seconds);

	struct scale_to_sound_data *stsf = data;

	//!This should really be done using a signal but I could not get those working so here we are...
	obs_source_t *target = stsf->target;

	uint32_t w = stsf->src_w;
	uint32_t h = stsf->src_h;

	uint32_t new_w = obs_source_get_base_width(target);
	uint32_t new_h = obs_source_get_base_height(target);

	if(new_w != w || new_h != h) {
		obs_data_t *settings = obs_source_get_settings(stsf->context);
		filter_update(stsf, settings);
		obs_data_release(settings);
	}
}

static float determine_position(int i, int target, enum positional_alignment option) {
	switch(option) {
	case BEGINNING:
		return 0;
		break;
	case END:
		return i - target;
		break;
	case CENTER:
	default:
		return (i - target) / 2;
		break;
	}
}
static void filter_render(void *data, gs_effect_t *effect)
{
	UNUSED_PARAMETER(effect);

	struct scale_to_sound_data *stsf = data;

	uint32_t w = stsf->src_w;
	uint32_t h = stsf->src_h;

	uint32_t min_scale_percent = stsf->min;
	uint32_t max_scale_percent = stsf->max;

	double min_audio_level = stsf->minimum_audio_level;
	double max_audio_level = stsf->maximum_audio_level;

	double range = stsf->audio_range;

	double audio_level = stsf->audio_level;

	if(min_audio_level >= 0) min_audio_level = -0.5f;

	if(stsf->audio_level > stsf->minimum_audio_level && os_gettime_ns() - stsf->last_update_ts > 500000000) { //0.5s timeout
		if(stsf->smooth < 1) stsf->audio_level -= stsf->smooth;
		else stsf->audio_level = stsf->minimum_audio_level;
	}

	double scale_percent = fabs(min_audio_level) - fabs(audio_level);

	//Scale the calculated from audio percentage down to the user-set range
	scale_percent = (scale_percent * (max_scale_percent - min_scale_percent)) / range + min_scale_percent;
	if(scale_percent < min_scale_percent || audio_level >= 0) scale_percent = min_scale_percent;

	if(stsf->invert) scale_percent = min_scale_percent + max_scale_percent - scale_percent;

	uint32_t audio_w = stsf->scale_w ? w * scale_percent / 100 : w;
	uint32_t audio_h = stsf->scale_h ? h * scale_percent / 100 : h;

	if((audio_level < min_audio_level && !stsf->invert) || audio_w < stsf->min_w || audio_h < stsf->min_h) {
		audio_w = stsf->scale_w ? stsf->min_w : w;
		audio_h = stsf->scale_h ? stsf->min_h : h;
	}

	if(audio_w > stsf->max_w) audio_w = stsf->scale_w ? stsf->max_w : w;
	if(audio_h > stsf->max_h) audio_h = stsf->scale_h ? stsf->max_h : h;

	obs_enter_graphics();
	obs_source_process_filter_begin(stsf->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING);

	gs_effect_t *move_effect = stsf->mover;
	gs_eparam_t *move_val = gs_effect_get_param_by_name(move_effect, "inputPos");
	gs_eparam_t *show = gs_effect_get_param_by_name(move_effect, "show");

	gs_effect_set_float(show, 1.0f);
	if(audio_w <= 0 || audio_h <= 0) {
		gs_effect_set_float(show, 0.0f);
		audio_w = 1;
		audio_h = 1;
	}

	float x_pos = determine_position(w, audio_w, stsf->x_pos_alignment);
	float y_pos = determine_position(h, audio_h, stsf->y_pos_alignment);

	struct vec4 move_vec;
	vec4_set(&move_vec, x_pos, y_pos, 0.0f, 0.0f);

	gs_effect_set_vec4(move_val, &move_vec);

	obs_source_process_filter_end(stsf->context, move_effect, audio_w, audio_h);
	obs_leave_graphics();
}

struct obs_source_info scale_to_sound = {
	.id = "scale_to_sound",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_VIDEO,
	.get_name = get_source_name,
	.get_defaults = filter_defaults,
	.get_properties = filter_properties,
	.create = filter_create,
	.load = filter_load,
	.update = filter_update,
	.video_tick = target_update,
	.video_render = filter_render,
	.destroy = filter_destroy
};

bool obs_module_load(void)
{
	obs_register_source(&scale_to_sound);
	return true;
}
