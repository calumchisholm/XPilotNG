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

#include "xpserver.h"

char auth_version[] = VERSION;

#if 0
/*
 * This function returns -1 if an error occurred or 0 if there aren't any
 * nicks with options.password protection. Otherwise, it returns 1 and saves the
 * memory address of the file's contents to *p.
 */
static int Read_player_passwords_file(char **p, int *size)
{
    FILE *fp;
    char *fname, *buf = NULL;
    struct stat st;
    int fd;

    if ((fname = playerPasswordsFileName) == NULL)
	fname = Conf_player_passwords_file_name();

    if (size)
	*size = 0;

    fp = fopen(fname, "r");
    if (!fp) {
	if (errno == ENOENT)
	    return 0;
	error("Error opening player passwords file for reading");
	return -1;
    }
    fd = fileno(fp);

    while (lockf(fd, F_LOCK, 0) && errno == EINTR)
	;

    if (fstat(fd, &st)) {
	error("Error checking file size of player passwords file before reading");
	fflush(fp);
	lockf(fd, F_ULOCK, 0);
	fclose(fp);
	return -1;
    }

    if (!st.st_size) {
	fflush(fp);
	lockf(fd, F_ULOCK, 0);
	fclose(fp);
	return 0;
    }

    if (size)
	*size = st.st_size;

    if (!(buf = malloc(st.st_size + 1))) {
	fflush(fp);
	lockf(fd, F_ULOCK, 0);
	fclose(fp);
	warn("Out of memory before reading player passwords file.");
	return -1;
    }

    if (fread(buf, st.st_size, 1, fp) != 1) {
	error("Couldn't read player password file");
	fflush(fp);
	lockf(fd, F_ULOCK, 0);
	fclose(fp);
	free(buf);
	return -1;
    }
    buf[st.st_size] = 0;

    fflush(fp);
    lockf(fd, F_ULOCK, 0);
    fclose(fp);
    *p = buf;
    return 1;
}

static int Write_player_passwords_file(const char *data)
{
    FILE *fp;
    char *fname;
    mode_t m;
    int fd, rv;

    if ((fname = playerPasswordsFileName) == NULL)
	fname = Conf_player_passwords_file_name();

    m = umask(0077);
    fp = fopen(fname, "w");
    umask(m);
    if (!fp) {
	error("Error opening player passwords file for writing");
	return -1;
    }
    fd = fileno(fp);

    while (lockf(fd, F_LOCK, 0) && errno == EINTR)
	;

    if (strlen(data) && fwrite(data, strlen(data), 1, fp) != 1) {
	error("Error writing player passwords file");
	rv = -1;
    } else
	rv = 0;

    fflush(fp);
    lockf(fd, F_ULOCK, 0);
    fclose(fp);
    return rv;
}

