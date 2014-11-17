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
#include "mb/mb.h"

int main (int argc, char *argv[])
{
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  int i, n;

  if (mb_init() != 0)
  	return -1;

  if (argc < 3)
  {
		g_print ("Usage: %s <media uri> <N> ...\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }
  n = atoi (argv[2]);

  for (i = 1; i <= n; i++)
  {
  	gchar alias_buff [8];
  	GstElement *media_bin;
  	sprintf (alias_buff, "media_%d", i);

  	media_bin = mb_add_media(argv[1], alias_buff);
  }

  if (mb_start(0))
  {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    return -1;
  }

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
        g_printerr ("Error received from element %s: %s.\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
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

  gst_object_unref (bus);
//  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
//  gst_object_unref (pipeline);
  return 0;
}

