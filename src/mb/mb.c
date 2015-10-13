/*
*
*  Copyright (C) 2015 "Rodrigo Costa" <rodrigocosta@telemidia.puc-rio.br>
* 
*  This file is part of Libmicromb.
*
*  Libmicromb is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Libmicromb is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Libmicromb.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "mb.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_WINDOW_WIDTH 1920
#define DEFAULT_WINDOW_HEIGHT 1080


int
mb_init ()
{
	return init (DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
}

int
mb_init_args (int width, int height)
{
	return init (width, height);
}

MbMedia *
mb_media_new (const char *media_name, const char *uri,
							int x, int y, int z, int width, int height)
{
	size_t lenght;
	MbMedia *media;

	media = (MbMedia *) malloc (sizeof (MbMedia));

	g_mutex_init (&(media->mutex));

	media->bin = gst_bin_new (media_name);
	media->decoder = gst_element_factory_make ("uridecodebin", NULL);
	media->video_scaler = gst_element_factory_make ("videoscale", NULL);
	media->video_filter = gst_element_factory_make ("capsfilter", NULL);

	media->image_freezer = NULL;

	media->audio_volume = NULL;
	media->audio_converter = NULL;
	media->audio_resampler = NULL;
	media->audio_filter = NULL;


	lenght = strlen(media_name);
	media->name = (char *) malloc ((lenght + 1)  * sizeof (char));
	strcpy(media->name, media_name);

	media->video_pad_name  = NULL;
	media->audio_pad_name  = NULL;

	media->valid_pads = 0;

	//properties
	media->x_pos = x;
	media->y_pos = y;
	media->width = width;
	media->height = height;
	media->z_index = z;
	media->alpha = 1.0;
	media->volume = 1.0;

	g_object_set (media->decoder, "uri", uri, NULL);
	g_signal_connect (G_OBJECT(media->decoder), "pad-added",
										G_CALLBACK (pad_added_cb), media);

	gst_bin_add (GST_BIN(media->bin), media->decoder);

	return media;
}

gboolean
mb_media_start (MbMedia *media)
{
	GstStateChange ret;
	GstState current_state;
  MbStateChangeEvent *event;

	g_assert (media != NULL);

	gst_bin_add (GST_BIN(_global.pipeline), media->bin);

	media->start_offset = gst_clock_get_time(_global.clock_provider);

	gst_element_set_state (media->bin, GST_STATE_PLAYING);
	do
	{
		ret = gst_element_get_state (media->bin, &current_state,
																 NULL, GST_CLOCK_TIME_NONE);
		if (ret == GST_STATE_CHANGE_FAILURE)
			return FALSE;

	} while (current_state != GST_STATE_PLAYING);

	g_print ("'%s' state: %s.\n", media->name,
					 gst_element_state_get_name(current_state));

  event = create_state_change_event (MB_BEGIN, media);

	notify_handler(event);

  free (event);

	return TRUE;
}

gboolean
mb_media_stop (MbMedia *media)
{
	GstStateChange ret;
	GstIterator *it = NULL;
  GValue data = G_VALUE_INIT;
	GstPad *pad = NULL;
	gboolean done = FALSE;

	g_assert (media);

	/*
	 * If the media is an image (media->image_freezer != 0), our concern
	 * should be the image_freezer's source pad instead of the decoder's
	 * source pad (block this pad cause no effect).
	 */
	it = gst_element_iterate_src_pads(media->image_freezer == NULL ?
																		media->decoder : media->image_freezer);
	while (!done)
	{
		switch (gst_iterator_next (it, &data))
		{
			case GST_ITERATOR_OK:
				pad = GST_PAD(g_value_get_object (&data));
				gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
													 stop_pad_cb, media, NULL);

				break;
			case GST_ITERATOR_RESYNC:
				gst_iterator_resync (it);
				break;
			case GST_ITERATOR_ERROR:
			case GST_ITERATOR_DONE:
				done = TRUE;
				break;
		}
	}

	gst_iterator_free(it);

	return TRUE;
}

GstBus *
mb_get_message_bus ()
{
	if (_global.bus == NULL)
		_global.bus = gst_element_get_bus (_global.pipeline);

	return _global.bus;
}

gboolean
mb_media_set_size (MbMedia *media, int width, int height)
{
	GstCaps *new_caps = NULL;
	GstPad *filter_sink_pad = NULL;

	g_assert (media != NULL);

	g_mutex_lock(&(media->mutex));

	if (!media->video_filter)
	{
		g_printerr ("Video stream not found\n");
		return FALSE;
	}

	filter_sink_pad = gst_element_get_static_pad(media->video_filter, "sink");
	g_assert (filter_sink_pad);

	new_caps = gst_caps_new_simple ("video/x-raw", "pixel-aspect-ratio",
																	GST_TYPE_FRACTION, 1, 1,
																	"width", G_TYPE_INT, width,
																	"height", G_TYPE_INT, height,
																	NULL);
	g_assert(new_caps);

	g_object_set (G_OBJECT(media->video_filter), "caps", new_caps, NULL);
	media->width = width;
	media->height = height;

	g_mutex_unlock(&(media->mutex));

	//Cleaning up
	gst_caps_unref(new_caps);
	gst_object_unref(filter_sink_pad);

	return TRUE;
}

gboolean
mb_media_set_volume (MbMedia *media, gdouble volume)
{
	gboolean return_code = TRUE;

	g_assert (media != NULL);

	g_mutex_lock(&(media->mutex));

	if (media->audio_volume == NULL)
	{
		return_code = FALSE;

		media->volume = volume;
		g_printerr ("This media has no audio output yet\n");
	}
	else
	{
		gdouble m_volume;

		g_object_set (G_OBJECT(media->audio_volume), "volume", volume, NULL);

		//Volume has really changed?
		g_object_get (G_OBJECT(media->audio_volume), "volume", &m_volume, NULL);
		if (m_volume == volume)
			media->volume = volume;
		else
			return_code = FALSE;
	}

	g_mutex_unlock(&(media->mutex));

	return return_code;
}