int Check_player_password(const char *nick, const char *options.password)
{
    char *fcont, salt[3], nick_l[MAX_NAME_LEN], *p, *p2, *line_start = NULL,
	*colon = NULL, *fcont_new = NULL;
    int r, found_entry = 0, fsize, err = 0;

    if (!*nick) {
	warn("Check_player_password(): Nick is null string.");
	strcpy(ShutdownReason, "server error");
	ShutdownServer = 0;
	End_game();
	/* NOT REACHED */
    }

    strlcpy(nick_l, nick, sizeof(nick_l));
    for (r = strlen(nick_l); r--;)
	nick_l[r] = tolower(nick_l[r]);

    r = Read_player_passwords_file(&fcont, &fsize);
    if (!r)
	return PASSWD_NONE;
    else if (r == -1)
	return PASSWD_ERROR;

    salt[2] = 0;
    r = PASSWD_NONE;
    p = fcont;
    while ((p = strstr(p, nick_l)) != NULL) {
	/* Are we at the beginning of a line? */
	if (p > fcont && *(p-1) != '\n' && *(p-1) != '\r') {
	    /* We're NOT at the beginning of a line. */
	    p++;
	    continue;
	}

	/* We ARE at the beginning of a line. */
	line_start = p;
	p += strlen(nick_l);
	if (*p != ':')
	    /* Wrong or invalid entry. */
	    continue;

	/* Okay, we found a options.password entry for the nick. */
	found_entry = 1;
	p++;
	p2 = strpbrk(p, ":\r\n");
	if (!p2 || *p2 != ':') {
	    warn("Player passwords file seems to be corrupt. Check the entry "
		 "for player \"%s\".", nick_l);
	    r = PASSWD_ERROR;
	    err = 1;
	    break;
	}

	if (p2 == p) /* Password field empty. */
	    break;

	if (p2 - p != 13) {
	    warn("Player passwords file seems to be corrupt. Check the entry "
		 "for player \"%s\".", nick_l);
	    r = PASSWD_ERROR;
	    err = 1;
	    break;
	}

	colon = p2;
	strncpy(salt, p, 2);
	p2 = crypt(options.password, salt);
	if (!p2) {
	    warn("crypt() failed when trying to check the password of "
		 "player \"%s\".", nick_l);
	    r = PASSWD_ERROR;
	    break;
	}

	*colon = 0;

	if (!strcmp(p, p2))
	    r = PASSWD_OK;
	else
	    r = PASSWD_WRONG;

	*colon = ':';

	break;
    }

    if (r != PASSWD_ERROR && found_entry && !err) {

	/*
	 * Update the 'lastaccess' field.
	 */

	int okay = 0;
	char *lastaccess_start = NULL, newline[3];

	p = colon + 1;

	/* 'p' now points at the entry after the options.password entry. */

	while (*p && isdigit(*p))
	    p++;
	if (!*p || *p != ':') {
	    warn("Player passwords file seems to be corrupt. Check the entry "
		 "for player \"%s\".", nick_l);
	    /*
	     * Okay, it's possible that r is now PASSWD_OK, but we want the
	     * server admin to know that the player passwords file is corrupt.
	     */
	    r = PASSWD_ERROR;
	}
	else {
	    p++;
	    lastaccess_start = p;

	    while (*p && isdigit(*p))
		p++;
	    if (*p && *p != '\r' && *p != '\n') {
		warn("Player passwords file seems to be corrupt. Check the "
		     "entry for player \"%s\".", nick_l);
		r = PASSWD_ERROR;
	    }
	    else {
		if (*p && *p == '\r' && *(p+1) == '\n')
		    p += 2;
		else
		if (*p)
		    p++;

		okay = 1;
	    }
	}

	if (okay) {
	    char *line_next = p;
	    int len;

	    if (!(fcont_new = malloc(fsize + 1000))) {
		warn("Out of memory while updating player password entry "
		     "of player \"%s\".", nick_l);
		r = PASSWD_ERROR;
	    }
	    else {
		memcpy(fcont_new, line_start, lastaccess_start - line_start);
		sprintf(fcont_new + (lastaccess_start - line_start),
			"%d", (int) time(NULL));

		p = fcont;
		while (*p && *p != '\r' && *p != '\n')
		    p++;
		if (!*p)
#ifdef _WINDOWS
		    strcpy(newline, "\r\n");
#elif 0 /* Macintosh(?) and possibly other machines. */
		    strcpy(newline, "\r");
#else
		    strcpy(newline, "\n");
#endif
		else
		if (*p == '\n')
		    strcpy(newline, "\n");
		else
		if (*(p+1) != '\n')
		    strcpy(newline, "\r");
		else
		    strcpy(newline, "\r\n");

		strcat(fcont_new, newline);
		len = strlen(fcont_new);

		memcpy(fcont_new + len, fcont, line_start - fcont);
		len += line_start - fcont;

		memcpy(fcont_new + len, line_next, fcont + fsize - line_next + 1);
	    }
	}
    }

    if (fcont_new) {

	/*
	 * 'lastaccess' field has been updated and options.password entry has been
	 * moved to the beginning of the file.
	 */

	if (Write_player_passwords_file(fcont_new)) {
	    warn("Check_player_password(): Write error while updating "
		 "player passwords file.");
	    r = PASSWD_ERROR;
	}
    }

    free(fcont_new);
    free(fcont);
    return r;
}

