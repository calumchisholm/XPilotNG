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

/*
 * Configure.c: real-time option control.
 */

#include "xpclient_x11.h"

char configure_version[] = VERSION;

/* int */
static int Config_create_sparkSize(int widget_desc, int *height);
/* bool */
static int Config_create_showNastyShots(int widget_desc, int *height);
/* color */
static int Config_create_hudHLineColor(int widget_desc, int *height);
/* float */
static int Config_create_hudRadarScale(int widget_desc, int *height);



static int Config_create_save(int widget_desc, int *height);
static int Config_update_bool(int widget_desc, void *data, bool *val);
static int Config_update_instruments(int widget_desc, void *data, bool *val);
static int Config_close(int widget_desc, void *data, const char **strptr);
static int Config_next(int widget_desc, void *data, const char **strptr);
static int Config_prev(int widget_desc, void *data, const char **strptr);
static int Config_save(int widget_desc, void *data, const char **strptr);
static int Config_save_confirm_callback(int widget_desc, void *popup_desc,
					const char **strptr);

typedef struct xpilotrc {
    char	*line;
    size_t	size;
} xpilotrc_t;

static xpilotrc_t	*xpilotrc_ptr;
static int		num_xpilotrc, max_xpilotrc;

static bool		config_created = false,
			config_mapped = false;
static int		config_page,
			config_x,
			config_y,
			config_width,
			config_height,
			config_space,
			config_max,
			config_button_space,
			config_text_space,
			config_text_height,
			config_button_height,
			config_entry_height,
			config_bool_width,
			config_bool_height,
			config_int_width,
			config_float_width,
			config_arrow_width,
			config_arrow_height;
static int		*config_widget_desc,
			config_save_confirm_desc = NO_WIDGET;
static int	(*config_creator_default[])(int widget_desc, int *height) = {
    Config_create_sparkSize,
    Config_create_showNastyShots,
    Config_create_hudRadarScale,

    Config_create_save			/* must be last */
};

static int	(*config_creator_colors[])(int widget_desc, int *height) = {

    Config_create_hudHLineColor,

    Config_create_save			/* must be last */
};

static int (**config_creator)(int widget_desc, int *height);

/* this must be updated if new config menu items are added */
#define CONFIG_MAX_WIDGET_IDS \
(MAX(NELEM(config_creator_default), \
     NELEM(config_creator_colors)))

static int config_widget_ids[CONFIG_MAX_WIDGET_IDS];

/* this must be updated if new config menu items are added */
static int Nelem_config_creator(void)
{
    if (config_creator == config_creator_colors)
	return NELEM(config_creator_colors);
    if (config_creator == config_creator_default)
	return NELEM(config_creator_default);
    return 0;
}
 

