/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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

#include "xpclient.h"

char event_version[] = VERSION;


static BITV_DECL(keyv, NUM_KEYS);



int Key_init(void)
{
    if (sizeof(keyv) != KEYBOARD_SIZE) {
	warn("%s, %d: keyv size %d, KEYBOARD_SIZE is %d",
	     __FILE__, __LINE__,
	     sizeof(keyv), KEYBOARD_SIZE);
	exit(1);
    }
    memset(keyv, 0, sizeof keyv);
    BITV_SET(keyv, KEY_SHIELD);

    return 0;
}

int Key_update(void)
{
    return Send_keyboard(keyv);
}

static bool Key_check_talk_macro(keys_t key)
{
    if (key >= KEY_MSG_1 && key < KEY_MSG_1 + TALK_FAST_NR_OF_MSGS)
	Talk_macro(talk_fast_msgs[key - KEY_MSG_1]);
    return true;
}


static bool Key_press_id_mode(void)
{
    showRealName = showRealName ? false : true;
    scoresChanged++;
    return false;	/* server doesn't need to know */
}

static bool Key_press_autoshield_hack(void)
{
    if (auto_shield && BITV_ISSET(keyv, KEY_SHIELD))
	BITV_CLR(keyv, KEY_SHIELD);
    return false;
}

static bool Key_press_shield(keys_t key)
{
    if (toggle_shield) {
	shields = !shields;
	if (shields)
	    BITV_SET(keyv, key);
	else
	    BITV_CLR(keyv, key);
	return true;
    }
    else if (auto_shield) {
	shields = true;
#if 0
	shields = false;
	BITV_CLR(keyv, key);
	return true;
#endif
    }
    return false;
}

static bool Key_press_fuel(void)
{
    fuelTime = FUEL_NOTIFY_TIME;
    return false;
}

static bool Key_press_swap_settings(void)
{
    double tmp;
#define SWAP(a, b) (tmp = (a), (a) = (b), (b) = tmp)

    SWAP(power, power_s);
    SWAP(turnspeed, turnspeed_s);
    SWAP(turnresistance, turnresistance_s);
    controlTime = CONTROL_TIME;
    Config_redraw();

    return true;
}

static bool Key_press_increase_power(void)
{
    power = power * 1.10;
    power = MIN(power, MAX_PLAYER_POWER);
    Send_power(power);

    Config_redraw();
    controlTime = CONTROL_TIME;
    return false;	/* server doesn't see these keypresses anymore */

}

static bool Key_press_decrease_power(void)
{
    power = power / 1.10;
    power = MAX(power, MIN_PLAYER_POWER);
    Send_power(power);

    Config_redraw();
    controlTime = CONTROL_TIME;
    return false;	/* server doesn't see these keypresses anymore */
}

static bool Key_press_increase_turnspeed(void)
{
    turnspeed = turnspeed * 1.05;
    turnspeed = MIN(turnspeed, MAX_PLAYER_TURNSPEED);
    Send_turnspeed(turnspeed);

    Config_redraw();
    controlTime = CONTROL_TIME;
    return false;	/* server doesn't see these keypresses anymore */
}

static bool Key_press_decrease_turnspeed(void)
{
    turnspeed = turnspeed / 1.05;
    turnspeed = MAX(turnspeed, MIN_PLAYER_TURNSPEED);
    Send_turnspeed(turnspeed);

    Config_redraw();
    controlTime = CONTROL_TIME;
    return false;	/* server doesn't see these keypresses anymore */
}

static bool Key_press_show_items(void)
{
    instruments.showItems = !instruments.showItems;
    return false;	/* server doesn't need to know */
}

static bool Key_press_show_messages(void)
{
    instruments.showMessages = !instruments.showMessages;
    return false;	/* server doesn't need to know */
}

static bool Key_press_msgs_stdout(void)
{
    if (selectionAndHistory)
	Print_messages_to_stdout();
    return false;	/* server doesn't need to know */
}

