/*
 * util.c
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#include "util.h"

int
init (int width, int height)
{
	GstStateChangeReturn ret;
	if (!gst_init_check(NULL, NULL, NULL))
	{
		g_printerr ("Failed to initialize gstreamer...\n");
		return -1;
	}

	_global.pipeline = gst_pipeline_new ("pipeline");
	g_assert (_global.pipeline);

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
		return FAILURE;
	}

	_global.window_width = width;
	_global.window_height = height;

	return SUCCESS;
}

void
pad_added_handler (GstElement *src, GstPad *new_pad, MbMedia *media)
{
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	GstElement *bin = NULL;
	const char *new_pad_type = NULL;

	g_print ("Received new pad '%s' from '%s'\n", GST_PAD_NAME(new_pad),
					 media->name);

	new_pad_caps = gst_pad_query_caps (new_pad, NULL);
	new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);

	g_print ("New pad type: %s\n", new_pad_type);

	bin = gst_bin_get_by_name(GST_BIN(_global.pipeline), media->name);
	g_assert (bin);

	if (g_strcmp0(new_pad_type, "video/x-raw") == 0)
	{
		set_video_bin (bin, media, new_pad);
	}
	else if (g_strcmp0(new_pad_type, "audio/x-raw") == 0)
	{
		set_audio_bin (bin, media, new_pad);
	}

	gst_object_unref(bin);

	if (new_pad_caps != NULL)
		gst_caps_unref (new_pad_caps);
}

int
set_video_elements ()
{
	_global.video_sink = gst_element_factory_make("xvimagesink", "videosink");
	g_assert (_global.video_sink);

	g_object_set (G_OBJECT (_global.video_sink), "window-width",
								_global.window_width, NULL);
	g_object_set (G_OBJECT (_global.video_sink), "window-height",
								_global.window_height, NULL);

	gst_element_set_state(_global.video_sink, GST_STATE_PAUSED);
	gst_bin_add(GST_BIN (_global.pipeline), _global.video_sink);
	gst_element_set_state(_global.video_sink, GST_STATE_PLAYING);

	_global.v_init = 1;

	return SUCCESS;
}

int
set_audio_elements ()
{
	_global.audio_sink = gst_element_factory_make	("autoaudiosink", "audiosink");
	_global.audio_mixer = gst_element_factory_make("adder", "audiomixer");

	g_assert (_global.audio_sink);
	g_assert (_global.audio_mixer);

	gst_element_set_state(_global.audio_mixer, GST_STATE_PAUSED);
	gst_element_set_state(_global.audio_sink, GST_STATE_PAUSED);

	gst_bin_add_many(GST_BIN (_global.pipeline), _global.audio_mixer,
									 _global.audio_sink, NULL);

	if (!gst_element_link(_global.audio_mixer, _global.audio_sink))
	{
		g_printerr ("Could not link audio mixer and audio_sink together\n");
		return FAILURE;
	}

	gst_element_set_state(_global.audio_mixer, GST_STATE_PLAYING);
	gst_element_set_state(_global.audio_sink, GST_STATE_PLAYING);

	_global.a_init = 1;

	return 0;
}

int
set_video_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad)
{
	GstElement *video_scaler = NULL, *caps_filter = NULL;
	GstCaps *caps = NULL;
	GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;
	int width = 1080, height = 480;

	//Create video sink on demand

	//The following test is for optimization purposes: there's no
	//need to always try the lock, because the 'set_video_elements'
	//function should execute only once as soon as the first video
	//stream is decoded.
	if (_global.v_init == 0)
	{
		g_mutex_lock (&_global.v_mutex);
		if (!_global.video_sink && set_video_elements() != SUCCESS)
		{
			g_mutex_unlock (&_global.v_mutex);
			g_printerr ("Unable to set video sink\n");
			return -1;
		}
		g_mutex_unlock (&_global.v_mutex);
	}

	video_scaler = gst_element_factory_make ("videoscale", NULL);
	caps_filter = gst_element_factory_make ("capsfilter", NULL);

	g_assert (video_scaler);
	g_assert (caps_filter);

	gst_element_set_state(video_scaler, GST_STATE_PAUSED);
	gst_element_set_state(caps_filter, GST_STATE_PAUSED);

	caps = gst_caps_new_simple ("video/x-raw",
															"pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
															"width", G_TYPE_INT, width,
															"height", G_TYPE_INT, height,
															NULL);

	g_object_set (G_OBJECT (video_scaler), "add-borders", 0, NULL);
	g_object_set (G_OBJECT (caps_filter), "caps", caps, NULL);

	gst_bin_add_many(GST_BIN (bin), video_scaler, caps_filter, NULL);
	if (!gst_element_link (video_scaler, caps_filter))
	{
		g_printerr ("Could not link elements together");
		gst_object_unref (video_scaler);
		gst_object_unref (caps_filter);
		return FAILURE;
	}

	sink_pad = gst_element_get_static_pad (video_scaler, "sink");
	if (sink_pad)
	{
		GstPadLinkReturn ret = gst_pad_link (decoder_src_pad, sink_pad);
		if (GST_PAD_LINK_FAILED(ret))
			g_print (" Link failed.\n");
		else
			g_print (" Link succeeded.\n");
	}

	media->video_scaler = GST_ELEMENT_NAME (video_scaler);
	media->video_filter= GST_ELEMENT_NAME (caps_filter);

	gst_element_set_state (video_scaler, GST_STATE_PLAYING);
	gst_element_set_state (caps_filter, GST_STATE_PLAYING);

	ghost_pad = gst_ghost_pad_new (
			"v_src", gst_element_get_static_pad (caps_filter, "src"));

	gst_pad_set_active (ghost_pad, TRUE);
	gst_element_add_pad (bin, ghost_pad);

	output_sink_pad = gst_element_get_static_pad (_global.video_sink, "sink");

	if (output_sink_pad)
		gst_pad_link (ghost_pad, output_sink_pad);

	gst_caps_unref(caps);
	gst_object_unref (output_sink_pad);
	gst_object_unref(sink_pad);

	return 0;
}

int
set_audio_bin(GstElement *bin, MbMedia *media, GstPad *decoder_src_pad)
{
	GstElement *audio_converter = NULL;
	GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;

	//Create video sink on demand

	//The following test is also for optimization purposes
	//(see the comments on function set_video_bin).
	if (_global.a_init == 0)
	{
		g_mutex_lock (&_global.a_mutex);
		if (!_global.audio_sink && set_audio_elements () != 0)
		{
			g_printerr ("Unable to set video sink\n");
			g_mutex_unlock (&_global.a_mutex);
			return FAILURE;
		}
		g_mutex_unlock (&_global.a_mutex);
	}

	audio_converter = gst_element_factory_make ("audioconvert", NULL);
	g_assert (audio_converter);

	gst_element_set_state (audio_converter, GST_STATE_PAUSED);

	gst_bin_add (GST_BIN(bin), audio_converter);

	sink_pad = gst_element_get_static_pad (audio_converter, "sink");
	if (sink_pad)
	{
		GstPadLinkReturn ret = gst_pad_link (decoder_src_pad, sink_pad);
		if (GST_PAD_LINK_FAILED(ret))
		{
			g_print (" Link failed.\n");
		}
		else
		{
			g_print (" Link succeeded.\n");
		}
	}

	media->audio_converter = GST_ELEMENT_NAME(audio_converter);

	gst_element_set_state (audio_converter, GST_STATE_PLAYING);

	ghost_pad = gst_ghost_pad_new (
			"a_src", gst_element_get_static_pad (audio_converter, "src"));

	gst_pad_set_active (ghost_pad, TRUE);
	gst_element_add_pad (bin, ghost_pad);

	output_sink_pad = gst_element_get_request_pad (_global.audio_mixer,
																								"sink_%u");

	if (output_sink_pad)
		gst_pad_link (ghost_pad, output_sink_pad);

	gst_object_unref (output_sink_pad);
	gst_object_unref (sink_pad);
	return SUCCESS;
}

