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
#include <assert.h>


int
mb_init ()
{
	GstStateChangeReturn ret;
	if (!gst_init_check(NULL, NULL, NULL))
	{
		g_printerr ("Failed to initialize gstreamer...\n");
		return -1;
	}

	_global.pipeline = gst_pipeline_new ("pipeline");

	_global.video_mixer = NULL;
	_global.audio_mixer = NULL;
	_global.video_sink  = NULL;
	_global.audio_sink  = NULL;

	_global.v_init  = 0;
	_global.a_init  = 0;

	ret = gst_element_set_state (_global.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		mb_clean_up ();
		return -1;
	}

	return 0;
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

	assert (media != NULL);
	GstElement *element = gst_bin_get_by_name (GST_BIN(_global.pipeline),
	                                               media->name);

	if (!element) return -1;

	ret = gst_element_set_state (element, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		return -1;
	}

	return 0;
}

GstBus *
mb_get_message_bus ()
{
	return gst_element_get_bus (_global.pipeline);
}

void
mb_media_free (MbMedia *media)
{
	if (media)
	{
		g_free (media->name);
		g_free (media->decoder);
		g_free (media->video_scaler);
		g_free (media->video_filter);
		g_free (media->audio_converter);
		free (media);

		media = NULL;
	}
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
