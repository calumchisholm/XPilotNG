/*
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient_x11.h"

char xevent_version[] = VERSION;


bool		initialPointerControl = false;
bool		pointerControl = false;

int	talk_key_repeating;
XEvent	talk_key_repeat_event;
struct timeval talk_key_repeat_time;
static struct timeval time_now;

static ipos_t	delta;
ipos_t	mousePosition;	/* position of mouse pointer. */
int	mouseMovement;	/* horizontal mouse movement. */


keys_t Lookup_key(XEvent *event, KeySym ks, bool reset)
{
    keys_t ret = Generic_lookup_key((xp_keysym_t)ks, reset);

    UNUSED_PARAM(event);
    IFWINDOWS( Trace("Lookup_key: got key ks=%04X ret=%d\n", ks, ret) );
  
#ifdef DEVELOPMENT
    if (reset && ret == KEY_DUMMY) {
	static XComposeStatus compose;
	char str[4];
	int count;

	memset(str, 0, sizeof str);
	count = XLookupString(&event->xkey, str, 1, &ks, &compose);
	if (count == NoSymbol)
	    warn("Unknown keysym: 0x%03lx.", ks);
	else {
	    warn("No action bound to keysym 0x%03lx.", ks);
	    if (*str)
		warn("(which is key \"%s\")", str);
	}
    }
#endif

    return ret;
}

