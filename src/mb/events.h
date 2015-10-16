/*
 *
 *  Copyright (C) 2015 "Rodrigo Costa" <rodrigocosta@telemidia.puc-rio.br>
 * 
 *  This file is part of Libmicromb.
 *
 *  Libmicromb is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Libmicromb is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Libmicromb.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

typedef enum
{
  MB_MOUSE_LEFT_BUTTON = 1,
  MB_MOUSE_MIDDLE_BUTTON,
  MB_MOUSE_RIGHT_BUTTON
} MbMouseButton; /*!< This enum is used in MB_MOUSE_BUTTON_PRESS
                   and MB_MOUSE_BUTTON_RELEASE events. These values
                   should be consistent with the values used to
                   represent mouse buttons in the Xlib */

typedef enum
{
  /* FILL ME*/
  /* Application internal event */
  MB_APP_INIT_DONE = 0x100, 

  //Presentation
  MB_BEGIN        = 0x200,
  MB_END, 
  MB_PAUSE,
  MB_REMOVED, /*media has been removed from the pipeline.
                This means that from now on, it's safe to free it
                (mb_media_free) */

  /* Input */
  MB_MOUSE_BUTTON_PRESS  = 0x300,
  MB_MOUSE_BUTTON_RELEASE,
  MB_MOUSE_MOVE,
  MB_KEY_PRESS,
  MB_KEY_RELEASE,

  /* Attribution */
} MbEventType; /*!< This enum holds all the events this library
                 use to communicate with client applications */

typedef struct MbMouseButtonEvent
{
  uint32_t type;  /* MB_MOUSE_BUTTON_PRESS or MB_MOUSE_BUTTON_RELEASE */
  uint32_t button;
  int x;
  int y;
} MbMouseButtonEvent;

typedef struct MbMouseMoveEvent
{
  uint32_t type;  /* MB_MOUSE_MOVE*/
  int x;
  int y;
} MbMouseMoveEvent;

typedef struct MbKeyboardEvent
{
  uint32_t type; /* MB_KEY_PRESS or MB_KEY_RELEASE */
  const char *key;
} MbKeyboardEvent;

typedef struct MbStateChangeEvent
{
  uint32_t type;
  const char *media_name;
} MbStateChangeEvent;

typedef union MbEvent
{
  uint32_t type; /*this field should be present in all event structures*/
  MbStateChangeEvent state_change;
  MbMouseButtonEvent mouse_button;
  MbMouseMoveEvent mouse_move;
  MbKeyboardEvent keyboard;

} MbEvent; /*!< Represetation of events to clients of this library */

#endif //EVENTS_H
