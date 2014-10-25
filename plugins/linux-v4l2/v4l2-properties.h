/*
Copyright (C) 2014 by Leonhard Oelke <leonhard@in-verted.de>
Copyright (C) 2014 by Azat Khasanshin <akhasanshin3@gatech.edu>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <obs-module.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data structure for storing user controls
 */
struct v4l2_user_controls {
	int brightness;
	int contrast;
	int saturation;
	int white_balance;
	bool white_balance_auto;
	int sharpness;
	int focus_absolute;
	bool focus_auto;
};

/**
 * Enable/Disable all properties for the source.
 *
 * @note A property that should be ignored can be specified
 *
 * @param props the source properties
 * @param ignore ignore this property
 * @param enable enable/disable all properties
 */
void v4l2_props_set_enabled(obs_properties_t *props,
		obs_property_t *ignore, bool enable);
/*
 * Device selected callback
 *
 * @param props source properties
 * @param p property that has been changed
 * @param settings source settings
 *
 * @return true if properties needs to be refreshed
 */
bool device_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings);

/*
 * Input selected callback
 *
 * @param props source properties
 * @param p property that has been changed
 * @param settings source settings
 *
 * @return true if properties needs to be refreshed
 */
bool input_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings);

/*
 * Format selected callback
 *
 * @param props source properties
 * @param p property that has been changed
 * @param settings source settings
 *
 * @return true if properties needs to be refreshed
 */
bool format_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings);

/*
 * Resolution selected callback
 *
 * @param props source properties
 * @param p property that has been changed
 * @param settings source settings
 *
 * @return true if properties needs to be refreshed
 */
bool resolution_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings);

/*
 * Advanced checkbox callback
 *
 * @param props source properties
 * @param p property that has been changed
 * @param settings source settings
 *
 * @return true if properties needs to be refreshed
 */
bool advanced_settings(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings);

/*
 * List inputs for device
 *
 * @param dev handle for the v4l2 device
 * @param prop property to be populated
 */
void v4l2_input_list(int_fast32_t dev, obs_property_t *prop);

/*
 * List formats for device
 *
 * @param dev handle for the v4l2 device
 * @param prop property to be populated
 */
void v4l2_format_list(int dev, obs_property_t *prop);

/*
 * List resolutions for device and format
 *
 * @param dev handle for the v4l2 device
 * @param prop property to be populated
 */
void v4l2_resolution_list(int dev, uint_fast32_t pixelformat,
		obs_property_t *prop);

/*
 * List framerates for device and resolution
 *
 * @param dev handle for the v4l2 device
 * @param prop property to be populated
 */
void v4l2_framerate_list(int dev, uint_fast32_t pixelformat,
		uint_fast32_t width, uint_fast32_t height, obs_property_t *prop);

/*
 * List available devices
 *
 * @param prop property to be populated
 * @param settings source settings
 */
void v4l2_device_list(obs_property_t *prop, obs_data_t *settings);

/*
 * Apply v4l2 user controls
 *
 * @param controls v4l2 user control settings
 */
void v4l2_set_controls(int_fast32_t dev, struct v4l2_user_controls controls);

#ifdef __cplusplus
}
#endif
