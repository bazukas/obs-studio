/*
Copyright (C) 2014 by Leonhard Oelke <leonhard@in-verted.de>

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

#include <fcntl.h>
#include <dirent.h>

#include <libv4l2.h>

#include <util/dstr.h>

#include "v4l2-helpers.h"
#include "v4l2-properties.h"

#define blog(level, msg, ...) blog(level, "v4l2-helpers: " msg, ##__VA_ARGS__)

void v4l2_props_set_enabled(obs_properties_t *props,
		obs_property_t *ignore, bool enable)
{
	if (!props)
		return;

	for (obs_property_t *prop = obs_properties_first(props); prop != NULL;
			obs_property_next(&prop)) {
		if (prop == ignore)
			continue;

		obs_property_set_enabled(prop, enable);
	}
}

bool device_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings)
{
	int dev;
	obs_property_t *prop;

	dev  = v4l2_open(obs_data_get_string(settings, "device_id"),
			O_RDWR | O_NONBLOCK);

	v4l2_props_set_enabled(props, p, (dev == -1) ? false : true);

	if (dev == -1)
		return false;

	prop = obs_properties_get(props, "input");
	v4l2_input_list(dev, prop);
	obs_property_modified(prop, settings);
	v4l2_close(dev);
	return true;
}

bool input_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings)
{
	UNUSED_PARAMETER(p);
	int dev;
	obs_property_t *prop;

	dev  = v4l2_open(obs_data_get_string(settings, "device_id"),
			O_RDWR | O_NONBLOCK);
	if (dev == -1)
		return false;

	prop = obs_properties_get(props, "pixelformat");
	v4l2_format_list(dev, prop);
	obs_property_modified(prop, settings);
	v4l2_close(dev);
	return true;
}

bool format_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings)
{
	UNUSED_PARAMETER(p);
	int dev;
	obs_property_t *prop;

	dev  = v4l2_open(obs_data_get_string(settings, "device_id"),
			O_RDWR | O_NONBLOCK);
	if (dev == -1)
		return false;

	prop = obs_properties_get(props, "resolution");
	v4l2_resolution_list(dev, obs_data_get_int(settings, "pixelformat"),
			prop);
	obs_property_modified(prop, settings);
	v4l2_close(dev);
	return true;
}

bool resolution_selected(obs_properties_t *props, obs_property_t *p,
		obs_data_t *settings)
{
	UNUSED_PARAMETER(p);
	int dev;
	int width, height;
	obs_property_t *prop;

	dev  = v4l2_open(obs_data_get_string(settings, "device_id"),
			O_RDWR | O_NONBLOCK);
	if (dev == -1)
		return false;

	prop = obs_properties_get(props, "framerate");
	v4l2_unpack_tuple(&width, &height, obs_data_get_int(settings,
			"resolution"));
	v4l2_framerate_list(dev, obs_data_get_int(settings, "pixelformat"),
			width, height, prop);
	obs_property_modified(prop, settings);
	v4l2_close(dev);
	return true;
}

void v4l2_input_list(int_fast32_t dev, obs_property_t *prop)
{
	struct v4l2_input in;
	memset(&in, 0, sizeof(in));

	obs_property_list_clear(prop);

	obs_property_list_add_int(prop, obs_module_text("LeaveUnchanged"), -1);

	while (v4l2_ioctl(dev, VIDIOC_ENUMINPUT, &in) == 0) {
		if (in.type & V4L2_INPUT_TYPE_CAMERA) {
			obs_property_list_add_int(prop, (char *) in.name,
					in.index);
			blog(LOG_INFO, "Found input '%s' (Index %d)", in.name,
					in.index);
		}
		in.index++;
	}
}

void v4l2_format_list(int dev, obs_property_t *prop)
{
	struct v4l2_fmtdesc fmt;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.index = 0;
	struct dstr buffer;
	dstr_init(&buffer);

	obs_property_list_clear(prop);

	obs_property_list_add_int(prop, obs_module_text("LeaveUnchanged"), -1);

	while (v4l2_ioctl(dev, VIDIOC_ENUM_FMT, &fmt) == 0) {
		dstr_copy(&buffer, (char *) fmt.description);
		if (fmt.flags & V4L2_FMT_FLAG_EMULATED)
			dstr_cat(&buffer, " (Emulated)");

		if (v4l2_to_obs_video_format(fmt.pixelformat)
				!= VIDEO_FORMAT_NONE) {
			obs_property_list_add_int(prop, buffer.array,
					fmt.pixelformat);
			blog(LOG_INFO, "Pixelformat: %s (available)",
			     buffer.array);
		} else {
			blog(LOG_INFO, "Pixelformat: %s (unavailable)",
			     buffer.array);
		}
		fmt.index++;
	}

	dstr_free(&buffer);
}

void v4l2_resolution_list(int dev, uint_fast32_t pixelformat,
		obs_property_t *prop)
{
	struct v4l2_frmsizeenum frmsize;
	frmsize.pixel_format = pixelformat;
	frmsize.index = 0;
	struct dstr buffer;
	dstr_init(&buffer);

	obs_property_list_clear(prop);

	obs_property_list_add_int(prop, obs_module_text("LeaveUnchanged"), -1);

	v4l2_ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &frmsize);

	switch(frmsize.type) {
	case V4L2_FRMSIZE_TYPE_DISCRETE:
		while (v4l2_ioctl(dev, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
			dstr_printf(&buffer, "%dx%d", frmsize.discrete.width,
					frmsize.discrete.height);
			obs_property_list_add_int(prop, buffer.array,
					v4l2_pack_tuple(frmsize.discrete.width,
					frmsize.discrete.height));
			frmsize.index++;
		}
		break;
	default:
		blog(LOG_INFO, "Stepwise and Continuous framesizes "
			"are currently hardcoded");

		for (const int *packed = v4l2_framesizes; *packed; ++packed) {
			int width;
			int height;
			v4l2_unpack_tuple(&width, &height, *packed);
			dstr_printf(&buffer, "%dx%d", width, height);
			obs_property_list_add_int(prop, buffer.array, *packed);
		}
		break;
	}

	dstr_free(&buffer);
}

void v4l2_framerate_list(int dev, uint_fast32_t pixelformat,
		uint_fast32_t width, uint_fast32_t height, obs_property_t *prop)
{
	struct v4l2_frmivalenum frmival;
	frmival.pixel_format = pixelformat;
	frmival.width = width;
	frmival.height = height;
	frmival.index = 0;
	struct dstr buffer;
	dstr_init(&buffer);

	obs_property_list_clear(prop);

	obs_property_list_add_int(prop, obs_module_text("LeaveUnchanged"), -1);

	v4l2_ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS, &frmival);

	switch(frmival.type) {
	case V4L2_FRMIVAL_TYPE_DISCRETE:
		while (v4l2_ioctl(dev, VIDIOC_ENUM_FRAMEINTERVALS,
				&frmival) == 0) {
			float fps = (float) frmival.discrete.denominator /
				frmival.discrete.numerator;
			int pack = v4l2_pack_tuple(frmival.discrete.numerator,
					frmival.discrete.denominator);
			dstr_printf(&buffer, "%.2f", fps);
			obs_property_list_add_int(prop, buffer.array, pack);
			frmival.index++;
		}
		break;
	default:
		blog(LOG_INFO, "Stepwise and Continuous framerates "
			"are currently hardcoded");

		for (const int *packed = v4l2_framerates; *packed; ++packed) {
			int num;
			int denom;
			v4l2_unpack_tuple(&num, &denom, *packed);
			float fps = (float) denom / num;
			dstr_printf(&buffer, "%.2f", fps);
			obs_property_list_add_int(prop, buffer.array, *packed);
		}
		break;
	}

	dstr_free(&buffer);
}

/*
 * List available devices
 */
