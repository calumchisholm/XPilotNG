/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "xpclient_x11.h"

char xevent_version[] = VERSION;


bool		initialPointerControl = false;
bool		pointerControl = false;
extern Cursor	pointerControlCursor;

int num_keydefs = 0;
int max_keydefs = 0;
keydefs_t *keydefs = NULL;


#ifdef OPTIONHACK

typedef struct {
    KeySym xks;
    xp_keysym_t xpks;
} xp_xks2xpks_t;

xp_xks2xpks_t xks2xpks_table[] = {

    /* /usr/include/X11/keysymdef.h */

    { XK_BackSpace,	XP_KS_BACKSPACE },
    { XK_Tab,		XP_KS_TAB },
    { XK_Linefeed,	XP_KS_RETURN },
    { XK_Pause,		XP_KS_PAUSE },
    { XK_Scroll_Lock,	XP_KS_SCROLLOCK },
    { XK_Escape,	XP_KS_ESCAPE },
    { XK_Delete,	XP_KS_DELETE },

    { XK_Left,		XP_KS_LEFT },
    { XK_Up,		XP_KS_UP },
    { XK_Right,		XP_KS_RIGHT },
    { XK_Home,		XP_KS_HOME },
    { XK_Page_Up,	XP_KS_PAGEUP },
    { XK_Page_Down,	XP_KS_PAGEDOWN },
    { XK_End,		XP_KS_END },

    { XK_Insert,	XP_KS_INSERT },
    { XK_Num_Lock,	XP_KS_NUMLOCK },
    { XK_KP_Enter,	XP_KS_KP_ENTER },

    { XK_KP_Multiply,	XP_KS_KP_MULTIPLY },
    { XK_KP_Add,	XP_KS_KP_PLUS },
    { XK_KP_Subtract,	XP_KS_KP_MINUS },
    { XK_KP_Separator,	XP_KS_KP_PERIOD },
    { XK_KP_Divide,	XP_KS_KP_DIVIDE },
    { XK_KP_0,	XP_KS_KP0 },
    { XK_KP_1,	XP_KS_KP1 },
    { XK_KP_2,	XP_KS_KP2 },
    { XK_KP_3,	XP_KS_KP3 },
    { XK_KP_4,	XP_KS_KP4 },
    { XK_KP_5,	XP_KS_KP5 },
    { XK_KP_6,	XP_KS_KP6 },
    { XK_KP_7,	XP_KS_KP7 },
    { XK_KP_8,	XP_KS_KP8 },
    { XK_KP_9,	XP_KS_KP9 },

    { XK_F1,	XP_KS_F1 },
    { XK_F2,	XP_KS_F2 },
    { XK_F3,	XP_KS_F3 },
    { XK_F4,	XP_KS_F4 },
    { XK_F5,	XP_KS_F5 },
    { XK_F6,	XP_KS_F6 },
    { XK_F7,	XP_KS_F7 },
    { XK_F8,	XP_KS_F8 },
    { XK_F9,	XP_KS_F9 },
    { XK_F10,	XP_KS_F10 },
    { XK_F11,	XP_KS_F11 },
    { XK_F12,	XP_KS_F12 },

    { XK_Shift_L,	XP_KS_LSHIFT },
    { XK_Shift_R,	XP_KS_RSHIFT },
    { XK_Control_L,	XP_KS_LCTRL },
    { XK_Control_R,	XP_KS_RCTRL },
    { XK_Caps_Lock,	XP_KS_CAPSLOCK },

    { XK_space,		XP_KS_SPACE },
    { XK_quotedbl,	XP_KS_QUOTE },
    { XK_comma,		XP_KS_COMMA },
    { XK_plus,		XP_KS_PLUS },
    { XK_minus,		XP_KS_MINUS },
    { XK_period,	XP_KS_PERIOD },
    { XK_slash,		XP_KS_SLASH },

    { XK_0,	XP_KS_0 },
    { XK_1,	XP_KS_1 },
    { XK_2,	XP_KS_2 },
    { XK_3,	XP_KS_3 },
    { XK_4,	XP_KS_4 },
    { XK_5,	XP_KS_5 },
    { XK_6,	XP_KS_6 },
    { XK_7,	XP_KS_7 },
    { XK_8,	XP_KS_8 },
    { XK_9,	XP_KS_9 },

    { XK_semicolon,	XP_KS_SEMICOLON },
    { XK_equal,		XP_KS_EQUALS },

    { XK_a,	XP_KS_a },
    { XK_b,	XP_KS_b },
    { XK_c,	XP_KS_c },
    { XK_d,	XP_KS_d },
    { XK_e,	XP_KS_e },
    { XK_f,	XP_KS_f },
    { XK_g,	XP_KS_g },
    { XK_h,	XP_KS_h },
    { XK_i,	XP_KS_i },
    { XK_j,	XP_KS_j },
    { XK_k,	XP_KS_k },
    { XK_l,	XP_KS_l },
    { XK_m,	XP_KS_m },
    { XK_n,	XP_KS_n },
    { XK_o,	XP_KS_o },
    { XK_p,	XP_KS_p },
    { XK_q,	XP_KS_q },
    { XK_r,	XP_KS_r },
    { XK_s,	XP_KS_s },
    { XK_t,	XP_KS_t },
    { XK_u,	XP_KS_u },
    { XK_v,	XP_KS_v },
    { XK_w,	XP_KS_w },
    { XK_x,	XP_KS_x },
    { XK_y,	XP_KS_y },
    { XK_z,	XP_KS_z },

    { XK_A,	XP_KS_a },
    { XK_B,	XP_KS_b },
    { XK_C,	XP_KS_c },
    { XK_D,	XP_KS_d },
    { XK_E,	XP_KS_e },
    { XK_F,	XP_KS_f },
    { XK_G,	XP_KS_g },
    { XK_H,	XP_KS_h },
    { XK_I,	XP_KS_i },
    { XK_J,	XP_KS_j },
    { XK_K,	XP_KS_k },
    { XK_L,	XP_KS_l },
    { XK_M,	XP_KS_m },
    { XK_N,	XP_KS_n },
    { XK_O,	XP_KS_o },
    { XK_P,	XP_KS_p },
    { XK_Q,	XP_KS_q },
    { XK_R,	XP_KS_r },
    { XK_S,	XP_KS_s },
    { XK_T,	XP_KS_t },
    { XK_U,	XP_KS_u },
    { XK_V,	XP_KS_v },
    { XK_W,	XP_KS_w },
    { XK_X,	XP_KS_x },
    { XK_Y,	XP_KS_y },
    { XK_Z,	XP_KS_z },

    { XK_bracketleft,	XP_KS_LEFTBRACKET },
    { XK_backslash,	XP_KS_BACKSLASH },
    { XK_bracketright,	XP_KS_RIGHTBRACKET },
    { XK_quoteleft,	XP_KS_BACKQUOTE }, /* kps -just a guess */
    
    { XK_KP_Insert,	XP_KS_KP_INSERT },
    { XK_KP_Delete,	XP_KS_KP_DELETE },
    { XK_Print,		XP_KS_PRINT },
    { XK_section,	XP_KS_SECTION },   /* § */
};

