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

GMainLoop *loop = NULL;

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
			if (count == 3)
				g_main_loop_quit(loop);

			break;
		}
		default:
			g_printerr ("Unknown event received!\n");
	}
}
int main (int argc, char *argv[])
{
	MbMedia *medias[3];
  int i;

  if (!mb_init_args (400, 80))
   	return -1;

  mb_register_handler(handler);

  loop = g_main_loop_new (NULL, FALSE);

 	medias[0] = mb_media_new ("media1", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video1.mp4", 0, 0, 0, 128, 78);

 	medias[1] = mb_media_new ("media2", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video2.mkv", 130, 0, 0, 128, 78);

 	medias[2] = mb_media_new ("media3", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/video3.avi", 260, 0, 0, 128, 78);


  mb_media_start(medias[0]);
  mb_media_start(medias[1]);
  mb_media_start(medias[2]);

  sleep (20);

  mb_media_stop(medias[0]);
	mb_media_stop(medias[1]);
	mb_media_stop(medias[2]);

	g_print ("Running...\n");
	g_main_loop_run (loop);

  for (i = 0; i < 3; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