void v4l2_device_list(obs_property_t *prop, obs_data_t *settings)
{
	DIR *dirp;
	struct dirent *dp;
	struct dstr device;
	bool cur_device_found;
	size_t cur_device_index;
	const char *cur_device_name;

	dirp = opendir("/sys/class/video4linux");
	if (!dirp)
		return;

	cur_device_found = false;
	cur_device_name = (settings)
			? obs_data_get_string(settings, "device_id")
			: NULL;

	obs_property_list_clear(prop);

	dstr_init_copy(&device, "/dev/");

	while ((dp = readdir(dirp)) != NULL) {
		int fd;
		uint32_t caps;
		struct v4l2_capability video_cap;

		if (dp->d_type == DT_DIR)
			continue;

		dstr_resize(&device, 5);
		dstr_cat(&device, dp->d_name);

		if ((fd = v4l2_open(device.array, O_RDWR | O_NONBLOCK)) == -1) {
			blog(LOG_INFO, "Unable to open %s", device.array);
			continue;
		}

		if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &video_cap) == -1) {
			blog(LOG_INFO, "Failed to query capabilities for %s",
			     device.array);
			v4l2_close(fd);
			continue;
		}

		caps = (video_cap.capabilities & V4L2_CAP_DEVICE_CAPS)
			? video_cap.device_caps
			: video_cap.capabilities;

		if (!(caps & V4L2_CAP_VIDEO_CAPTURE)) {
			blog(LOG_INFO, "%s seems to not support video capture",
			     device.array);
			v4l2_close(fd);
			continue;
		}

		obs_property_list_add_string(prop, (char *) video_cap.card,
				device.array);
		blog(LOG_INFO, "Found device '%s' at %s", video_cap.card,
				device.array);

		/* check if this is the currently used device */
		if (cur_device_name && !strcmp(cur_device_name, device.array))
			cur_device_found = true;

		v4l2_close(fd);
	}

	/* add currently selected device if not present, but disable it ... */
	if (!cur_device_found && cur_device_name) {
		cur_device_index = obs_property_list_add_string(prop,
				cur_device_name, cur_device_name);
		obs_property_list_item_disable(prop, cur_device_index, true);
	}

	closedir(dirp);
	dstr_free(&device);
}