static xp_keysym_t KeySym_to_xp_keysym(KeySym xks)
{
    int i;

    for (i = 0; i < NELEM(xks2xpks_table); i++) {
	xp_xks2xpks_t *x = &xks2xpks_table[i];

	if (x->xks == xks)
	    return x->xpks;
    }

    return XP_KS_UNKNOWN;
}

static keys_t Optionhack_lookup_key(KeySym xks, bool reset)
{
    xp_keysym_t xpks;

    xpks = KeySym_to_xp_keysym(xks);

    return Generic_lookup_key(xpks, reset);
}

#endif

keys_t Lookup_key(XEvent *event, KeySym ks, bool reset)
{
    keys_t ret = KEY_DUMMY;

#ifdef OPTIONHACK

    ret = Optionhack_lookup_key(ks, reset);


#else /* ! OPTIONHACK */
    static int i = 0;

    (void)event;

#if 1 /* linear search */
    if (reset)
	i = 0;

    /*
     * Variable 'i' is already initialized.
     * Use brute force linear search to find the key.
     */
    for (; i < num_keydefs; i++) {
	if (ks == keydefs[i].keysym) {
	    ret = keydefs[i].key;
	    i++;
	    break;
	}
    }
#else /* binary search */
    if (reset) {
	/* binary search since keydefs is sorted on keysym. */
	int lo = 0, hi = num_keydefs - 1;
	while (lo < hi) {
	    i = (lo + hi) >> 1;
	    if (ks > keydefs[i].keysym)
		lo = i + 1;
	    else
		hi = i;
	}
	if (lo == hi && ks == keydefs[lo].keysym) {
	    while (lo > 0 && ks == keydefs[lo - 1].keysym)
		lo--;
	    i = lo;
	    ret = keydefs[i].key;
	    i++;
	}
    }
    else {
	if (i < num_keydefs && ks == keydefs[i].keysym) {
	    ret = keydefs[i].key;
	    i++;
	}
    }
#endif
#endif /* OPTIONHACK */

    IFWINDOWS( Trace("Lookup_key: got key ks=%04X ret=%d\n", ks, ret) );

#ifdef DEVELOPMENT
    if (reset && ret == KEY_DUMMY) {
	static XComposeStatus	compose;
	char			str[4];
	int			count;

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
    char* wintalkstr;

    if (pointerControl) {
	initialPointerControl = true;
	Pointer_control_set_state(false);
    }

    wintalkstr = (char*)mfcDoTalkWindow();
    if (*wintalkstr)
	Net_talk(wintalkstr);

    if (initialPointerControl) {
	initialPointerControl = false;
	Pointer_control_set_state(true);
    }

    scoresChanged = true;
}
#endif

bool Key_press_pointer_control(void)
{
    Pointer_control_set_state(!pointerControl);
    return false;	/* server doesn't need to know */
}

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
    KeySym 		ks;
    keys_t		key;
    int			change = false;
    bool		(*key_do)(keys_t);

    switch(event->type) {
    case KeyPress:
	key_do = Key_press;
	break;
    case KeyRelease:
	key_do = Key_release;
	break;
    default:
	return;
    }

    if ((ks = XLookupKeysym(&event->xkey, 0)) == NoSymbol)
	return;

    for (key = Lookup_key(event, ks, true);
	 key != KEY_DUMMY;
	 key = Lookup_key(event, ks, false))
	change |= (*key_do)(key);

    if (change)
	Net_key_change();
}