static void Create_config(void)
{
    int			i,
			num,
			height,
			offset,
			width,
			widget_desc;
    bool		full;

    /*
     * Window dimensions relative to the top window.
     */
    config_x = 0;
    config_y = RadarHeight + ButtonHeight + 2;
    config_width = 256;
    config_height = top_height - config_y;

    /*
     * Space between label-text and label-border.
     */
    config_text_space = 3;
    /*
     * Height of a label window.
     */
    config_text_height = 2 * 1 + textFont->ascent + textFont->descent;

    /*
     * Space between button-text and button-border.
     */
    config_button_space = 3;
    /*
     * Height of a button window.
     */
    config_button_height = buttonFont->ascent + buttonFont->descent + 2 * 1;

    config_entry_height = MAX(config_text_height, config_button_height);

    /*
     * Space between entries and between an entry and the border.
     */
    config_space = 6;

    /*
     * Sizes of the different widget types.
     */
    config_bool_width = XTextWidth(buttonFont, "Yes", 3)
			+ 2 * config_button_space;
    config_bool_height = config_button_height;
    config_arrow_height = config_text_height;
    config_arrow_width = config_text_height;
    config_int_width = 4 + XTextWidth(buttonFont, "1000", 4);
    config_float_width = 4 + XTextWidth(buttonFont, "0.22", 4);

    config_max = Nelem_config_creator();
    config_widget_desc = malloc(config_max * sizeof(int));
    if (config_widget_desc == NULL) {
	error("No memory for config");
	return;
    }

    num = -1;
    full = true;
    for (i = 0; i < Nelem_config_creator(); i++) {
	if (full == true) {
	    full = false;
	    num++;
	    config_widget_desc[num]
		= Widget_create_form(NO_WIDGET, topWindow,
				     config_x, config_y,
				     config_width, config_height,
				     0);
	    if (config_widget_desc[num] == 0)
		break;

	    height = config_height - config_space - config_button_height;
	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "PREV", 4);
	    offset = (config_width - width) / 2;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "PREV", Config_prev,
				       (void *)(long)num);
	    if (widget_desc == 0)
		break;

	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "NEXT", 4);
	    offset = config_width - width - config_space;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "NEXT", Config_next,
				       (void *)(long)num);
	    if (widget_desc == 0)
		break;

	    width = 2 * config_button_space + XTextWidth(buttonFont,
							  "CLOSE", 5);
	    offset = config_space;
	    widget_desc =
		Widget_create_activate(config_widget_desc[num],
				       offset, height,
				       width, config_button_height,
				       0, "CLOSE", Config_close,
				       (void *)(long)num);
	    if (widget_desc == 0)
		break;

	    height = config_space;
	}
	if ((config_widget_ids[i] =
	     (*config_creator[i])(config_widget_desc[num], &height)) == 0) {
	    i--;
	    full = true;
	    if (height == config_space)
		break;
	    continue;
	}
    }
    if (i < Nelem_config_creator()) {
	for (; num >= 0; num--) {
	    if (config_widget_desc[num] != 0)
		Widget_destroy(config_widget_desc[num]);
	}
	config_created = false;
	config_mapped = false;
    } else {
	config_max = num + 1;
	config_widget_desc = realloc(config_widget_desc,
				     config_max * sizeof(int));
	config_page = 0;
	for (i = 0; i < config_max; i++)
	    Widget_map_sub(config_widget_desc[i]);
	config_created = true;
	config_mapped = false;
    }
}

static int Config_close(int widget_desc, void *data, const char **strptr)
{
    (void)widget_desc; (void)data; (void)strptr;
    Widget_unmap(config_widget_desc[config_page]);
    config_mapped = false;
    return 0;
}

static int Config_next(int widget_desc, void *data, const char **strptr)
{
    int			prev_page = config_page;

    (void)widget_desc; (void)data; (void)strptr;
    if (config_max > 1) {
	config_page = (config_page + 1) % config_max;
	Widget_raise(config_widget_desc[config_page]);
	Widget_unmap(config_widget_desc[prev_page]);
	config_mapped = true;
    }
    return 0;
}

static int Config_prev(int widget_desc, void *data, const char **strptr)
{
    int			prev_page = config_page;

    (void)widget_desc; (void)data; (void)strptr;
    if (config_max > 1) {
	config_page = (config_page - 1 + config_max) % config_max;
	Widget_raise(config_widget_desc[config_page]);
	Widget_unmap(config_widget_desc[prev_page]);
	config_mapped = true;
    }
    return 0;
}

static int Config_create_bool(int widget_desc, int *height,
			      const char *str, bool val,
			      int (*callback)(int, void *, bool *),
			      void *data)
{
    int			offset,
			label_width,
			boolw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height)
	return 0;
    label_width = XTextWidth(textFont, str, (int)strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + config_bool_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space >= config_height)
	    return 0;
    }

    Widget_create_label(widget_desc, config_space, *height
			    + (config_entry_height - config_text_height) / 2,
			label_width, config_text_height, true,
			0, str);
    if (config_space + label_width > offset)
	*height += config_entry_height;
    boolw = Widget_create_bool(widget_desc,
		       offset, *height
			   + (config_entry_height - config_bool_height) / 2,
		       config_bool_width,
		       config_bool_height,
		       0, val, callback, data);
    *height += config_entry_height + config_space;

    return boolw;
}

