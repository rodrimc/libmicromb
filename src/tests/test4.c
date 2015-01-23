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
			if (evt->media == medias[0])
				mb_media_start(medias[1]);
			else if (evt->media == medias[1])
				mb_media_start(medias[2]);
			break;
		}
		case MB_REMOVED:
		{
			g_print ("%s has been removed from pipeline.\n", evt->media->name);
			mb_media_free(evt->media);

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
  int width, height, x, i;

  if (!mb_init_args (800, 600))
   	return -1;

  loop = g_main_loop_new (NULL, FALSE);

  width = mb_get_window_width ();
  height = mb_get_window_height ();

  mb_register_handler(handler);

  n = 3;

  medias [0] = mb_media_new ("m_0", "file:////media/rodrigocosta/Data/"
														 "primeirojoao/media/shoes.mp4",
														 0, 0, 0, width, height);

  medias [1] = mb_media_new ("m_1", "file:////media/rodrigocosta/Data/"
  														 "primeirojoao/media/drible.mp4",
  														 0, 0, 0, width, height);

  medias [2] = mb_media_new ("m_2", "file:////media/rodrigocosta/Data/"
  														 "primeirojoao/media/animGar.mp4",
  														 0, 0, 0, width, height);


  mb_media_start(medias[0]);

  g_print ("Running...\n");
	g_main_loop_run (loop);
	g_print ("Quitting...\n");

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