gboolean
mb_media_set_pos (MbMedia *media, int x, int y)
{
	int return_code = TRUE;

	g_assert (media);
	g_assert (media->bin);

	g_mutex_lock(&(media->mutex));

	if (media->video_pad_name == NULL)
	{
		return_code = FALSE;
		media->x_pos = x;
		media->y_pos = y;
	}
	else
	{
		GstPad *video_mixer_pad = NULL;
		int x_pos, y_pos;

		video_mixer_pad =
				gst_element_get_static_pad (_global.video_mixer, media->video_pad_name);
		g_assert(video_mixer_pad);

		g_object_set (video_mixer_pad, "xpos", x, NULL);
		g_object_set (video_mixer_pad, "ypos", y, NULL);

		//Properties have really changed?
		g_object_get (video_mixer_pad, "xpos", &x_pos, NULL);
		g_object_get (video_mixer_pad, "ypos", &y_pos, NULL);

		if (x_pos == x && y_pos == y)
		{
			media->x_pos = x;
			media->y_pos = y;
		}
		else
			return_code = FALSE;

		gst_object_unref(video_mixer_pad);
	}

	g_mutex_unlock(&(media->mutex));

	return return_code;
}

gboolean
mb_media_set_z (MbMedia *media, int z)
{
	int return_code = TRUE;

	g_assert (media != NULL);
	g_assert (media->bin);

	g_mutex_lock(&(media->mutex));

	if (media->video_pad_name == NULL)
	{
		return_code = FALSE;
		media->z_index = z;
	}
	else
	{
		GstPad *video_mixer_pad = NULL;
		int zorder;

		video_mixer_pad =
				gst_element_get_static_pad (_global.video_mixer, media->video_pad_name);
		g_assert(video_mixer_pad);

		g_object_set (video_mixer_pad, "zorder", z, NULL);

		//Property has really changed?
		g_object_get (video_mixer_pad, "zorder", &zorder, NULL);

		if (zorder == z)
		{
			media->z_index = z;
		}
		else
			return_code = FALSE;

		gst_object_unref(video_mixer_pad);
	}

	g_mutex_unlock(&(media->mutex));

	return return_code;
}

gboolean
mb_media_set_alpha (MbMedia *media, double alpha)
{
	int return_code = TRUE;

	g_assert (media != NULL);
	g_assert (media->bin);

	g_mutex_lock(&(media->mutex));

	if (media->video_pad_name == NULL)
	{
		return_code = FALSE;
		media->alpha = alpha;

		g_printerr ("This media has no visual output\n");
	}
	else
	{
		GstPad *video_mixer_pad = NULL;
		double alpha_channel;

		video_mixer_pad =
				gst_element_get_static_pad (_global.video_mixer, media->video_pad_name);
		g_assert(video_mixer_pad);

		g_object_set (video_mixer_pad, "alpha", alpha, NULL);

		//Property has really changed?
		g_object_get (video_mixer_pad, "alpha", &alpha_channel, NULL);

		if (alpha_channel == alpha)
		{
			media->alpha = alpha;
		}
		else
			return_code = FALSE;

		gst_object_unref(video_mixer_pad);
	}
	g_mutex_unlock(&(media->mutex));

	return return_code;
}

void
mb_register_handler (void (*handler)(MbEvent*))
{
	if (handler != NULL)
		_global.evt_handler = handler;
}

void
mb_unregister_handler ()
{
	_global.evt_handler = NULL;
}

int
mb_get_window_height ()
{
	return _global.window_height;
}

int
mb_get_window_width ()
{
	return _global.window_width;
}

void
mb_media_free (MbMedia *media)
{
	g_assert (media != NULL);

	gst_object_unref (media->bin);

	gst_object_unref(media->decoder);

	if (media->video_scaler != NULL)
		gst_object_unref(media->video_scaler);

	if (media->video_filter != NULL)
		gst_object_unref(media->video_filter);

	if (media->image_freezer != NULL)
		gst_object_unref(media->image_freezer);

	if (media->audio_converter != NULL)
		gst_object_unref(media->audio_converter);

	if (media->audio_volume != NULL)
		gst_object_unref(media->audio_volume);

	if (media->audio_resampler != NULL)
			gst_object_unref(media->audio_resampler);

	if (media->audio_filter != NULL)
			gst_object_unref(media->audio_filter);

	g_mutex_clear(&(media->mutex));

	g_free (media->name);
	g_free (media->video_pad_name);
	g_free (media->audio_pad_name);

	media->name = NULL;
	media->video_pad_name = NULL;
	media->audio_pad_name = NULL;

	media->x_pos = 0;
	media->y_pos = 0;
	media->width = 0;
	media->height = 0;
	media->z_index = 0;
	media->alpha = 0.0;
	media->volume = 0.0;

	media->name = NULL;

	free (media);
}

void
mb_clean_up ()
{
	if (_global.bus != NULL)
		gst_object_unref(_global.bus);

	if (_global.pipeline != NULL)
	{
		gst_object_unref(_global.clock_provider);

		gst_element_set_state(_global.pipeline, GST_STATE_NULL);
		gst_object_unref(_global.pipeline);
	}

	if (_global.loop != NULL)
	{
		g_main_loop_quit(_global.loop);
		g_main_loop_unref(_global.loop);
		g_thread_unref(_global.loop_thread);
	}

	gst_deinit();
}
