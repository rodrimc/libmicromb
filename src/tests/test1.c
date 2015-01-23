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




int main (int argc, char *argv[])
{
	MbMedia *medias[3];
	//GMainLoop *loop;
  int i;

  if (!mb_init_args (400, 80))
   	return -1;

//  loop = g_main_loop_new (NULL, FALSE);

 	medias[0] = mb_media_new ("media1", "file:///home/rodrigocosta/workspace/"
														"Libmb/resources/img1.jpg", 0, 0, 0, 128, 78);

 	medias[1] = mb_media_new ("media2", "file:///home/rodrigocosta/workspace/"
 														"Libmb/resources/img2.jpg", 130, 0, 0, 128, 78);

 	medias[2] = mb_media_new ("media3", "file:///home/rodrigocosta/workspace/"
 	 													"Libmb/resources/img3.jpg", 260, 0, 0, 128, 78);

  mb_media_start(medias[0]);
  mb_media_start(medias[1]);
  mb_media_start(medias[2]);

  sleep (2);

  mb_media_stop(medias[0]);
	mb_media_stop(medias[1]);
	mb_media_stop(medias[2]);

  sleep (2);

  for (i = 0; i < 3; i++)
  	mb_media_free(medias[i]);

  mb_clean_up();

  g_print ("All done!\n");

  return 0;
}
