/* 
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2003 by
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

/* kps - you should be able to change this without a recompile */
#define DATADIR ".xpilot_data"
#define COPY_BUF_SIZE 8192

typedef struct {
    char *protocol;
    char *host;
    int   port;
    const char *path;
    char *query;
} URL;

static int Mapdata_extract (const char *name);
static int Mapdata_download (const URL *url, const char *filePath);
static int Url_parse (const char *urlstr, URL *url);
static void Url_free_parsed (URL *url);


int Mapdata_setup(const char *urlstr)
{
    URL url;
    const char *name, *dir;
    char path[1024], buf[1024], *ptr;
    int rv = false;

    memset(path, 0, sizeof(path));
    memset(buf, 0, sizeof(buf));

    if (!Url_parse(urlstr, &url)) {
	warn("malformed URL: %s", urlstr);
	return false;
    }
	
    for (name = url.path + strlen(url.path) - 1; name > url.path; name--) {
	if (*(name - 1) == '/')
	    break;
    }

    if (*name == '\0') {
	warn("no file name in URL: %s", urlstr);
	goto end;
    }

    if (texturePath == NULL) {
	warn("texture path is null");
	goto end;
    }

    for (dir = strtok(texturePath, ":"); dir; dir = strtok(NULL, ":"))
	if (access(dir, R_OK | W_OK | X_OK) == 0)
	    break;
    
    if (dir == NULL) {
	
	/* texturePath hasn't got a directory with proper access rights */
	/* so lets create one into users home dir */

	char *home = getenv("HOME");
	if (home == NULL) {
	    error("couldn't access any dir in %s and HOME is unset", path);
	    goto end;
	}
	
	if (strlen(home) == 0)
	    sprintf(buf, "%s", DATADIR);
	else if (home[strlen(home) - 1] == PATHNAME_SEP) 
	    sprintf(buf, "%s%s", home, DATADIR);
	else
	    sprintf(buf, "%s%c%s", home, PATHNAME_SEP, DATADIR);

	if (access(buf, F_OK) != 0) {
	    if (mkdir(buf, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
		error("failed to create directory %s", dir);
		goto end;
	    }
	}

	dir = buf;
    }
    
    if (strlen(dir) == 0)
	sprintf(path, "%s", name);
    else if (dir[strlen(dir) - 1] == PATHNAME_SEP) 
	sprintf(path, "%s%s", dir, name);
    else
	sprintf(path, "%s%c%s", dir, PATHNAME_SEP, name);

    if (strrchr(path, '.') == NULL) {
	error("no extension in file name %s.", name);
	goto end;
    }
    
    /* temporarily make path point to the directory name */
    ptr = strrchr(path, '.');
    *ptr = '\0';

    /* add this new texture directory to texturePath */
    if (texturePath == NULL) {
	texturePath = strdup(path);
    } else {
	char *temp = malloc(strlen(texturePath) + strlen(path) + 2);
	if (temp == NULL) {
	    error("not enough memory to new texturePath");
	    goto end;
	}
	sprintf(temp, "%s:%s", texturePath, path);
	free(texturePath);
	texturePath = temp;
    }

    if (access(path, F_OK) == 0) {
	printf("Required bitmaps have already been downloaded.\n");
	rv = true;
	goto end;
    }
    /* reset path so that it points to the package file name */
    *ptr = '.';

    printf("Downloading map data from %s to %s.\n", urlstr, path);

    if (!Mapdata_download(&url, path)) {
	warn("downloading map data failed");
	goto end;
    }

    if (!Mapdata_extract(path)) {
	warn("extracting map data failed");
	goto end;
    }

    rv = true;

 end:
    Url_free_parsed(&url);
    return rv;
}


static int Mapdata_extract (const char *name)
{
    gzFile in;
    FILE *out;
    int retval;
    size_t rlen, wlen;
    char dir[256], buf[COPY_BUF_SIZE], fname[256], *ptr;
    long int size;
    int count, i;

    strlcpy(dir, name, sizeof dir);
    ptr = strrchr(dir, '.');
    if (ptr == NULL) {
	error("file name has no extension %s", dir);
	return 0;
    }
    *ptr = '\0';

    if (mkdir(dir, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
	error("failed to create directory %s", dir);
	return 0;
    }

    if ((in = gzopen(name, "rb")) == NULL) {
	error("failed to open %s for reading", name);
	return 0;
    }

    if (gzgets(in, buf, COPY_BUF_SIZE) == Z_NULL) {
	error("failed to read header from %s", name);
	gzclose(in);
	return 0;
    }

    if (sscanf(buf, "XPD %d\n", &count) != 1) {
	error("invalid header in %s", name);
	gzclose(in);
	return 0;
    }

    for (i = 0; i < count; i++) {

	if (gzgets(in, buf, COPY_BUF_SIZE) == Z_NULL) {
	    error("failed to read file info from %s", name);
	    gzclose(in);
	    return 0;
	}

	sprintf(fname, "%s%c", dir, PATHNAME_SEP);

	if (sscanf(buf, "%s\n%ld\n", fname + strlen(dir) + 1, &size) != 2) {
	    error("failed to parse file info %s", buf);
	    gzclose(in);
	    return 0;
	}

	/* security check */
	if (strchr(fname + strlen(dir) + 1, PATHNAME_SEP) != NULL) {
	    error("file name %s is illegal", fname);
	    gzclose(in);
	    return 0;
	}

	printf("Extracting %s (%ld)\n", fname, size);

	if ((out = fopen(fname, "wb")) == NULL) {
	    error("failed to open %s for writing", buf);
	    gzclose(in);
	    return 0;
	}

	while (size > 0) {
	    retval = gzread(in, buf, MIN(COPY_BUF_SIZE, (unsigned)size));
	    if (retval == -1) {
		error("error when reading %s", name);
		gzclose(in);
		fclose(out);
		return 0;
	    }
	    if (retval == 0) {
		error("unexpected end of file %s", name);
		gzclose(in);
		fclose(out);
		return 0;
	    }

	    rlen = retval;
	    wlen = fwrite(buf, 1, rlen, out);
	    if (wlen != rlen) {
		error("failed to write to %s", fname);
		gzclose(in);
		fclose(out);
		return 0;
	    }

	    size -= rlen;
	}

	fclose(out);
    }

    gzclose(in);
    return 1;
}


static int Mapdata_download (const URL *url, const char *filePath)
{
    char buf[1024];
    int rv, header, c, len, i;
    sock_t s;
    FILE *f;
    size_t n;

    if (strncmp("http", url->protocol, 4) != 0) {
	error("unsupported protocol %s", url->protocol);
	return false;
    }

    if ((f = fopen(filePath, "wb")) == NULL) {
	error("failed to open %s", filePath);
	return false;
    }

    if (sock_open_tcp(&s) == SOCK_IS_ERROR) {
	error("failed to create a socket");
	fclose(f);
	return false;
    }
    if (sock_connect(&s, url->host, url->port) == SOCK_IS_ERROR) {
	error("couldn't connect to download address");
	sock_close(&s);
	fclose(f);
	return false;
    }

    if (url->query) {
	if (snprintf(buf, sizeof buf,
	     "GET %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
	     url->path, url->query, url->host, url->port) == -1) {
	    error("too long URL");
	    fclose(f);
	    sock_close(&s);
	    return false;
	}

    } else {
	if (snprintf(buf, sizeof buf,
	     "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
	     url->path, url->host, url->port) == -1) {

	    error("too long URL");
	    fclose(f);
	    sock_close(&s);
	    return false;
	}
    }

    if (sock_write(&s, buf, (int)strlen(buf)) == -1) {
	error("socket write failed");
	fclose(f);
	sock_close(&s);
	return false;
    }

    header = 1;
    c = 0;

    for(;;) {
	if ((len = sock_read(&s, buf, sizeof buf)) == -1) {
	    error("socket read failed");
	    rv = false;
	    break;
	}

	if (len == 0) {
	    rv = true;
	    break;
	}

	printf("#");
	fflush(stdout);

	if (header) {
	    for (i = 0; i < len; i++) {
		if (c % 2 == 0 && buf[i] == '\r')
		    c++;
		else if (c % 2 == 1 && buf[i] == '\n')
		    c++;
		else
		    c = 0;

		if (c == 4) {
		    header = 0;
		    if (i < len - 1) {
			n = len - i - 1;
			memmove(buf, buf + i + 1, n);
			len = len - i - 1;
		    }
		}
	    }
	}

	if (!header) {
	    n = len;
	    if (fwrite(buf, 1, n, f) < n) {
		error("file write failed");
		rv =  false;
		break;
	    }
	}
    }
    printf("\n");
    fclose(f);
    sock_close(&s);
    return rv;
}


static int Url_parse (const char *urlstr, URL *url)
{
    int len, i, beg, doPort;
    char *buf;

    memset(url, 0, sizeof(URL));
    url->port = 80;
    url->path = "/";

    len = strlen(urlstr);
    buf = strdup(urlstr);
    if (buf == NULL) {
	error("no memory for URL");
	return false;
    }

    for (i = 0; i < len; i++) {
	if (buf[i] == ':') {
	    buf[i] = '\0';
	    url->protocol = buf;
	    break;
	}
    }

    beg = i + 3;
    if (beg >= len || buf[i + 1] != '/' || buf[i + 2] != '/') {
	free(buf);
	return false;
    }

    doPort = 0;
    for (i = beg; i < len; i++) {
	if (buf[i] == ':' || buf[i] == '/') {
	    if (buf[i] == ':') doPort = 1;
	    buf[i] = '\0';
	    break;
	}
    }

    url->host = buf + beg;
    beg = i + 1;
    if (beg >= len) return true;

    if (doPort) {
	for (i = beg; i < len; i++) {
	    if (buf[i] == '/') {
		buf[i] = '\0';
		break;
	    }
	}
	url->port = atoi(buf + beg);
	/* error detection should be added */

	beg = i + 1;
	if (beg >= len)
	    return true;
    }

    /* make space for / in the beginning of path */
    memmove(url->host - 1, url->host, strlen(url->host) + 1);
    url->host--;
    buf[beg - 1] = '/';

    for (i = beg; i < len; i++) {
	if (buf[i] == '?') {
	    buf[i] = '\0';
	    break;
	}
    }
    url->path = buf + beg - 1;

    beg = i + 1;
    if (beg >= len) return true;

    url->query = buf + beg;
    return true;
}


static void Url_free_parsed (URL *url)
{
    free(url->protocol);
}