static int Config_create_int(int widget_desc, int *height,
			     const char *str, int *val, int min, int max,
			     int (*callback)(int, void *, int *), void *data)
{
    int			offset,
			label_width,
			intw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height)
	return 0;
    label_width = XTextWidth(textFont, str, (int)strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + 2 * config_arrow_width
	    + config_int_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space >= config_height)
	    return 0;
    }
    Widget_create_label(widget_desc, config_space, *height
			+ (config_entry_height - config_text_height) / 2,
			label_width, config_text_height, true,
			0, str);
    if (config_space + label_width > offset)
	*height += config_entry_height;
    intw = Widget_create_int(widget_desc, offset, *height
			      + (config_entry_height - config_text_height) / 2,
			     config_int_width, config_text_height,
			     0, val, min, max, callback, data);
    offset += config_int_width;
    Widget_create_arrow_left(widget_desc, offset, *height
			     + (config_entry_height - config_arrow_height) / 2,
			     config_arrow_width, config_arrow_height,
			     0, intw);
    offset += config_arrow_width;
    Widget_create_arrow_right(widget_desc, offset, *height
			      + (config_entry_height-config_arrow_height) / 2,
			      config_arrow_width, config_arrow_height,
			      0, intw);
    *height += config_entry_height + config_space;

    return intw;
}

static int Config_create_color(int widget_desc, int *height, int color,
			       const char *str, int *val, int min, int max,
			       int (*callback)(int, void *, int *), void *data)
{
    int			offset,	label_width, colw;
 
    if (*height + 2*config_entry_height + 2*config_space >= config_height)
 	return 0;
    label_width = XTextWidth(textFont, str, (int)strlen(str))
	+ 2 * config_text_space;
    offset = config_width - (config_space + 2 * config_arrow_width
			     + config_int_width);
    if (config_space + label_width > offset) {
 	if (*height + 3*config_entry_height + 2*config_space >= config_height)
 	    return 0;
    }
    Widget_create_label(widget_desc, config_space, *height
 			+ (config_entry_height - config_text_height) / 2,
 			label_width, config_text_height, true,
 			0, str);
    if (config_space + label_width > offset)
 	*height += config_entry_height;
    colw = Widget_create_color(widget_desc, color, offset, *height
			       + (config_entry_height - config_text_height)/2,
			       config_int_width, config_text_height,
			       0, val, min, max, callback, data);
    offset += config_int_width;
    Widget_create_arrow_left(widget_desc, offset, *height
			     + (config_entry_height - config_arrow_height)/2,
 			     config_arrow_width, config_arrow_height,
 			     0, colw);
    offset += config_arrow_width;
    Widget_create_arrow_right(widget_desc, offset, *height
			      + (config_entry_height - config_arrow_height)/2,
 			      config_arrow_width, config_arrow_height,
 			      0, colw);
    *height += config_entry_height + config_space;

    return colw;
}
 


static int Config_create_float(int widget_desc, int *height,
			       const char *str, double *val,
			       double min, double max,
			       int (*callback)(int, void *, double *),
			       void *data)
{
    int			offset,
			label_width,
			floatw;

    if (*height + 2*config_entry_height + 2*config_space >= config_height)
	return 0;
    label_width = XTextWidth(textFont, str, (int)strlen(str))
		  + 2 * config_text_space;
    offset = config_width - (config_space + 2 * config_arrow_width
	    + config_float_width);
    if (config_space + label_width > offset) {
	if (*height + 3*config_entry_height + 2*config_space >= config_height)
	    return 0;
    }
    Widget_create_label(widget_desc, config_space, *height
			+ (config_entry_height - config_text_height) / 2,
			label_width, config_text_height, true,
			0, str);
    if (config_space + label_width > offset)
	*height += config_entry_height;
    floatw = Widget_create_float(widget_desc, offset, *height
				 + (config_entry_height
				 - config_text_height) / 2,
				 config_float_width, config_text_height,
				 0, val, min, max, callback, data);
    offset += config_float_width;
    Widget_create_arrow_left(widget_desc, offset, *height
			     + (config_entry_height - config_arrow_height) / 2,
			     config_arrow_width, config_arrow_height,
			     0, floatw);
    offset += config_arrow_width;
    Widget_create_arrow_right(widget_desc, offset, *height
			      + (config_entry_height-config_arrow_height) / 2,
			      config_arrow_width, config_arrow_height,
			      0, floatw);
    *height += config_entry_height + config_space;

    return floatw;
}


