/*
 * util.c
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#include "mb.h"
#include "util.h"

#include <assert.h>

static GlobalElements _global;

int
mb_init ()
{
	if (!gst_init_check(NULL, NULL, NULL))
	{
		g_printerr ("Failed to initialize gstreamer...");
		return -1;
	}

	_global.pipeline = gst_pipeline_new ("pipeline");
	_global.video_sink = gst_element_factory_make ("autovideosink", "video_sink");

	if (!_global.pipeline || !_global.video_sink)
	{
		g_printerr ("Could not create the pipeline element.\n");
		return -1;

	}

	gst_bin_add (GST_BIN(_global.pipeline), _global.video_sink);

	return 0;
}

GstElement *
mb_add_media (const gchar *uri, const gchar *media_alias)
{
	GstElement *media_bin = NULL, *uri_decoder = NULL;
	static Context context;

	media_bin = gst_bin_new (media_alias);
	uri_decoder = gst_element_factory_make ("uridecodebin", NULL);

	context.element = media_bin;
	context.p_global = &_global;

	g_object_set (uri_decoder, "uri", uri, NULL);
	g_signal_connect (uri_decoder, "pad-added", G_CALLBACK (pad_added_handler),
											&context);

	gst_bin_add (GST_BIN(media_bin), uri_decoder);
	gst_bin_add (GST_BIN(_global.pipeline), media_bin);

	return media_bin;
}

int
mb_start (GstElement* element)
{
	GstStateChange ret;

//	assert (element != NULL);
	ret = gst_element_set_state (_global.pipeline, GST_STATE_PLAYING);

	if (ret == GST_STATE_CHANGE_FAILURE)
	{
		gst_object_unref(_global.pipeline);
		return -1;
	}
	return 0;
}

GstBus *
mb_get_message_bus ()
{
	return gst_element_get_bus (_global.pipeline);
}
