/*
 * util.c
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
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
	GstElement *media_bin = NULL;
	size_t lenght;
	MbMedia *media;

	media = (MbMedia *) malloc (sizeof (MbMedia));

	media_bin = gst_bin_new (media_name);
	media->decoder = gst_element_factory_make ("uridecodebin", NULL);
	media->video_scaler = gst_element_factory_make ("videoscale", NULL);
	media->video_filter = gst_element_factory_make ("capsfilter", NULL);
	media->audio_volume = gst_element_factory_make ("volume", NULL);

	media->audio_converter = NULL;
	media->audio_resampler = NULL;
	media->audio_filter = NULL;


	lenght = strlen(media_name);
	media->name = (char *) malloc ((lenght + 1)  * sizeof (char));
	strcpy(media->name, media_name);

	media->video_pad_name  = NULL;
	media->audio_pad_name  = NULL;

	//properties
	media->x_pos = x;
	media->y_pos = y;
	media->width = width;
	media->height = height;
	media->z_index = z;
	media->alpha = 1;

	g_object_set (media->decoder, "uri", uri, NULL);
	g_signal_connect (media->decoder, "pad-added", G_CALLBACK (pad_added_handler),
											media);

	gst_bin_add (GST_BIN(media_bin), media->decoder);
	gst_bin_add (GST_BIN(_global.pipeline), media_bin);

	return media;
}

int
mb_media_start (MbMedia *media)
{
	GstStateChange ret;
	GstElement *element;

	g_assert (media != NULL);

	element = gst_bin_get_by_name (GST_BIN(_global.pipeline), media->name);
	g_assert (element);

	ret = gst_element_set_state (element, GST_STATE_PLAYING);

	gst_object_unref(element);

	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		return FAILURE;
	}

	return SUCCESS;
}

GstBus *
mb_get_message_bus ()
{
	return gst_element_get_bus (_global.pipeline);
}

int
mb_media_set_size (MbMedia *media, int width, int height)
{
	GstCaps *new_caps = NULL;
	GstPad *filter_sink_pad = NULL;

	g_assert (media != NULL);

	if (!media->video_filter)
	{
		g_printerr ("Video stream not found\n");
		return FAILURE;
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

	//Cleaning up
	gst_caps_unref(new_caps);
	gst_object_unref(filter_sink_pad);

	return SUCCESS;
}

int
mb_media_set_pos (MbMedia *media, int x, int y)
{
	GstElement *element;
	int return_code = SUCCESS;

	g_assert (media != NULL);

	element = gst_bin_get_by_name (GST_BIN(_global.pipeline), media->name);
	g_assert (element);

	if (media->video_pad_name == NULL)
	{
		return_code = FAILURE;
		g_printerr ("This media has no video output.\n");
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
			return_code = FAILURE;

		gst_object_unref(video_mixer_pad);
	}

	gst_object_unref (element);

	return return_code;
}

int
mb_media_set_z (MbMedia *media, int z)
{
	GstElement *element;
	int return_code = SUCCESS;

	g_assert (media != NULL);

	element = gst_bin_get_by_name (GST_BIN(_global.pipeline), media->name);
	g_assert (element);

	if (media->video_pad_name == NULL)
	{
		return_code = FAILURE;
		g_printerr ("This media has no visual output\n");
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
			return_code = FAILURE;

		gst_object_unref(video_mixer_pad);
	}

	gst_object_unref (element);

	return return_code;
}

int
mb_media_set_alpha (MbMedia *media, double alpha)
{
	GstElement *element;
	int return_code = SUCCESS;

	g_assert (media != NULL);

	element = gst_bin_get_by_name (GST_BIN(_global.pipeline), media->name);
	g_assert (element);

	if (media->video_pad_name == NULL)
	{
		return_code = FAILURE;
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
			return_code = FAILURE;

		gst_object_unref(video_mixer_pad);
	}

	gst_object_unref (element);

	return return_code;
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

	g_free (media->name);
	g_free (media->decoder);
	g_free (media->video_scaler);
	g_free (media->video_filter);
	g_free (media->audio_converter);
	g_free (media->video_pad_name);
	g_free (media->audio_pad_name);
	free (media);

	media = NULL;
}


void
mb_clean_up ()
{
	if (_global.pipeline)
	{
		gst_element_set_state(_global.pipeline, GST_STATE_NULL);
		gst_object_unref(_global.pipeline);
	}
}
