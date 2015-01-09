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

int n, count = 0;
GMainLoop *loop;
MbMedia *medias[4];


void handler (MbMediaEvent *evt)
{
	switch (evt->evt)
	{
		case MB_BEGIN:
		{
			g_print ("%s has started.\n", evt->media->name);
			break;
		}
		case MB_PAUSE:
		{
			g_print ("%s has paused.\n", evt->media->name);
			break;
		}
		case MB_END:
		{
			g_print ("%s has ended.\n", evt->media->name);

			if (evt->media == medias[3])
			{
				mb_media_stop(medias[0]);
				mb_media_stop(medias[1]);
				mb_media_stop(medias[2]);
			}
			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);
			//mb_media_free(evt->media);

			count++;
			if (count == n)
				g_main_loop_quit(loop);

			break;
		}
		default:
			g_printerr ("Unknown event received!\n");
	}
}

int main (int argc, char *argv[])
{
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  int width, height, x;

  int i;
  if (argc < 2)
  {
		g_print ("Usage: %s <media uri> ...\n", argv[0]);
    g_print ("Exiting...\n");
    return -1;
  }

  if (!mb_init_args (800, 600))
   	return -1;

  loop = g_main_loop_new (NULL, FALSE);

  mb_register_handler(handler);

  width = mb_get_window_width ();
  height = mb_get_window_height ();

  printf ("%dx%d\n", width, height);

  n = argc - 1;

//  medias = (MbMedia **) malloc (sizeof (MbMedia*) * n);
  for (i = 1; i <= n; i++)
  {
  	gchar alias_buff [8];
  	sprintf (alias_buff, "media_%d", i);

  	medias[i-1] = mb_media_new (alias_buff, argv[i],
																0, 0, 0, width, height);
																//width / 2 - 640 / 2, height / 2 - 480 / 2, 0,
																//640, 480);
  }

  for (i = 0; i < n; i++)
  {
  	if (!mb_media_start(medias[i]))
  	{
  		g_printerr ("Unable to start %s.\n", medias[i]->name);
  	}
  }
  mb_media_set_size(medias[0], width, height);
  mb_media_set_z (medias[0], 0);
  mb_media_set_z (medias[1], 1);
  mb_media_set_pos(medias[1], width * 0.05, height * 0.067);
  mb_media_set_size(medias[1], width * 0.45, height * 0.45);
  mb_media_set_alpha (medias[1], 0.8);

  mb_media_set_pos(medias[3], width * 0.15, height * 0.60);
  mb_media_set_size(medias[3], width * 0.25, height * 0.25);
  mb_media_set_z(medias[3], 3);

  mb_media_set_volume(medias[2], 0.09);

  sleep (10);
  mb_media_stop(medias[3]);

//  sleep (2);
//
//  mb_media_set_z(medias[0], 0);
//  mb_media_set_alpha(medias[0], 0.9);
//
//  mb_media_set_z(medias[1], 1);
//  mb_media_set_alpha(medias[1], 0.5);
//
//  mb_media_set_z(medias[2], 2);
//  mb_media_set_alpha(medias[2], 0.3);
//
//  for (x = 0; x + 640 < width; x += 150)
//  {
//  	sleep (1);
//  	mb_media_set_pos(medias[0], x, 0);
//  	mb_media_set_pos(medias[2], x, height - 480);
//  }
//
//  mb_media_set_size (medias[0], 100, 480);
//  mb_media_set_size (medias[1], 200, 480);
//  mb_media_set_size (medias[2], 640, 100);


  g_print ("Running...\n");
	g_main_loop_run (loop);

  for (i = 0; i < n; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