#define CONFIG_CREATE_COLOR(c) \
Config_create_color(widget_desc, height, c, #c, &c, 0, maxColors-1, NULL, NULL)



static int Config_create_sparkSize(int widget_desc, int *height)
{
    return Config_create_int(widget_desc, height,
			     "sparkSize", &spark_size,
			     MIN_SPARK_SIZE, MAX_SPARK_SIZE,
			     NULL, NULL);
}




static int Config_create_showNastyShots(int widget_desc, int *height)
{
    return Config_create_bool(widget_desc, height, "nastyShots",
                              (showNastyShots) ? true : false,
                              Config_update_bool, &showNastyShots);
}


static int Config_create_hudHLineColor(int widget_desc, int *height)
{
    return CONFIG_CREATE_COLOR(hudHLineColor);
}


static int Config_create_hudRadarScale(int widget_desc, int *height)
{
    return Config_create_float(widget_desc, height,
			       "hudRadarScale", &hudRadarScale, 0.5, 4.0,
			       NULL, NULL);
}








static int Config_create_save(int widget_desc, int *height)
{
    static char		save_str[] = "Save Configuration";
    int			space,
			button_desc,
			width = 2 * config_button_space
				+ XTextWidth(buttonFont, save_str,
					     (int)strlen(save_str));

    space = config_height - (*height + 2*config_entry_height + 2*config_space);
    if (space < 0)
	return 0;
    button_desc =
	Widget_create_activate(widget_desc,
			       (config_width - width) / 2,
			       *height + space / 2,
			       width, config_button_height,
			       0, save_str,
			       Config_save, (void *)save_str);
    if (button_desc == NO_WIDGET)
	return 0;
    *height += config_entry_height + config_space + space;

    return 1;
}

/* General purpose update callback for booleans.
 * Requires that a pointer to the boolean value has been given as
 * client_data argument, and updates this value to the real value.
 */
static int Config_update_bool(int widget_desc, void *data, bool *val)
{
    bool *client_data = (bool *) data;

    (void)widget_desc;
    *client_data = *val;
    return 0;
}

static int Config_update_instruments(int widget_desc, void *data, bool *val)
{
    bool *client_data = (bool *) data;

    (void)widget_desc;
    *client_data = *val;

    if (packetDropMeterColor || packetLossMeterColor) {
	packetMeasurement = true;
	Net_init_measurement();
	if (!packetMeasurement)
	    packetDropMeterColor = 
		packetLossMeterColor = 0;
    }
    if (packetLagMeterColor)
	Net_init_lag_measurement();

    return 0;
}

static void Config_save_failed(const char *reason, const char **strptr)
{
    if (config_save_confirm_desc != NO_WIDGET)
	Widget_destroy(config_save_confirm_desc);

    config_save_confirm_desc
	= Widget_create_confirm(reason, Config_save_confirm_callback);

    if (config_save_confirm_desc != NO_WIDGET)
	Widget_raise(config_save_confirm_desc);

    *strptr = "Saving failed...";
}

#ifndef _WINDOWS
static int Xpilotrc_add(char *line)
{
    int			size;
    char		*str;

    if (strncmp(line, "XPilot", 6) != 0 && strncmp(line, "xpilot", 6) != 0)
	return 0;
    if (line[6] != '.' && line[6] != '*')
	return 0;
    if ((str = strchr(line + 7, ':')) == NULL)
	return 0;

    size = str - (line + 7);
    if (max_xpilotrc <= 0 || xpilotrc_ptr == NULL) {
	num_xpilotrc = 0;
	max_xpilotrc = 75;
	if ((xpilotrc_ptr = (xpilotrc_t *)
		malloc(max_xpilotrc * sizeof(xpilotrc_t))) == NULL) {
	    max_xpilotrc = 0;
	    return -1;
	}
    }
    if (num_xpilotrc >= max_xpilotrc) {
	max_xpilotrc *= 2;
	if ((xpilotrc_ptr = (xpilotrc_t *) realloc(xpilotrc_ptr,
		max_xpilotrc * sizeof(xpilotrc_t))) == NULL) {
	    max_xpilotrc = 0;
	    return -1;
	}
    }
    if ((str = xp_strdup(line)) == NULL)
	return -1;

    xpilotrc_ptr[num_xpilotrc].line = str;
    xpilotrc_ptr[num_xpilotrc].size = size;
    num_xpilotrc++;
    return 0;
}

static void Xpilotrc_end(FILE *fp)
{
    int			i;

    if (max_xpilotrc <= 0 || xpilotrc_ptr == NULL)
	return;

    for (i = 0; i < num_xpilotrc; i++) {
	fprintf(fp, "%s", xpilotrc_ptr[i].line);
	free(xpilotrc_ptr[i].line);
    }
    free(xpilotrc_ptr);
    xpilotrc_ptr = NULL;
    max_xpilotrc = 0;
    num_xpilotrc = 0;
}

static void Xpilotrc_use(char *line)
{
    int			i;

    for (i = 0; i < num_xpilotrc; i++) {
	if (strncmp(xpilotrc_ptr[i].line + 7, line + 7,
		    xpilotrc_ptr[i].size + 1) == 0) {
	    free(xpilotrc_ptr[i].line);
	    xpilotrc_ptr[i--] = xpilotrc_ptr[--num_xpilotrc];
	}
    }
}
#endif


#ifndef _WINDOWS
static void Config_save_resource(FILE *fp, const char *resource, char *value)
{
    char		buf[256];
    int			len, numtabs, i;

    sprintf(buf, "xpilot.%s:", resource);
    len = (int) strlen(buf);
#define TABSIZE 8
    /* assume tabs are max size of TABSIZE */
    numtabs = ((5 * TABSIZE - 1) - len) / TABSIZE;
    for (i = 0; i < numtabs; i++)
	strcat(buf, "\t");
    sprintf(buf + strlen(buf), "%s\n", value);
#if 0
    sprintf(buf, "xpilot.%s:\t\t%s\n", resource, value);
#endif
    Xpilotrc_use(buf);
    fprintf(fp, "%s", buf);
}
#endif


static void Config_save_comment(FILE *fp, const char *comment)
{
    IFNWINDOWS(fprintf(fp, "%s", comment));
}

static void Config_save_float(FILE *fp, const char *resource, double value)
{
    char		buf[40];

    sprintf(buf, "%.3f", value);
    Config_save_resource(fp, resource, buf);
}

static void Config_save_int(FILE *fp, const char *resource, int value)
{
    char		buf[20];

    sprintf(buf, "%d", value);
    Config_save_resource(fp, resource, buf);
}

static void Config_save_bool(FILE *fp, const char *resource, int value)
{
    char		buf[20];

    sprintf(buf, "%s", (value != 0) ? "True" : "False");
    Config_save_resource(fp, resource, buf);
}


/*
 * Find a key in keydefs[].
 * On success set output pointer to index into keydefs[] and return true.
 * On failure return false.
 */
static int Config_find_key(keys_t key, int start, int end, int *key_index)
{
    int			i;

    for (i = start; i < end; i++) {
	if (keydefs[i].key == key) {
	    *key_index = i;
	    return true;
	}
    }

    return false;
}

static void Config_save_keys(FILE *fp)
{
    int			i, j;
    KeySym		ks;
    keys_t		key;
    const char		*str,
			*res;
    char		buf[512];

    buf[0] = '\0';
    for (i = 0; i < num_keydefs; i++) {
	ks = keydefs[i].keysym;
	key = keydefs[i].key;

	/* try and see if we have already saved this key. */
	if (Config_find_key(key, 0, i, &j) == true)
	    /* yes, saved this one before.  skip it now. */
	    continue;

	if ((str = XKeysymToString(ks)) == NULL)
	    continue;

	if ((res = Get_keyResourceString(key)) != NULL) {
	    strlcpy(buf, str, sizeof(buf));
	    /* find all other keysyms which map to the same key. */
	    j = i;
	    while (Config_find_key(key, j + 1, num_keydefs, &j) == true) {
		ks = keydefs[j].keysym;
		if ((str = XKeysymToString(ks)) != NULL) {
		    strlcat(buf, " ", sizeof(buf));
		    strlcat(buf, str, sizeof(buf));
		}
	    }
	    Config_save_resource(fp, res, buf);
	}
    }
}


static int Config_save(int widget_desc, void *button_str, const char **strptr)
{
    int			i;
    FILE		*fp = NULL;
    char		buf[512];
#ifndef _WINDOWS	/* Windows does no file handling on its own. */
    char		oldfile[PATH_MAX + 1],
			newfile[PATH_MAX + 1];

    *strptr = "Saving...";
    Widget_draw(widget_desc);
    XFlush(dpy);

    Get_xpilotrc_file(oldfile, sizeof(oldfile));
    if (oldfile[0] == '\0') {
	Config_save_failed("Can't find .xpilotrc file", strptr);
	return 1;
    }
    if ((fp = fopen(oldfile, "r")) != NULL) {
	while (fgets(buf, sizeof buf, fp))
	    Xpilotrc_add(buf);
	fclose(fp);
    }
    sprintf(newfile, "%s.new", oldfile);
    unlink(newfile);
    if ((fp = fopen(newfile, "w")) == NULL) {
	Config_save_failed("Can't open file to save to.", strptr);
	return 1;
    }
#endif

    Config_save_comment(fp,
			";\n"
			"; Config\n"
			";\n"
			"; General configuration options\n"
			";\n");
    Config_save_resource(fp, "name", nickname);
    Config_save_bool(fp, "autoShield", auto_shield);
    Config_save_int(fp, "clientPortStart", clientPortStart);
    Config_save_float(fp, "scaleFactor", scaleFactor);

    /* colors */
    Config_save_comment(fp,
			";\n"
			"; Colors\n"
			";\n"
			"; The value 0 means transparent for the color "
			"options.\n"
			";\n");
    Config_save_int(fp, "team9Color", team9Color);

    Config_save_comment(fp,
			";\n"
			"; Keys\n"
			";\n"
			"; The X Window System program xev can be used to\n"
			"; find out the names of keyboard keys.\n"
			";\n");
    Config_save_keys(fp);

    Config_save_comment(fp,
			";\n"
			"; Modifiers\n"
			";\n"
			"; These modify how your weapons work.\n"
			";\n");
    for (i = 0; i < NUM_MODBANKS; i++) {
	sprintf(buf, "modifierBank%d", i + 1);
	Config_save_resource(fp, buf, modBankStr[i]);
    }

    IFWINDOWS( Config_save_window_positions() );
    Config_save_comment(fp,
			";\n"
			"; Other options\n"
			";\n");


#ifndef _WINDOWS
    Xpilotrc_end(fp);
    fclose(fp);
    sprintf(newfile, "%s.bak", oldfile);
    rename(oldfile, newfile);
    unlink(oldfile);
    sprintf(newfile, "%s.new", oldfile);
    rename(newfile, oldfile);
#endif

    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_destroy(config_save_confirm_desc);
	config_save_confirm_desc = NO_WIDGET;
    }

    *strptr = (char *) button_str;
    return 1;
}

