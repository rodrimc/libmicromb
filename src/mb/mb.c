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
mb_media_new (const gchar *uri, const gchar *media_alias)
{
	GstElement *media_bin = NULL, *uri_decoder = NULL;
	size_t m_lenght;
	MbMedia *media;

	media_bin = gst_bin_new (media_alias);
	uri_decoder = gst_element_factory_make ("uridecodebin", NULL);

	m_lenght = strlen(media_alias);


	media = (MbMedia *) malloc (sizeof (MbMedia));
	media->name = (char *) malloc ((m_lenght + 1)  * sizeof (char));
	strcpy(media->name, media_alias);

	media->decoder = GST_ELEMENT_NAME(uri_decoder);

	media->video_filter = NULL;
	media->video_scaler = NULL;
	media->audio_converter = NULL;
	media->video_pad_name = NULL;
	media->audio_pad_name = NULL;

	g_object_set (uri_decoder, "uri", uri, NULL);
	g_signal_connect (uri_decoder, "pad-added", G_CALLBACK (pad_added_handler),
											media);

	gst_bin_add (GST_BIN(media_bin), uri_decoder);
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
mb_media_set_size_property (MbMedia *media, const char *property, int value)
{
	GstElement *video_filter = NULL;
	GstCaps *current_caps = NULL, *new_caps = NULL;
	GstPad *filter_sink_pad = NULL;
	GValue g_value = G_VALUE_INIT;

	g_assert (media != NULL);

	if (!media->video_filter)
	{
		g_printerr ("Video stream not found\n");
		return FAILURE;
	}

	g_value_init (&g_value, G_TYPE_INT);
	g_value_set_int (&g_value, value);

	video_filter = gst_bin_get_by_name (GST_BIN (_global.pipeline),
																media->video_filter);
	g_assert (video_filter);

	filter_sink_pad = gst_element_get_static_pad(video_filter, "sink");
	g_assert (filter_sink_pad);

	current_caps = gst_pad_get_current_caps(filter_sink_pad);
	g_assert (current_caps);

	new_caps = gst_caps_copy(current_caps);
	g_assert(new_caps);
	gst_caps_set_value(new_caps, property, &g_value);

	g_object_set (G_OBJECT(video_filter), "caps", new_caps, NULL);

	//Cleaning up
	g_value_unset (&g_value);
	gst_caps_unref(current_caps);
	gst_caps_unref(new_caps);
	gst_object_unref(video_filter);

	return SUCCESS;
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
