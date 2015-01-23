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
#include <unistd.h>

#include "mb/mb.h"

int n, count = 0;
GMainLoop *loop;
MbMedia *medias[2];

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

			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);

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
  int width = 800, height = 600, x;

  if (!mb_init_args (800, 600))
   	return -1;

  loop = g_main_loop_new (NULL, FALSE);
  n = 2;

  mb_register_handler(handler);

  medias[0] = mb_media_new ("media1", "file:///home/rodrigocosta/workspace/"
  													"Libmb/resources/lipsync.mp4", 0, 0, 0,
														width, height);

 	medias[1] = mb_media_new ("media2", "file:///home/rodrigocosta/workspace/"
   													"Libmb/resources/lipsync.mp3", 0, 0, 0, 0, 0);

  mb_media_start(medias[0]);

  sleep (5);
  mb_media_start(medias[1]);

  g_print ("Running...\n");
	g_main_loop_run (loop);

  mb_media_free(medias[0]);
  mb_media_free(medias[1]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
