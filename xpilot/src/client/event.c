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