void Talk_event(XEvent *event)
{
    if (!Talk_do_event(event))
	Talk_set_state(false);
}

/*
 * The option code calls this callback when key options are processed.
 */
bool Key_binding_callback(keys_t key, const char *str)
{
    KeySym ks;
    keydefs_t keydef;
    int i;

    assert(key != KEY_DUMMY);
    assert(str);

    if ((ks = XStringToKeysym(str)) == NoSymbol)
	/* Invalid keysym */
	return false;

    for (i = 0; i < max_keydefs; i++) {
	if (keydefs[i].keysym == ks
	    && keydefs[i].key == key)
	    /* This binding exists already. */
	    return true;
    }

    keydef.keysym = ks;
    keydef.key = key;

    STORE(keydefs_t, keydefs, num_keydefs, max_keydefs, keydef);

    return true;
}


int	talk_key_repeating;
XEvent	talk_key_repeat_event;
struct timeval talk_key_repeat_time;
static struct timeval time_now;

void xevent_keyboard(int queued)
{
    int			i;
#ifndef _WINDOWS
    int			n;
    XEvent		event;
#endif

    if (talk_key_repeating) {
	/* TODO: implement gettimeofday() for windows */
	IFNWINDOWS(gettimeofday(&time_now, NULL));
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

#ifndef _WINDOWS
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
#endif
}

static ipos	delta;
ipos	mousePosition;	/* position of mouse pointer. */
int	mouseMovement;	/* horizontal mouse movement. */


void xevent_pointer(void)
{
#ifndef _WINDOWS
    XEvent		event;
#endif

    if (pointerControl) {
	if (!talk_mapped) {

#ifdef _WINDOWS
	    /* This is a HACK to fix mouse control under windows. */
	    {
		 POINT point;

		 GetCursorPos(&point);
		 mouseMovement = point.x - draw_width/2;
		 XWarpPointer(dpy, None, drawWindow,
			      0, 0, 0, 0,
			      draw_width/2, draw_height/2);
	    }
		/* fix end */
#endif

	    if (mouseMovement != 0) {
		Send_pointer_move(mouseMovement);
		delta.x = draw_width / 2 - mousePosition.x;
		delta.y = draw_height / 2 - mousePosition.y;
		if (ABS(delta.x) > 3 * draw_width / 8
		    || ABS(delta.y) > 1 * draw_height / 8) {

#ifndef _WINDOWS
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
#endif
		    XFlush(dpy);
		}
	    }
	}
    }
}

#ifndef _WINDOWS
int x_event(int new_input)
#else
int win_xevent(XEvent event)
#endif
{
    int			queued = 0;
#ifndef _WINDOWS
    int			i, n;
    XEvent		event;
#endif

#ifdef SOUND
    audioEvents();
#endif /* SOUND */

    mouseMovement = 0;

#ifndef _WINDOWS
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
#endif
	switch (event.type) {

#ifndef _WINDOWS
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
#endif

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
#ifndef _WINDOWS
    }
#endif

    xevent_keyboard(queued);
    xevent_pointer();
    return 0;
}
