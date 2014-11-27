/*
 * util.c
 *
 *  Created on: Nov 17, 2014
 *      Author: Rodrigo Costa
 *			e-mail: rodrigocosta@telemidia.puc-rio.br
 */

#include "util.h"

void
pad_added_handler (GstElement *src, GstPad *new_pad, Context *context)
{
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	g_print ("Received new pad '%s' from '%s'\n", GST_PAD_NAME(new_pad),
					 context->media->name);

	new_pad_caps = gst_pad_query_caps (new_pad, NULL);
	new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	new_pad_type = gst_structure_get_name (new_pad_struct);

	g_print ("New pad type: %s\n", new_pad_type);

	if (g_strcmp0(new_pad_type, "video/x-raw") == 0)
	{
		set_video_bin (context, new_pad);
	}
	else if (g_strcmp0(new_pad_type, "audio/x-raw") == 0)
	{
		set_audio_bin (context, new_pad);
	}

	if (new_pad_caps != NULL)
		gst_caps_unref (new_pad_caps);
}

int
set_video_elements (MbData *p_global)
{
	p_global->video_sink = gst_element_factory_make("autovideosink", "videosink");

	if (!p_global->video_sink)
	{
		g_printerr ("Could not create video sink element.\n");
		return -1;
	}

	gst_element_set_state(p_global->video_sink, GST_STATE_PAUSED);
	gst_bin_add(GST_BIN (p_global->pipeline), p_global->video_sink);
	gst_element_set_state(p_global->video_sink, GST_STATE_PLAYING);

	return 0;
}

int
set_audio_elements (MbData *p_global)
{
	p_global->audio_sink = gst_element_factory_make	("autoaudiosink", "audiosink");
	p_global->audio_mixer = gst_element_factory_make("adder", "audiomixer");

	if (!p_global->audio_mixer || !p_global->audio_sink)
	{
		g_printerr ("Could not create audio sink elements.\n");
		return -1;
	}

	gst_element_set_state(p_global->audio_mixer, GST_STATE_PAUSED);
	gst_element_set_state(p_global->audio_sink, GST_STATE_PAUSED);

	gst_bin_add_many(GST_BIN (p_global->pipeline), p_global->audio_mixer,
									 p_global->audio_sink, NULL);

	if (!gst_element_link(p_global->audio_mixer, p_global->audio_sink))
	{
		g_printerr ("Could not link audio mixer and audio_sink together\n");
		return -1;
	}

	gst_element_set_state(p_global->audio_mixer, GST_STATE_PLAYING);
	gst_element_set_state(p_global->audio_sink, GST_STATE_PLAYING);

	return 0;
}

int
set_video_bin(Context *context, GstPad *decoder_src_pad)
{
	GstElement *video_scaler = NULL, *caps_filter = NULL;
	GstCaps *caps = NULL;
	GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;
	int width = 680, height = 480;

	//Create video sink on demand
	if (!context->p_global->video_sink)
	{
		if (set_video_elements(context->p_global) != 0)
		{
			g_printerr ("Unable to set video sink\n");
			return -1;
		}
	}

	video_scaler = gst_element_factory_make ("videoscale", NULL);
	caps_filter = gst_element_factory_make ("capsfilter", NULL);

	if (!video_scaler || !caps_filter)
	{
		g_printf ("Unable to set video bin...\n");

		if (video_scaler) gst_object_unref (video_scaler);
		if (caps_filter) gst_object_unref (caps_filter);

		return -1;
	}

	gst_element_set_state(video_scaler, GST_STATE_PAUSED);
	gst_element_set_state(caps_filter, GST_STATE_PAUSED);

	caps = gst_caps_new_simple ("video/x-raw", "pixel-aspect-ratio",
															GST_TYPE_FRACTION,
															1, 1, "width", G_TYPE_INT, width, "height",
															G_TYPE_INT, height,
															NULL);

	g_object_set (G_OBJECT (video_scaler), "add-borders", 0, NULL);
	g_object_set (G_OBJECT (caps_filter), "caps", caps, NULL);

	gst_bin_add_many(GST_BIN (context->bin), video_scaler, caps_filter, NULL);
	if (!gst_element_link (video_scaler, caps_filter))
	{
		g_printerr ("Could not link elements together");
		gst_object_unref (video_scaler);
		gst_object_unref (caps_filter);
		return -1;
	}

	sink_pad = gst_element_get_static_pad (video_scaler, "sink");
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

	context->media->video_scaler = GST_ELEMENT_NAME (video_scaler);
	context->media->video_filter= GST_ELEMENT_NAME (caps_filter);

	gst_element_set_state(video_scaler, GST_STATE_PLAYING);
	gst_element_set_state (caps_filter, GST_STATE_PLAYING);

	ghost_pad = gst_ghost_pad_new (
			"v_src", gst_element_get_static_pad (caps_filter, "src"));

	gst_pad_set_active (ghost_pad, TRUE);
	gst_element_add_pad (context->bin, ghost_pad);

	output_sink_pad = gst_element_get_static_pad (context->p_global->video_sink,
	                                              "sink");

	if (output_sink_pad)
		gst_pad_link (ghost_pad, output_sink_pad);

	gst_object_unref (output_sink_pad);
	gst_object_unref(sink_pad);

	return 0;
}

int
set_audio_bin(Context *context, GstPad *decoder_src_pad)
{
	GstElement *audio_converter = NULL;
	GstPad *sink_pad = NULL, *ghost_pad = NULL, *output_sink_pad = NULL;

	//Create video sink on demand
	if (!context->p_global->audio_sink)
	{
		if (set_audio_elements (context->p_global) != 0)
		{
			g_printerr ("Unable to set video sink\n");
			return -1;
		}
	}

	audio_converter = gst_element_factory_make ("audioconvert", NULL);

	if (!audio_converter)
	{
		g_printf ("Unable to set audio bin...\n");

		return -1;
	}

	gst_element_set_state (audio_converter, GST_STATE_PAUSED);

	gst_bin_add (GST_BIN(context->bin), audio_converter);

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

	context->media->audio_converter = GST_ELEMENT_NAME(audio_converter);

	gst_element_set_state (audio_converter, GST_STATE_PLAYING);

	ghost_pad = gst_ghost_pad_new (
			"a_src", gst_element_get_static_pad (audio_converter, "src"));

	gst_pad_set_active (ghost_pad, TRUE);
	gst_element_add_pad (context->bin, ghost_pad);

	output_sink_pad = gst_element_get_request_pad (context->p_global->audio_mixer,
																								"sink_%u");

	if (output_sink_pad)
		gst_pad_link (ghost_pad, output_sink_pad);

	gst_object_unref (output_sink_pad);
	gst_object_unref (sink_pad);
	return 0;
}