static bool Key_press_select_lose_item(void)
{
    if (lose_item_active == 1)
        lose_item_active = 2;
    else
	lose_item_active = 1;
    return true;
}


bool Key_press(keys_t key)
{
    Key_check_talk_macro(key);

    switch (key) {
    case KEY_ID_MODE:
	return (Key_press_id_mode());

    case KEY_FIRE_SHOT:
    case KEY_FIRE_LASER:
    case KEY_FIRE_MISSILE:
    case KEY_FIRE_TORPEDO:
    case KEY_FIRE_HEAT:
    case KEY_DROP_MINE:
    case KEY_DETACH_MINE:
	Key_press_autoshield_hack();
	break;

    case KEY_SHIELD:
	if (Key_press_shield(key))
	    return true;
	break;

    case KEY_REFUEL:
    case KEY_REPAIR:
    case KEY_TANK_NEXT:
    case KEY_TANK_PREV:
	Key_press_fuel();
	break;

    case KEY_SWAP_SETTINGS:
	if (!Key_press_swap_settings())
	    return false;
	break;

    case KEY_SWAP_SCALEFACTOR:
	if (!Key_press_swap_scalefactor())
	    return false;
	break;

    case KEY_INCREASE_POWER:
	return Key_press_increase_power();

    case KEY_DECREASE_POWER:
	return Key_press_decrease_power();

    case KEY_INCREASE_TURNSPEED:
	return Key_press_increase_turnspeed();

    case KEY_DECREASE_TURNSPEED:
	return Key_press_decrease_turnspeed();

    case KEY_TALK:
	return Key_press_talk();

    case KEY_TOGGLE_OWNED_ITEMS:
	return Key_press_show_items();

    case KEY_TOGGLE_MESSAGES:
	return Key_press_show_messages();

    case KEY_POINTER_CONTROL:
	return Key_press_pointer_control();

    case KEY_TOGGLE_RECORD:
	return Key_press_toggle_record();

    case KEY_TOGGLE_RADAR_SCORE:
	return Key_press_toggle_radar_score();

    case KEY_PRINT_MSGS_STDOUT:
	return Key_press_msgs_stdout();

    case KEY_SELECT_ITEM:
    case KEY_LOSE_ITEM:
	if (!Key_press_select_lose_item())
	    return false;
    default:
	break;
    }

    if (key < NUM_KEYS)
	BITV_SET(keyv, key);

    return true;
}

bool Key_release(keys_t key)
{
    switch (key) {
    case KEY_ID_MODE:
    case KEY_TALK:
    case KEY_TOGGLE_OWNED_ITEMS:
    case KEY_TOGGLE_MESSAGES:
	return false;	/* server doesn't need to know */

    /* Don auto-shield hack */
    /* restore shields */
    case KEY_FIRE_SHOT:
    case KEY_FIRE_LASER:
    case KEY_FIRE_MISSILE:
    case KEY_FIRE_TORPEDO:
    case KEY_FIRE_HEAT:
    case KEY_DROP_MINE:
    case KEY_DETACH_MINE:
	if (auto_shield && shields && !BITV_ISSET(keyv, KEY_SHIELD)) {
	    /* Here We need to know if any other weapons are still on */
	    /*      before we turn shield back on   */
	    BITV_CLR(keyv, key);
	    if (!BITV_ISSET(keyv, KEY_FIRE_SHOT) &&
		!BITV_ISSET(keyv, KEY_FIRE_LASER) &&
		!BITV_ISSET(keyv, KEY_FIRE_MISSILE) &&
		!BITV_ISSET(keyv, KEY_FIRE_TORPEDO) &&
		!BITV_ISSET(keyv, KEY_FIRE_HEAT) &&
		!BITV_ISSET(keyv, KEY_DROP_MINE) &&
		!BITV_ISSET(keyv, KEY_DETACH_MINE))
		BITV_SET(keyv, KEY_SHIELD);
	}
	break;

    case KEY_SHIELD:
	if (toggle_shield)
	    return false;
	else if (auto_shield) {
	    shields = false;
#if 0
	    shields = true;
	    BITV_SET(keyv, key);
	    return true;
#endif
	}
	break;

    case KEY_REFUEL:
    case KEY_REPAIR:
	fuelTime = FUEL_NOTIFY_TIME;
	break;

    case KEY_SELECT_ITEM:
    case KEY_LOSE_ITEM:
	if (lose_item_active == 2)
	    lose_item_active = 1;
	else
	    lose_item_active = -clientFPS;
        break;

    default:
	break;
    }
    if (key < NUM_KEYS)
	BITV_CLR(keyv, key);

    return true;
}

