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
MbMedia *background;
MbMedia *anim;
MbMedia *choro;
MbMedia *drible;
MbMedia *shoes;


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
			if (evt->media == shoes)
			{
				mb_media_set_pos(anim, 0, 0);
				mb_media_set_size(anim, mb_get_window_width(),
													mb_get_window_height());
			}

			if (evt->media == anim)
				mb_media_stop(background);

			break;
		}
		case MB_REMOVED:
		{
			gboolean quit = FALSE;
			g_print ("%s has been removed from pipeline.\n", evt->media->name);
			if (evt->media == background)
				quit = TRUE;

			mb_media_free(evt->media);

			if (quit)
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
  int width = 800, height = 600;

  if (!mb_init_args (width, height))
   	return -1;

  n = 5;

  loop = g_main_loop_new (NULL, FALSE);

  mb_register_handler(handler);

  n = argc - 1;

  background = mb_media_new ("background", "file:////media/rodrigocosta/Data/"
														 "primeirojoao/media/background.png", 0, 0, 0,
														 width, height);

  anim = mb_media_new ("animation", "file:////media/rodrigocosta/Data/"
											 "primeirojoao/media/animGar.mp4", 0, 0, 1,
											 width, height);

  choro = mb_media_new ("choro", "file:////media/rodrigocosta/Data/"
												 "primeirojoao/media/choro.mp4", 0, 0, 0, 0, 0);

  drible = mb_media_new ("drible", "file:////media/rodrigocosta/Data/"
												 "primeirojoao/media/drible.mp4",
												 width * 0.05 /* left=5% */,
												 height * 0.067 /* top=6.7% */,
												 3 /* zIndex=3 */,
												 width * 0.185 /* width=18.5% */,
												 height * 0.185 /* height=18.5% */);

  shoes = mb_media_new ("shoes", "file:////media/rodrigocosta/Data/"
												 "primeirojoao/media/shoes.mp4",
												 width * 0.15 /* left=15% */,
												 height * 0.60 /* top=60% */,
												 3 /* zIndex=3 */,
												 width * 0.25 /* width=25% */,
												 height * 0.25 /* height=25% */);

  mb_media_start(background);
  mb_media_start(anim);
  sleep (5);

  mb_media_start(choro);
  sleep (7);

  mb_media_start(drible);
  sleep (33);

  mb_media_set_pos(anim, width * 0.05, height * 0.067);
  mb_media_set_size(anim, width * 0.45, height * 0.45);

  mb_media_start(shoes);

  g_print ("Running...\n");
	g_main_loop_run (loop);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