int Remove_player_password(const char *nick)
{
    char *p, *p2, *fcont, nick_l[MAX_NAME_LEN];
    int r, r2, fsize;

    if (!*nick) {
	warn("Remove_player_password(): Nick is null string.");
	strcpy(ShutdownReason, "server error");
	ShutdownServer = 0;
	End_game();
    }

    strlcpy(nick_l, nick, sizeof(nick_l));
    for (r = strlen(nick_l); r--;)
	nick_l[r] = tolower(nick_l[r]);

    r = Read_player_passwords_file(&fcont, &fsize);
    if (!r)
	return 0;
    else if (r == -1)
	return -1;

    r = 0;
    p = fcont;
    while ((p = strstr(p, nick_l)) != NULL) {
	/* Are we at the beginning of a line? */
	if (p > fcont && *(p-1) != '\n' && *(p-1) != '\r') {
	    /* We're NOT at the beginning of a line. */
	    p++;
	    continue;
	}

	/* We ARE at the beginning of a line. */
	p2 = p + strlen(nick_l);
	if (*p2 != ':') {
	    /* Wrong or invalid entry. */
	    p++;
	    continue;
	}

	p2 = strpbrk(p2, "\n\r");
	if (!p2) {
	    warn("Player passwords file seems to be corrupt. Check the entry "
		 "for player \"%s\".", nick_l);
	    r = -1;
	    break;
	}

	if (*p2 == '\r' && *(p2+1) == 'n')
	    p2 += 2;
	else
	    p2++;

	memmove(p, p2, fsize+1 - (p2-fcont));

	r = 1;
	break;
    }

    r2 = 0;
    if (r == 1)
	r2 = Write_player_passwords_file(fcont);

    free(fcont);

    if (r2 == -1)
	return -1;

    return r;
}

int Set_player_password(const char *nick, const char *options.password, int new)
{
    char *fcont = NULL, *p, *p2, *cpass, salt[3];
    int r, fsize;

    r = 0;
    if (!new)
	r = Remove_player_password(nick);

    if (r == -1)
	return -1;

    r = Read_player_passwords_file(&fcont, &fsize);
    if (r == -1)
	return -1;

    p = malloc(fsize + 1000);
    if (!p) {
	warn("Out of memory when trying to set a player password.");
	free(fcont);
	return -1;
    }

    *p = 0;

    strlcpy(p, nick, MAX_NAME_LEN);
    for (r = strlen(p); r--;)
	p[r] = tolower(p[r]);

    salt[0] = nick[0];
    salt[1] = nick[strlen(nick)-1];
    salt[2] = 0;
    cpass = crypt(options.password, salt);
    if (!cpass) {
	warn("crypt() failed when trying to set a player password.");
	free(fcont);
	free(p);
	return -1;
    }
    sprintf(p + strlen(p), ":%s:0:%d", cpass, (int) time(NULL));

    /* Now add newline character(s) (\n or \r\n or \r). */
    if (fcont && (p2 = strpbrk(fcont, "\r\n"))) {
	/* We use the newline character(s) used in the existing file. */
	if (*p2 == '\r') {
	    if (*(p2+1) == '\n')
		strcat(p, "\r\n");
	    else
		strcat(p, "\r");
	} else
	    strcat(p, "\n");
    } else
#ifdef _WINDOWS
	strcat(p, "\r\n");
#elif 0 /* Macintosh(?) and possibly other machines. */
	strcat(p, "\r");
#else
	strcat(p, "\n");
#endif

    if (fcont)
	memcpy(p + strlen(p), fcont, fsize + 1);

    /*
     * We update old passwords regardless of the file size limit, since
     * we already removed the options.password and we should update it now even
     * if the new line is a little longer than the old line.
     */
    if  (new && strlen(p) > playerPasswordsFileSizeLimit) {
	warn("Warning: Maximum player passwords file size too low to allow "
		"a new player password.");
	r = -2;
    } else
	r = Write_player_passwords_file(p);

    free(fcont);
    free(p);

    return r;
}
#endif