void Reset_shields(void)
{
    if (toggle_shield || auto_shield) {
	BITV_SET(keyv, KEY_SHIELD);
	shields = true;
	if (auto_shield) {
	    if (BITV_ISSET(keyv, KEY_FIRE_SHOT) ||
		BITV_ISSET(keyv, KEY_FIRE_LASER) ||
		BITV_ISSET(keyv, KEY_FIRE_MISSILE) ||
		BITV_ISSET(keyv, KEY_FIRE_TORPEDO) ||
		BITV_ISSET(keyv, KEY_FIRE_HEAT) ||
		BITV_ISSET(keyv, KEY_DROP_MINE) ||
		BITV_ISSET(keyv, KEY_DETACH_MINE))
		BITV_CLR(keyv, KEY_SHIELD);
	}
	Net_key_change();
    }
}

void Set_auto_shield(bool on)
{
    auto_shield = on;
}

void Set_toggle_shield(bool on)
{
    toggle_shield = on;
    if (toggle_shield) {
	if (auto_shield)
	    shields = true;
	else
	    shields = (BITV_ISSET(keyv, KEY_SHIELD)) ? true : false;
    }
}

#ifdef OPTIONHACK
/*
 * Standard key options.
 */
xp_option_t key_options[] = {

    XP_KEY_OPTION(
	"keyTurnLeft",
	"a",
	KEY_TURN_LEFT,
	NULL,
	"Turn left (anti-clockwise).\n"),

    XP_KEY_OPTION(
	"keyTurnRight",
	"s",
	KEY_TURN_RIGHT,
	NULL,
	"Turn right (clockwise).\n"),

    XP_KEY_OPTION(
	"keyThrust",
	"Shift_R Shift_L",
	KEY_THRUST,
	NULL,
	"Thrust.\n"),

    XP_KEY_OPTION(
	"keyShield",
	"space Caps_Lock",
	KEY_SHIELD,
	NULL,
	"Raise or toggle the shield.\n"),

    XP_KEY_OPTION(
	"keyFireShot",
	"Return Linefeed",
	KEY_FIRE_SHOT,
	NULL,
	"Fire shot.\n"
	"Note that shields must be down to fire.\n"),

    XP_KEY_OPTION(
	"keyFireMissile",
	"backslash",
	KEY_FIRE_MISSILE,
	NULL,
	"Fire smart missile.\n"),

    XP_KEY_OPTION(
	"keyFireTorpedo",
	"quoteright",
	KEY_FIRE_TORPEDO,
	NULL,
	"Fire unguided torpedo.\n"),

    XP_KEY_OPTION(
	"keyFireHeat",
	"semicolon",
	KEY_FIRE_HEAT,
	NULL,
	"Fire heatseeking missile.\n"),

    XP_KEY_OPTION(
	"keyFireLaser",
	"slash",
	KEY_FIRE_LASER,
	NULL,
	"Activate laser beam.\n"),

    XP_KEY_OPTION(
	"keyDropMine",
	"Tab",
	KEY_DROP_MINE,
	NULL,
	"Drop a stationary mine.\n"),

    XP_KEY_OPTION(
	"keyDetachMine",
	"bracketright",
	KEY_DETACH_MINE,
	NULL,
	"Detach a moving mine.\n"),

    XP_KEY_OPTION(
	"keyDetonateMines",
	"equal",
	KEY_DETONATE_MINES,
	NULL,
	"Detonate the closest mine you have dropped or thrown.\n"),

    XP_KEY_OPTION(
	"keyLockClose",
	"Select Up",
	KEY_LOCK_CLOSE,
	NULL,
	"Lock on closest player.\n"),

    XP_KEY_OPTION(
	"keyLockNextClose",
	"Down",
	KEY_LOCK_NEXT_CLOSE,
	NULL,
	"Lock on next closest player.\n"),

    XP_KEY_OPTION(
	"keyLockNext",
	"Next Right",
	KEY_LOCK_NEXT,
	NULL,
	"Lock on next player.\n"),

    XP_KEY_OPTION(
	"keyLockPrev",
	"Prior Left",
	KEY_LOCK_PREV,
	NULL,
	"Lock on previous player.\n"),

    XP_KEY_OPTION(
	"keyRefuel",
	"f Control_L Control_R",
	KEY_REFUEL,
	NULL,
	"Refuel.\n"),

    XP_KEY_OPTION(
	"keyRepair",
	"f",
	KEY_REPAIR,
	NULL,
	"Repair target.\n"),

    XP_KEY_OPTION(
	"keyCloak",
	"Delete BackSpace",
	KEY_CLOAK,
	NULL,
	"Toggle cloakdevice.\n"),

    XP_KEY_OPTION(
	"keyEcm",
	"bracketleft",
	KEY_ECM,
	NULL,
	"Use ECM.\n"),

    XP_KEY_OPTION(
	"keySelfDestruct",
	"End",
	KEY_SELF_DESTRUCT,
	NULL,
	"Toggle self destruct.\n"),

    XP_KEY_OPTION(
	"keyIdMode",
	"u",
	KEY_ID_MODE,
	NULL,
	"Toggle User mode (show real names).\n"),

    XP_KEY_OPTION(
	"keyPause",
	"Pause",
	KEY_PAUSE,
	NULL,
	"Toggle pause mode.\n"
	"When the ship is stationary on its homebase.\n"),

    XP_KEY_OPTION(
	"keySwapSettings",
	"Escape",
	KEY_SWAP_SETTINGS,
	NULL,
	"Swap control settings.\n"
	"These are the power, turn speed and turn resistance settings.\n"),

    XP_KEY_OPTION(
        "keySwapScaleFactor",
        "",
        KEY_SWAP_SCALEFACTOR,
        NULL,
        "Swap scalefactor settings.\n"),

    XP_KEY_OPTION(
	"keyChangeHome",
	"Home h",
	KEY_CHANGE_HOME,
	NULL,
	"Change home base.\n"
	"When the ship is stationary on a new homebase.\n"),

    XP_KEY_OPTION(
	"keyConnector",
	"Control_L",
	KEY_CONNECTOR,
	NULL,
	"Connect to a ball.\n"),

    XP_KEY_OPTION(
	"keyDropBall",
	"d",
	KEY_DROP_BALL,
	NULL,
	"Drop a ball.\n"),

    XP_KEY_OPTION(
	"keyTankNext",
	"e",
	KEY_TANK_NEXT,
	NULL,
	"Use the next tank.\n"),

    XP_KEY_OPTION(
	"keyTankPrev",
	"w",
	KEY_TANK_PREV,
	NULL,
	"Use the the previous tank.\n"),

    XP_KEY_OPTION(
	"keyTankDetach",
	"r",
	KEY_TANK_DETACH,
	NULL,
	"Detach the current tank.\n"),

    XP_KEY_OPTION(
	"keyIncreasePower",
	"KP_Multiply",
	KEY_INCREASE_POWER,
	NULL,
	"Increase engine power.\n"),

    XP_KEY_OPTION(
	"keyDecreasePower",
	"KP_Divide",
	KEY_DECREASE_POWER,
	NULL,
	"Decrease engine power.\n"),

    XP_KEY_OPTION(
	"keyIncreaseTurnspeed",
	"KP_Add",
	KEY_INCREASE_TURNSPEED,
	NULL,
	"Increase turnspeed.\n"),

    XP_KEY_OPTION(
	"keyDecreaseTurnspeed",
	"KP_Subtract",
	KEY_DECREASE_TURNSPEED,
	NULL,
	"Decrease turnspeed.\n"),

    XP_KEY_OPTION(
	"keyTransporter",
	"t",
	KEY_TRANSPORTER,
	NULL,
	"Use transporter to steal an item.\n"),

    XP_KEY_OPTION(
	"keyDeflector",
	"o",
	KEY_DEFLECTOR,
	NULL,
	"Toggle deflector.\n"),

    XP_KEY_OPTION(
	"keyHyperJump",
	"q",
	KEY_HYPERJUMP,
	NULL,
	"Teleport.\n"),

    XP_KEY_OPTION(
	"keyPhasing",
	"p",
	KEY_PHASING,
	NULL,
	"Use phasing device.\n"),

    XP_KEY_OPTION(
	"keyTalk",
	"m",
	KEY_TALK,
	NULL,
	"Toggle talk window.\n"),

    XP_KEY_OPTION(
	"keyToggleNuclear",
	"n",
	KEY_TOGGLE_NUCLEAR,
	NULL,
	"Toggle nuclear weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleCluster",
	"c",
	KEY_TOGGLE_CLUSTER,
	NULL,
	"Toggle cluster weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleImplosion",
	"i",
	KEY_TOGGLE_IMPLOSION,
	NULL,
	"Toggle implosion weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleVelocity",
	"v",
	KEY_TOGGLE_VELOCITY,
	NULL,
	"Toggle explosion velocity weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleMini",
	"x",
	KEY_TOGGLE_MINI,
	NULL,
	"Toggle mini weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleSpread",
	"z",
	KEY_TOGGLE_SPREAD,
	NULL,
	"Toggle spread weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyTogglePower",
	"b",
	KEY_TOGGLE_POWER,
	NULL,
	"Toggle power weapon modifier.\n"),

    XP_KEY_OPTION(
	"keyToggleCompass",
	"KP_7",
	KEY_TOGGLE_COMPASS,
	NULL,
	"Toggle HUD/radar compass lock.\n"),

    XP_KEY_OPTION(
	"keyToggleAutoPilot",
	"h",
	KEY_TOGGLE_AUTOPILOT,
	NULL,
	"Toggle automatic pilot mode.\n"),

    XP_KEY_OPTION(
	"keyToggleLaser",
	"l",
	KEY_TOGGLE_LASER,
	NULL,
	"Toggle laser modifier.\n"),

    XP_KEY_OPTION(
	"keyEmergencyThrust",
	"j",
	KEY_EMERGENCY_THRUST,
	NULL,
	"Pull emergency thrust handle.\n"),

    XP_KEY_OPTION(
	"keyEmergencyShield",
	"g",
	KEY_EMERGENCY_SHIELD,
	NULL,
	"Toggle emergency shield power.\n"),

    XP_KEY_OPTION(
	"keyTractorBeam",
	"comma",
	KEY_TRACTOR_BEAM,
	NULL,
	"Use tractor beam in attract mode.\n"),

    XP_KEY_OPTION(
	"keyPressorBeam",
	"period",
	KEY_PRESSOR_BEAM,
	NULL,
	"Use tractor beam in repulse mode.\n"),

    XP_KEY_OPTION(
	"keyClearModifiers",
	"k",
	KEY_CLEAR_MODIFIERS,
	NULL,
	"Clear current weapon modifiers.\n"),

    XP_KEY_OPTION(
	"keyLoadModifiers1",
	"1",
	KEY_LOAD_MODIFIERS_1,
	NULL,
	"Load the weapon modifiers from bank 1.\n"),

    XP_KEY_OPTION(
	"keyLoadModifiers2",
	"2",
	KEY_LOAD_MODIFIERS_2,
	NULL,
	"Load the weapon modifiers from bank 2.\n"),

    XP_KEY_OPTION(
	"keyLoadModifiers3",
	"3",
	KEY_LOAD_MODIFIERS_3,
	NULL,
	"Load the weapon modifiers from bank 3.\n"),

    XP_KEY_OPTION(
	"keyLoadModifiers4",
	"4",
	KEY_LOAD_MODIFIERS_4,
	NULL,
	"Load the weapon modifiers from bank 4.\n"),

    XP_KEY_OPTION(
	"keyToggleOwnedItems",
	"KP_8",
	KEY_TOGGLE_OWNED_ITEMS,
	NULL,
	"Toggle list of owned items on HUD.\n"),

    XP_KEY_OPTION(
	"keyToggleMessages",
	"KP_9",
	KEY_TOGGLE_MESSAGES,
	NULL,
	"Toggle showing of messages.\n"),

    XP_KEY_OPTION(
	"keyReprogram",
	"quoteleft",
	KEY_REPROGRAM,
	NULL,
	"Reprogram modifier or lock bank.\n"),

    XP_KEY_OPTION(
	"keyLoadLock1",
	"5",
	KEY_LOAD_LOCK_1,
	NULL,
	"Load player lock from bank 1.\n"),

    XP_KEY_OPTION(
	"keyLoadLock2",
	"6",
	KEY_LOAD_LOCK_2,
	NULL,
	"Load player lock from bank 2.\n"),

    XP_KEY_OPTION(
	"keyLoadLock3",
	"7",
	KEY_LOAD_LOCK_3,
	NULL,
	"Load player lock from bank 3.\n"),

    XP_KEY_OPTION(
	"keyLoadLock4",
	"8",
	KEY_LOAD_LOCK_4,
	NULL,
	"Load player lock from bank 4.\n"),

    XP_KEY_OPTION(
	"keyToggleRecord",
	"KP_5",
	KEY_TOGGLE_RECORD,
	NULL,
	"Toggle recording of session (see recordFile).\n"),

    XP_KEY_OPTION(
        "keyToggleRadarScore",
        "",
        KEY_TOGGLE_RADAR_SCORE,
        NULL,
        "Toggles the radar and score windows on and off.\n"),

    XP_KEY_OPTION(
	"keySelectItem",
	"KP_0 KP_Insert",
	KEY_SELECT_ITEM,
	NULL,
	"Select an item to lose.\n"),

    XP_KEY_OPTION(
	"keyLoseItem",
	"KP_Delete KP_Decimal",
	KEY_LOSE_ITEM,
	NULL,
	"Lose the selected item.\n"),

    XP_KEY_OPTION(
	"keyPrintMessagesStdout",
	"Print",
	KEY_PRINT_MSGS_STDOUT,
	NULL,
	"Print the current messages to stdout.\n"),

    XP_KEY_OPTION(
	"keyTalkCursorLeft",
	"Left",
	KEY_TALK_CURSOR_LEFT,
	NULL,
	"Move Cursor to the left in the talk window.\n"),

    XP_KEY_OPTION(
	"keyTalkCursorRight",
	"Right",
	KEY_TALK_CURSOR_RIGHT,
	NULL,
	"Move Cursor to the right in the talk window.\n"),

    XP_KEY_OPTION(
	"keyTalkCursorUp",
	"Up",
	KEY_TALK_CURSOR_UP,
	NULL,
	"Browsing in the history of the talk window.\n"),

    XP_KEY_OPTION(
	"keyTalkCursorDown",
	"Down",
	KEY_TALK_CURSOR_DOWN,
	NULL,
	"Browsing in the history of the talk window.\n"),

    XP_KEY_OPTION(
	"keyPointerControl",
	"KP_Enter",
	KEY_POINTER_CONTROL,
	NULL,
	"Toggle pointer control.\n"),

    /* talk macro keys */

    XP_KEY_OPTION(
	"keySendMsg1",
	"F1",
	KEY_MSG_1,
	NULL,
	"Sends the talkmessage stored in msg1.\n"),

    XP_KEY_OPTION(
	"keySendMsg2",
	"F2",
	KEY_MSG_2,
	NULL,
	"Sends the talkmessage stored in msg2.\n"),

    XP_KEY_OPTION(
	"keySendMsg3",
	"F3",
	KEY_MSG_3,
	NULL,
	"Sends the talkmessage stored in msg3.\n"),

    XP_KEY_OPTION(
	"keySendMsg4",
	"F4",
	KEY_MSG_4,
	NULL,
	"Sends the talkmessage stored in msg4.\n"),

    XP_KEY_OPTION(
	"keySendMsg5",
	"F5",
	KEY_MSG_5,
	NULL,
	"Sends the talkmessage stored in msg5.\n"),

    XP_KEY_OPTION(
	"keySendMsg6",
	"F6",
	KEY_MSG_6,
	NULL,
	"Sends the talkmessage stored in msg6.\n"),

    XP_KEY_OPTION(
	"keySendMsg7",
	"F7",
	KEY_MSG_7,
	NULL,
	"Sends the talkmessage stored in msg7.\n"),

    XP_KEY_OPTION(
	"keySendMsg8",
	"F8",
	KEY_MSG_8,
	NULL,
	"Sends the talkmessage stored in msg8.\n"),

    XP_KEY_OPTION(
	"keySendMsg9",
	"F9",
	KEY_MSG_9,
	NULL,
	"Sends the talkmessage stored in msg9.\n"),

    XP_KEY_OPTION(
	"keySendMsg10",
	"F10",
	KEY_MSG_10,
	NULL,
	"Sends the talkmessage stored in msg10.\n"),

    XP_KEY_OPTION(
	"keySendMsg11",
	"F11",
	KEY_MSG_11,
	NULL,
	"Sends the talkmessage stored in msg11.\n"),

    XP_KEY_OPTION(
	"keySendMsg12",
	"F12",
	KEY_MSG_12,
	NULL,
	"Sends the talkmessage stored in msg12.\n"),

    XP_KEY_OPTION(
	"keySendMsg13",
	"",
	KEY_MSG_13,
	NULL,
	"Sends the talkmessage stored in msg13.\n"),

    XP_KEY_OPTION(
	"keySendMsg14",
	"",
	KEY_MSG_14,
	NULL,
	"Sends the talkmessage stored in msg14.\n"),

    XP_KEY_OPTION(
	"keySendMsg15",
	"",
	KEY_MSG_15,
	NULL,
	"Sends the talkmessage stored in msg15.\n"),

    XP_KEY_OPTION(
	"keySendMsg16",
	"",
	KEY_MSG_16,
	NULL,
	"Sends the talkmessage stored in msg16.\n"),

    XP_KEY_OPTION(
	"keySendMsg17",
	"",
	KEY_MSG_17,
	NULL,
	"Sends the talkmessage stored in msg17.\n"),

    XP_KEY_OPTION(
	"keySendMsg18",
	"",
	KEY_MSG_18,
	NULL,
	"Sends the talkmessage stored in msg18.\n"),

    XP_KEY_OPTION(
	"keySendMsg19",
	"",
	KEY_MSG_19,
	NULL,
	"Sends the talkmessage stored in msg19.\n"),

    XP_KEY_OPTION(
	"keySendMsg20",
	"",
	KEY_MSG_20,
	NULL,
	"Sends the talkmessage stored in msg20.\n"),

};

void Store_key_options(void)
{
    STORE_OPTIONS(key_options);
}
#endif
