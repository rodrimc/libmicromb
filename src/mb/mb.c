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

#include <assert.h>

static MbData _global;

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

	//audio
//	_global.audio_mixer = gst_element_factory_make ("adder", "audiomixer");
//	_global.audio_sink = gst_element_factory_make ("autoaudiosink", "audiosink");


//	if (!_global.pipeline || !_global.video_sink ||
//			!_global.audio_mixer || !_global.audio_sink)
//	{
//		g_printerr ("Could not create the pipeline element.\n");
//		mb_clean_up();
//		return -1;
//	}

//	gst_bin_add_many(GST_BIN(_global.pipeline), _global.video_sink, NULL);
//									 _global.audio_mixer, _global.audio_sink, NULL);
//	if (!gst_element_link(_global.audio_mixer, _global.audio_sink))
//	{
//		g_printerr ("Could not link audio mixer and audio sink together.\n");
//		mb_clean_up();
//		return -1;
//	}

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
	MbMedia *media;
	static Context context;

	media_bin = gst_bin_new (media_alias);
	uri_decoder = gst_element_factory_make ("uridecodebin", NULL);


	media = (MbMedia *) malloc (sizeof (MbMedia));
	media->name = media_alias;
	media->decoder = GST_ELEMENT_NAME(uri_decoder);

	media->video_filter = NULL;
	media->video_scaler = NULL;
	media->audio_converter = NULL;

	context.media = media;
	context.bin = media_bin;
	context.p_global = &_global;


	g_object_set (uri_decoder, "uri", uri, NULL);
	g_signal_connect (uri_decoder, "pad-added", G_CALLBACK (pad_added_handler),
											&context);

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

  if (!gst_element_seek (element, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
                         GST_SEEK_TYPE_SET, 0,
                         GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE)) {
    g_print ("Seek failed!\n");
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