static int Config_save_confirm_callback(int widget_desc, void *popup_desc,
					const char **strptr)
{
    (void)widget_desc; (void)strptr;
    if (config_save_confirm_desc != NO_WIDGET) {
	Widget_destroy((int)(long int)popup_desc);
	config_save_confirm_desc = NO_WIDGET;
    }
    return 0;
}

int Config(bool doit, int what)
{
    IFWINDOWS( Trace("***Config %d\n", doit) );

    /* kps - get rid of the old widgets, it's the most easy way */
    Config_destroy();
    if (doit == false)
	return false;

    if (what == CONFIG_DEFAULT)
	config_creator = config_creator_default;
    else if (what == CONFIG_COLORS)
	config_creator = config_creator_colors;

    Create_config();
    if (config_created == false)
	return false;

    Widget_raise(config_widget_desc[config_page]);
    config_mapped = true;
    return true;
}

void Config_destroy(void)
{
    int			i;

    if (config_created == true) {
	if (config_mapped == true) {
	    Widget_unmap(config_widget_desc[config_page]);
	    config_mapped = false;
	}
	for (i = 0; i < config_max; i++)
	    Widget_destroy(config_widget_desc[i]);
	config_created = false;
	free(config_widget_desc);
	config_widget_desc = NULL;
	config_max = 0;
	config_page = 0;
    }
}

void Config_resize(void)
{
    bool		mapped = config_mapped;

    if (config_created == true) {
	Config_destroy();
	if (mapped == true)
	    Config(mapped, CONFIG_NONE);
    }
}

void Config_redraw(void)
{
    int i;

    if (!config_mapped)
	return;

    for (i = 0; i < Nelem_config_creator(); i++)
	Widget_draw(config_widget_ids[i]);
}