void Pointer_control_set_state(bool on)
{
    if (on) {
	pointerControl = true;
	XGrabPointer(dpy, drawWindow, true, 0, GrabModeAsync,
		     GrabModeAsync, drawWindow, pointerControlCursor,
		     CurrentTime);
	XWarpPointer(dpy, None, drawWindow,
		     0, 0, 0, 0,
		     (int)draw_width/2, (int)draw_height/2);
	XDefineCursor(dpy, drawWindow, pointerControlCursor);
	XSelectInput(dpy, drawWindow,
		     PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
    } else {
	pointerControl = false;
	XUngrabPointer(dpy, CurrentTime);
	XDefineCursor(dpy, drawWindow, None);
	if (!selectionAndHistory)
	    XSelectInput(dpy, drawWindow, 0);
	else
	    XSelectInput(dpy, drawWindow, ButtonPressMask | ButtonReleaseMask);
	XFlush(dpy);
    }
}

#ifndef _WINDOWS

void Talk_set_state(bool on)
{

    if (on) {
	/* Enable talking, disable pointer control if it is enabled. */
	if (pointerControl) {
	    initialPointerControl = true;
	    Pointer_control_set_state(false);
	}
	if (selectionAndHistory)
	    XSelectInput(dpy, drawWindow, PointerMotionMask
			 | ButtonPressMask | ButtonReleaseMask);
	Talk_map_window(true);
    }
    else {
	/* Disable talking, enable pointer control if it was enabled. */
	Talk_map_window(false);
	if (initialPointerControl) {
	    initialPointerControl = false;
	    Pointer_control_set_state(true);
	}
    }
}

#else

static void Talk_set_state(bool on)
{
    char *wintalkstr;

    if (pointerControl) {
	initialPointerControl = true;
	Pointer_control_set_state(false);
    }

    wintalkstr = (char *)mfcDoTalkWindow();
    if (*wintalkstr)
	Net_talk(wintalkstr);

    if (initialPointerControl) {
	initialPointerControl = false;
	Pointer_control_set_state(true);
    }

    scoresChanged = true;
}
#endif

#ifndef _WINDOWS

bool Key_press_pointer_control(void)
{
    if (mouseAccelInClient) {    
	if (pre_exists && pointerControl)
	    XChangePointerControl(dpy, True, True, 
				  pre_acc_num, pre_acc_denom, pre_threshold);
	else
	    XChangePointerControl(dpy, True, True,
				  new_acc_num, new_acc_denom, new_threshold);
    }

    Pointer_control_set_state(!pointerControl);
    
    return false;	/* server doesn't need to know */
}

#else

bool Key_press_pointer_control(void)
{
    Pointer_control_set_state(!pointerControl);
    
    return false;	/* server doesn't need to know */
}

#endif

bool Key_press_swap_scalefactor(void)
{
    double tmp;

    tmp = scaleFactor;
    scaleFactor = scaleFactor_s;
    scaleFactor_s = tmp;

    Init_scale_array();
    Scale_dashes();
    Config_redraw();
    Bitmap_update_scale();

    return false;
}

bool Key_press_talk(void)
{
    Talk_set_state((talk_mapped == false) ? true : false);
    return false;	/* server doesn't need to know */
}

bool Key_press_toggle_radar_score(void)
{
    if (radar_score_mapped) {

	/* change the draw area to be the size of the window */
	draw_width = top_width;
	draw_height = top_height;

	/*
	 * We need to unmap the score and radar windows
	 * if config is mapped, leave it there its useful
	 * to have it popped up whilst in full screen
	 * the user can close it with "close"
	 */

	XUnmapWindow(dpy, radarWindow);
	XUnmapWindow(dpy, playersWindow);
	Widget_unmap(button_form);

	/* Move the draw area */
	XMoveWindow(dpy, drawWindow, 0, 0);

	/* Set the global variable to show that */
	/* the radar and score are now unmapped */
	radar_score_mapped = false;

	/* Generate resize event */
	Resize(topWindow, top_width, top_height);

    } else {

	/*
	 * We need to map the score and radar windows
	 * move the window back, note how 258 is a hard coded
	 * value in xinit.c, if they cant be bothered to declare
	 * a constant, neither can I - kps fix
	 */
	draw_width = top_width - (258);
	draw_height = top_height;

	XMoveWindow(dpy, drawWindow, 258, 0);
	Widget_map(button_form);
	XMapWindow(dpy, radarWindow);
	XMapWindow(dpy, playersWindow);

	/* reflect that we are remapped to the client */

	radar_score_mapped = true;
    }

    return false;
}

bool Key_press_toggle_record(void)
{
    Record_toggle();
    return false;	/* server doesn't need to know */
}

bool Key_press_toggle_fullscreen(void)
{
    return false;
}

void Key_event(XEvent *event)
{
    KeySym ks;

    if ((ks = XLookupKeysym(&event->xkey, 0)) == NoSymbol)
	return;

    switch(event->type) {
    case KeyPress:
	Keyboard_button_pressed((xp_keysym_t)ks);
	break;
    case KeyRelease:
	Keyboard_button_released((xp_keysym_t)ks);
	break;
    default:
	return;
    }
}

void Talk_event(XEvent *event)
{
    if (!Talk_do_event(event))
	Talk_set_state(false);
}

static void Handle_talk_key_repeat(void)
{
    int i;

    if (talk_key_repeating) {
	gettimeofday(&time_now, NULL);
	i = 1000000 * (time_now.tv_sec - talk_key_repeat_time.tv_sec) +
	    time_now.tv_usec - talk_key_repeat_time.tv_usec;
	if ((talk_key_repeating > 1 && i > 50000) || i > 500000) {
	    Talk_event(&talk_key_repeat_event);
	    talk_key_repeating = 2;
	    talk_key_repeat_time = time_now;
	    if (!talk_mapped)
		talk_key_repeating = 0;
	}
    }
}

#ifndef _WINDOWS

void xevent_keyboard(int queued)
{
    int i, n;
    XEvent event;

    Handle_talk_key_repeat();

    if (kdpy) {
	n = XEventsQueued(kdpy, queued);
	for (i = 0; i < n; i++) {
	    XNextEvent(kdpy, &event);
	    switch (event.type) {
	    case KeyPress:
	    case KeyRelease:
		Key_event(&event);
		break;

		/* Back in play */
	    case FocusIn:
		gotFocus = true;
		XAutoRepeatOff(kdpy);
		break;

		/* Probably not playing now */
	    case FocusOut:
	    case UnmapNotify:
		gotFocus = false;
		XAutoRepeatOn(kdpy);
		break;

	    case MappingNotify:
		XRefreshKeyboardMapping(&event.xmapping);
		break;

	    default:
		warn("Unknown event type (%d) in xevent_keyboard",
		     event.type);
		break;
	    }
	}
    }
}

void xevent_pointer(void)
{
    XEvent event;

    if (!pointerControl || talk_mapped)
	return;

    if (mouseMovement != 0) {
	Client_pointer_move(mouseMovement);
	delta.x = draw_width / 2 - mousePosition.x;
	delta.y = draw_height / 2 - mousePosition.y;
	if (ABS(delta.x) > 3 * draw_width / 8
	    || ABS(delta.y) > 1 * draw_height / 8) {

	    memset(&event, 0, sizeof(event));
	    event.type = MotionNotify;
	    event.xmotion.display = dpy;
	    event.xmotion.window = drawWindow;
	    event.xmotion.x = draw_width/2;
	    event.xmotion.y = draw_height/2;
	    XSendEvent(dpy, drawWindow, False,
		       PointerMotionMask, &event);
	    XWarpPointer(dpy, None, drawWindow,
			 0, 0, 0, 0,
			 (int)draw_width/2, (int)draw_height/2);
	    XFlush(dpy);
	}
    }
}

int x_event(int new_input)
{
    int queued = 0, i, n;
    XEvent event;

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

    mouseMovement = 0;

    switch (new_input) {
    case 0: queued = QueuedAlready; break;
    case 1: queued = QueuedAfterReading; break;
    case 2: queued = QueuedAfterFlush; break;
    default:
	warn("Bad input queue type (%d)", new_input);
	return -1;
    }
    n = XEventsQueued(dpy, queued);
    for (i = 0; i < n; i++) {
	XNextEvent(dpy, &event);

	switch (event.type) {
	    /*
	     * after requesting a selection we are notified that we
	     * can access it.
	     */
	case SelectionNotify:
	    SelectionNotify_event(&event);
	    break;
	    /*
	     * we are requested to provide a selection.
	     */
	case SelectionRequest:
	    SelectionRequest_event(&event);
	    break;

	case SelectionClear:
	    if (selectionAndHistory)
		Clear_selection();
	    break;

	case MapNotify:
	    MapNotify_event(&event);
	    break;

	case ClientMessage:
	    if (ClientMessage_event(&event) == -1)
		return -1;
	    break;

	    /* Back in play */
	case FocusIn:
	    FocusIn_event(&event);
	    break;

	    /* Probably not playing now */
	case FocusOut:
	case UnmapNotify:
	    UnmapNotify_event(&event);
	    break;

	case MappingNotify:
	    XRefreshKeyboardMapping(&event.xmapping);
	    break;


	case ConfigureNotify:
	    ConfigureNotify_event(&event);
	    break;


	case KeyPress:
	    talk_key_repeating = 0;
	    /* FALLTHROUGH */
	case KeyRelease:
	    KeyChanged_event(&event);
	    break;

	case ButtonPress:
	    ButtonPress_event(&event);
	    break;

	case MotionNotify:
	    MotionNotify_event(&event);
	    break;

	case ButtonRelease:
	    if (ButtonRelease_event(&event) == -1)
	        return -1;
	    break;

	case Expose:
	    Expose_event(&event);
	    break;

	case EnterNotify:
	case LeaveNotify:
	    Widget_event(&event);
	    break;

	default:
	    break;
	}
    }

    xevent_keyboard(queued);
    xevent_pointer();
    return 0;
}

#else  /* _WINDOWS */

void xevent_keyboard(int queued)
{
    Handle_talk_key_repeat();
}

void xevent_pointer(void)
{
    POINT point;

    if (!pointerControl || talk_mapped)
	return;

    GetCursorPos(&point);
    mouseMovement = point.x - draw_width/2;
    XWarpPointer(dpy, None, drawWindow,
		 0, 0, 0, 0,
		 draw_width/2, draw_height/2);

    if (mouseMovement != 0) {
	Send_pointer_move(mouseMovement);
	delta.x = draw_width / 2 - mousePosition.x;
	delta.y = draw_height / 2 - mousePosition.y;
	if (ABS(delta.x) > 3 * draw_width / 8
	    || ABS(delta.y) > 1 * draw_height / 8)
	    XFlush(dpy);
    }
}

int win_xevent(XEvent event)
{
    int queued = 0;

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

    mouseMovement = 0;

    switch (event.type) {
    case KeyPress:
	talk_key_repeating = 0;
	/* FALLTHROUGH */
    case KeyRelease:
	KeyChanged_event(&event);
	break;

    case ButtonPress:
	ButtonPress_event(&event);
	break;

    case MotionNotify:
	MotionNotify_event(&event);
	break;

    case ButtonRelease:
	if (ButtonRelease_event(&event) == -1)
	    return -1;
	break;

    case Expose:
	Expose_event(&event);
	break;

    case EnterNotify:
    case LeaveNotify:
	Widget_event(&event);
	break;

    default:
	break;
    }

    xevent_keyboard(queued);
    xevent_pointer();
    return 0;
}

#endif /* _WINDOWS */
