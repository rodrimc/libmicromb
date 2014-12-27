/*
 ============================================================================
 Name        : Libmb_tests.c
 Author      : Rodrigo Costa
 Version     :
 Copyright   : GPL
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <gst/gst.h>
#include <unistd.h>

#include "mb/mb.h"

int main (int argc, char *argv[])
{
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  MbMedia **medias;
  int width, height, x;

  int i, n;
  if (argc < 2)
  {
		g_print ("Usage: %s <media uri> ...\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }

  if (mb_init () != 0)
   	return -1;

  width = mb_get_window_width ();
  height = mb_get_window_height ();

  printf ("%dx%d\n", width, height);

  n = argc - 1;

  medias = (MbMedia **) malloc (sizeof (MbMedia*) * n);
  for (i = 1; i <= n; i++)
  {
  	gchar alias_buff [8];
  	sprintf (alias_buff, "media_%d", i);

  	medias[i-1] = mb_media_new (alias_buff, argv[i],
																width / 2 - 640 / 2, height / 2 - 480 / 2, 0,
																640, 480);
  }

  for (i = 0; i < n; i++)
  {
  	if (mb_media_start(medias[i]) != 0)
  	{
  		g_printerr ("Unable to set the pipeline to the playing state.\n");
  	}
  }

  sleep (2);

  mb_media_set_z(medias[0], 0);
  mb_media_set_alpha(medias[0], 0.9);

  mb_media_set_z(medias[1], 1);
  mb_media_set_alpha(medias[1], 0.5);

  mb_media_set_z(medias[2], 2);
  mb_media_set_alpha(medias[2], 0.3);

  for (x = 0; x + 640 < width; x += 150)
  {
  	sleep (1);
  	mb_media_set_pos(medias[0], x, 0);
  	mb_media_set_pos(medias[2], x, height - 480);
  }

  mb_media_set_size_property(medias[0], "width", 100);
  mb_media_set_size_property(medias[1], "width", 200);
  mb_media_set_size_property(medias[2], "height", 100);

  bus = mb_get_message_bus();

  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      GST_MESSAGE_EOS | GST_MESSAGE_ERROR);

  if (msg != NULL)
  {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg))
    {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s.\n",
                    GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ?
        		debug_info : "none");
        g_clear_error (&err);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        g_printerr ("Unexpected message received.\n");
        break;
    }

    gst_message_unref (msg);
  }

  for (i = 0; i < n; i++)
  	mb_media_free(medias[i]);

  gst_object_unref (bus);

  mb_clean_up();
  return 0;
}



//#include <gst/gst.h>
//
//static GMainLoop *loop;
//
//static void
//cb_need_data (GstElement *appsrc,
//	      guint       unused_size,
//	      gpointer    user_data)
//{
//  static gboolean white = FALSE;
//  static GstClockTime timestamp = 0;
//  GstBuffer *buffer;
//  guint size;
//  GstFlowReturn ret;
//
//  g_print ("cb_need_data\n");
//
//  size = 385 * 288 * 2;
//
//  buffer = gst_buffer_new_allocate (NULL, size, NULL);
//
//  /* this makes the image black/white */
//  gst_buffer_memset (buffer, 0, white ? 0xff : 0x0, size);
//
//  white = !white;
//
//  GST_BUFFER_PTS (buffer) = timestamp;
//  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale_int (1, GST_SECOND, 2);
//
//  timestamp += GST_BUFFER_DURATION (buffer);
//
//  g_signal_emit_by_name (appsrc, "push-buffer", buffer, &ret);
//
//  if (ret != GST_FLOW_OK) {
//    /* something wrong, stop pushing */
//    g_main_loop_quit (loop);
//  }
//}
//
//gint
//main (gint   argc,
//      gchar *argv[])
//{
//  GstElement *pipeline, *appsrc, *conv, *videosink;
//  GstBus *bus;
//  GstMessage *msg;
//
//  /* init GStreamer */
//  gst_init (&argc, &argv);
//  loop = g_main_loop_new (NULL, FALSE);
//
//  /* setup pipeline */
//  pipeline = gst_pipeline_new ("pipeline");
//  appsrc = gst_element_factory_make ("appsrc", "source");
//  conv = gst_element_factory_make ("videoconvert", "conv");
//  videosink = gst_element_factory_make ("xvimagesink", "videosink");
//
//  /* setup */
//  g_object_set (G_OBJECT (appsrc), "caps",
//  		gst_caps_new_simple ("video/x-raw",
//				     "format", G_TYPE_STRING, "RGB16",
//				     "width", G_TYPE_INT, 384,
//				     "height", G_TYPE_INT, 288,
//				     "framerate", GST_TYPE_FRACTION, 0, 1,
//				     NULL), NULL);
//  gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, videosink, NULL);
//  gst_element_link_many (appsrc, conv, videosink, NULL);
//
//  /* setup appsrc */
//  g_object_set (G_OBJECT (appsrc),
//		"stream-type", 0,
//		"format", GST_FORMAT_TIME, NULL);
//  g_signal_connect (appsrc, "need-data", G_CALLBACK (cb_need_data), NULL);
//
//  /* play */
//  gst_element_set_state (pipeline, GST_STATE_PLAYING);
////  g_main_loop_run (loop);
//  bus = gst_element_get_bus (pipeline);
//
//	msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
//																		GST_MESSAGE_EOS | GST_MESSAGE_ERROR);
//
//	if (msg != NULL)
//	{
//		GError *err;
//		gchar *debug_info;
//
//		switch (GST_MESSAGE_TYPE(msg))
//		{
//			case GST_MESSAGE_ERROR:
//				gst_message_parse_error (msg, &err, &debug_info);
//				g_printerr ("Error received from element %s: %s.\n",
//										GST_OBJECT_NAME(msg->src), err->message);
//				g_printerr ("Debugging information: %s\n",
//										debug_info ? debug_info : "none");
//				g_clear_error (&err);
//				break;
//			case GST_MESSAGE_EOS:
//				g_print ("End-Of-Stream reached.\n");
//				break;
//			default:
//				g_printerr ("Unexpected message received.\n");
//				break;
//		}
//
//		gst_message_unref (msg);
//	}
//
//  /* clean up */
//  gst_element_set_state (pipeline, GST_STATE_NULL);
//
//  gst_object_unref (GST_OBJECT (pipeline));
////  g_main_loop_unref (loop);
//
//  return 0;
//  }
//
