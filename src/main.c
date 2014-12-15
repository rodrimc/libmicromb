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

  int i, n;
  if (argc < 2)
  {
		g_print ("Usage: %s <media uri> ...\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }

  if (mb_init() != 0)
   	return -1;

  n = argc - 1;

  medias = (MbMedia **) malloc (sizeof (MbMedia*) * n);
  for (i = 1; i <= n; i++)
  {
  	gchar alias_buff [8];
  	sprintf (alias_buff, "media_%d", i);

  	medias[i-1] = mb_media_new (argv[i], alias_buff);
  }

  for (i = 0; i < n; i++)
  {
  	if (mb_media_start(medias[i]) != 0)
  	{
  		g_printerr ("Unable to set the pipeline to the playing state.\n");
  	}
  }

  sleep (2);
//  mb_media_set_size_property(medias[0], "width", 100);
  mb_media_set_size_property(medias[0], "height", 300);



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


//#include <stdlib.h>
//#include <stdio.h>
//#include <gst/gst.h>
//
//#define MAX_ROUND 100
//
//int
//main (int argc, char **argv)
//{
//  GstElement *pipe, *filter;
//  GstCaps *caps;
//  gint width, height;
//  gint xdir, ydir;
//  gint round;
//  GstMessage *message;
//  gchar buff[100];
//
//  gst_init (&argc, &argv);
//
//  sprintf(buff, "uridecodebin uri=%s ! capsfilter name=filter ! "
//             "autovideosink", argv[1]);
//
//  printf ("%s\n", buff);
//
//  pipe = gst_parse_launch_full (buff, NULL, GST_PARSE_FLAG_NONE, NULL);
//  g_assert (pipe != NULL);
//
//  filter = gst_bin_get_by_name (GST_BIN (pipe), "filter");
//  g_assert (filter);
//
//  width = 320;
//  height = 240;
//  xdir = ydir = -10;
//
//  gst_element_set_state (pipe, GST_STATE_PLAYING);
//
//  sleep (4);
//
//  for (round = 0; round < MAX_ROUND; round++) {
//    gchar *capsstr;
//    g_print ("resize to %dx%d (%d/%d)   \r", width, height, round, MAX_ROUND);
//
//    /* we prefer our fixed width and height but allow other dimensions to pass
//     * as well */
//    capsstr = g_strdup_printf ("video/x-raw, width=(int)%d, height=(int)%d",
//        width, height);
//
//    caps = gst_caps_from_string (capsstr);
//    g_free (capsstr);
//    g_object_set (filter, "caps", caps, NULL);
//    gst_caps_unref (caps);
//
//    width += xdir;
//    if (width >= 320)
//      xdir = -10;
//    else if (width < 200)
//      xdir = 10;
//
//    height += ydir;
//    if (height >= 240)
//      ydir = -10;
//    else if (height < 150)
//      ydir = 10;
//
//    message =
//        gst_bus_poll (GST_ELEMENT_BUS (pipe), GST_MESSAGE_ERROR,
//        50 * GST_MSECOND);
//    if (message) {
//      g_print ("got error           \n");
//
//      gst_message_unref (message);
//    }
//  }
//
//  gst_bus_timed_pop_filtered (gst_element_get_bus(pipe), GST_CLOCK_TIME_NONE,
//        GST_MESSAGE_EOS | GST_MESSAGE_ERROR);
//
//  g_print ("done                    \n");
//
//  gst_object_unref (filter);
//  gst_element_set_state (pipe, GST_STATE_NULL);
//  gst_object_unref (pipe);
//
//  return 0;
//}
