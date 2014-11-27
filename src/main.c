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

