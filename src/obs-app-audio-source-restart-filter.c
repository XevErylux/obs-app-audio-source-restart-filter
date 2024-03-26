/*
OBS App Audio Source Restart Filter
Copyright (C) 2024 XevErylux XevErylux@users.noreply.github.com

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
#include <media-io/audio-math.h>
#include <math.h>

#define do_log(level, format, ...)                     \
	blog(level, "[app audio source restart filter: '%s'] " format, \
	     obs_source_get_name(gf->context), ##__VA_ARGS__)

#define warn(format, ...) do_log(LOG_WARNING, format, ##__VA_ARGS__)
#define info(format, ...) do_log(LOG_INFO, format, ##__VA_ARGS__)

#define S_DESCRIPTION "description"

#define MT_ obs_module_text
#define TEXT_DESCRIPTION "Restarts application audio source if toggle."
#define TEXT_LONG_DESCRIPTION                                                              \
	"If this filter is inside an application audio source, it will restart it, "          \
	"by changing the configured window twice (away and back). This happens every time "   \
	"the filter is getting toggled (enabled or disabled - the eye icon on the left). "     \
	"This is meant as a building block which is used with other plugins that can control " \
	"filters, like the Move plugin By Exeldro."

struct gain_data {
	obs_source_t *context;
	bool enabled;
};

static const char *gain_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return "App Audio Restart";
}

static void gain_destroy(void *data)
{
	struct gain_data *gf = data;
	bfree(gf);
}

static void gain_update(void *data, obs_data_t *s)
{
	struct gain_data *gf = data;
}

static void *gain_create(obs_data_t *settings, obs_source_t *filter)
{
	struct gain_data *gf = bzalloc(sizeof(*gf));
	gf->context = filter;
	gf->enabled = obs_source_enabled(filter);
	gain_update(gf, settings);
	return gf;
}

static void gain_filter_add(void *data, obs_source_t *filter)
{
	struct gain_data *gf = data;
	gf->enabled = obs_source_enabled(gf->context);
	info("gain_filter_add");
}

static void gain_filter_remove(void *data, obs_source_t *filter)
{
	struct gain_data *gf = data;
	gf->enabled = obs_source_enabled(gf->context);
	info("gain_filter_remove");
}

static obs_source_t *try_find_input_source(obs_source_t *source)
{
	while (source != NULL) {
		const enum obs_source_type source_type =
			obs_source_get_type(source);
		if (source_type == OBS_SOURCE_TYPE_INPUT)
			break;

		source = obs_filter_get_parent(source);
	}

	return source;
}

static const char *clear_window_title(const char *window)
{
	if (window[0] == ':') {
		// If first char is a colon, add a 1 before it
		char *result = bmalloc(strlen(window) + 2);
		result[0] = '1';
		strcpy(result + 1, window);
		return result;
	} else {
		// Otherwise remove all characters before the colon
		const char *colon_position = strchr(window, ':');
		if (colon_position != NULL) {
			return bstrdup(colon_position);
		} else {
			// If no colon is there give the original window back
			return window;
		}
	}
}

static void restart_source(struct gain_data *gf, obs_source_t *source)
{
	const char *name = obs_source_get_name(source);
	info("Restart source: %s...", name);

	// We only do something if it is the application audio source.
	const char *source_id = obs_source_get_unversioned_id(source);
	if (strcmp(source_id, "wasapi_process_output_capture"))
		return;

	obs_data_t *settings = obs_source_get_settings(source);
	if (settings != NULL) {
		const char *window = obs_data_get_string(settings, "window");
		const char *origwindow = bstrdup(window);
		//info("origwindow: %s", origwindow);
		// Remove window title, to make a change
		const char *newwindow = clear_window_title(origwindow);
		//info("newwindow: %s", newwindow);
		//info("origwindow: %s", origwindow);
		obs_data_set_string(settings, "window", newwindow);
		obs_source_update(source, settings);
		// Add window title back, so it reinitializes the source.
		obs_data_set_string(settings, "window", origwindow);
		obs_source_update(source, settings);
		info("Restarted source: %s!", name);
		bfree((char *)newwindow);
		bfree((char *)origwindow);
		obs_data_release(settings);
	}
}

static void gain_enabled_changed(struct gain_data *gf)
{
	obs_source_t *filter = gf->context;
	obs_source_t *source = try_find_input_source(filter);
	if (source == NULL) {
		info("Input source for filter not found");
		return;
	}

	restart_source(gf, source);
}

static void gain_tick(void *data, float seconds)
{
	struct gain_data *gf = data;
	bool currEnabled = obs_source_enabled(gf->context);
	if (gf->enabled == currEnabled)
		return;

	gf->enabled = currEnabled;
	if (currEnabled) {
		//info("gain_enabled");
	} else {
		//info("gain_disabled");
	}

	gain_enabled_changed(gf);
}

static void gain_defaults(obs_data_t *s)
{
}

static obs_properties_t *gain_properties(void *data)
{
	obs_properties_t *ppts = obs_properties_create();

	obs_property_t *description = obs_properties_add_text(ppts, S_DESCRIPTION, NULL,
				OBS_TEXT_INFO);
	obs_property_set_long_description(description, TEXT_LONG_DESCRIPTION);
	obs_property_text_set_info_type(description, OBS_TEXT_INFO_NORMAL);
	obs_property_text_set_info_word_wrap(description, true);

	UNUSED_PARAMETER(data);
	return ppts;
}

struct obs_source_info gain_filter = {
	.id = "app_audio_source_restart_filter",
	.type = OBS_SOURCE_TYPE_FILTER,
	.output_flags = OBS_SOURCE_AUDIO,
	.get_name = gain_name,
	.create = gain_create,
	.destroy = gain_destroy,
	.update = gain_update,
	.filter_add = gain_filter_add,
	.filter_remove = gain_filter_remove,
	.video_tick = gain_tick,
	.get_defaults = gain_defaults,
	.get_properties = gain_properties,
};