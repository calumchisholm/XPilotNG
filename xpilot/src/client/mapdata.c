#ifndef	_WINDOWS
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WINDOWS
#include <io.h>
#include <direct.h>
#define snprintf _snprintf
#define F_OK 0
#define W_OK 2
#define R_OK 4
#define X_OK 0
#define mkdir(A,B) _mkdir(A)
#endif

#include <zlib.h>

#include "socklib.h"
#include "const.h"
#include "error.h"

#define COPY_BUF_SIZE 8192

typedef struct {
    char *protocol;
    char *host;
    int   port;
    char *path;
    char *query;
} URL;

extern char *texturePath;

static int Mapdata_extract (const char *name);
static int Mapdata_download (const URL *url, const char *filePath);
static int Url_parse (const char *urlstr, URL *url);
static void Url_free_parsed (URL *url);


int Mapdata_setup (const char *urlstr) {

    URL url;
    char *name, *dir, *ptr;
    char path[1024];
    int rv = FALSE;

    if (!Url_parse(urlstr, &url)) {
        error("malformed URL: %s", urlstr);
        return FALSE;
    }

    for (name = url.path + strlen(url.path) - 1; name > url.path; name--)
        if (*(name - 1) == '/') break;

    if (*name == '\0') {
        error("No file name in URL: %s", urlstr);
        goto end;
    }

    if (texturePath == NULL) {
        error("Texture path is null");
        goto end;
    }

    strncpy(path, texturePath, 1024 - 1);
    path[1024 - 1] = '\0';

    for (dir = strtok(texturePath, ":"); dir; dir = strtok(NULL, ":"))
        if (access(dir, R_OK | W_OK | X_OK) == 0)
            break;

    if (dir == NULL) {
        error("Couldn't access any directory in %s", path);
        goto end;
    }

    sprintf(path, "%s%s", dir, name);

    if (strrchr(path, '.') == NULL) {
        error("No extension in file name %s.", name);
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
            error("Not enough memory to new texturePath");
            goto end;
        }
        sprintf(temp, "%s:%s", texturePath, path);
        free(texturePath);
        texturePath = temp;
    }

    if (access(path, F_OK) == 0) {
        printf("Required bitmaps have already been downloaded.\n");
        rv = TRUE;
        goto end;
    }
    /* reset path so that it points to the package file name */
    *ptr = '.';

    printf("Downloading map data from %s to %s\n", urlstr, path);

    if (!Mapdata_download(&url, path)) {
        error("Downloading map data failed");
        goto end;
    }

    if (!Mapdata_extract(path)) {
        error("Extracting map data failed");
        goto end;
    }

    rv = TRUE;

 end:
    Url_free_parsed(&url);
    return rv;
}


static int Mapdata_extract (const char *name) {

    gzFile in;
    FILE *out;
    size_t rlen, wlen;
    char dir[256], buf[COPY_BUF_SIZE], fname[256], *ptr;
    long int size;
    int count, i;

    dir[255] = '\0';
    strncpy(dir, name, 255);
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

        sprintf(fname, "%s/", dir);

        if (sscanf(buf, "%s\n%ld\n", fname + strlen(dir) + 1, &size) != 2) {
            error("failed to parse file info %s", buf);
            gzclose(in);
            return 0;
        }

        /* security check */
        if (strchr(fname + strlen(dir) + 1, '/') != NULL) {
            error("file name %s is illegal", fname);
            gzclose(in);
            return 0;
        }

        printf("extracting %s (%ld)\n", fname, size);

        if ((out = fopen(fname, "w")) == NULL) {
            error("failed to open %s for writing", buf);
            gzclose(in);
            return 0;
        }

        while (size > 0) {
            rlen = gzread(in, buf, MIN(COPY_BUF_SIZE, size));
            if (rlen == -1) {
                error("error when reading %s", name);
                gzclose(in);
                fclose(out);
                return 0;
            }
            if (rlen == 0) {
                error("unexpected end of file %s", name);
                gzclose(in);
                fclose(out);
                return 0;
            }

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
    int rv, header, c;
    unsigned int i;
    sock_t s;
    FILE *f;
    size_t len;

    if (strncmp("http", url->protocol, 4) != 0) {
        error("unsupported protocol %s", url->protocol);
        return FALSE;
    }

    if ((f = fopen(filePath, "w")) == NULL) {
        error("failed to open %s", filePath);
        return FALSE;
    }

    if (sock_open_tcp(&s) == SOCK_IS_ERROR) {
	error("Failed to create a socket");
	fclose(f);
	return FALSE;
    }
    if (sock_connect(&s, url->host, url->port) == SOCK_IS_ERROR) {
        error("Couldn't connect to download address");
	sock_close(&s);
        fclose(f);
        return FALSE;
    }

    if (url->query) {
        if (snprintf(buf, 1024,
             "GET %s?%s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
             url->path, url->query, url->host, url->port) == -1) {
            error("too long URL");
            fclose(f);
            sock_close(&s);
            return FALSE;
        }

    } else {
        if (snprintf(buf, 1024,
             "GET %s HTTP/1.1\r\nHost: %s:%d\r\nConnection: close\r\n\r\n",
             url->path, url->host, url->port) == -1) {

            error("too long URL");
            fclose(f);
	    sock_close(&s);
            return FALSE;
        }
    }

    if (sock_write(&s, buf, strlen(buf)) == -1) {
        error("socket write failed");
        fclose(f);
        sock_close(&s);
        return FALSE;
    }

    header = 1;
    c = 0;

    for(;;) {
        if ((len = read(s.fd, buf, 1024)) == -1) {
            error("socket read failed");
            rv = FALSE;
            break;
        }

        if (len == 0) {
            rv = TRUE;
            break;
        }

        printf("#");
        fflush(stdout);

        if (header) {
            for (i = 0; i < len; i++) {

                if (c % 2 == 0 && buf[i] == '\r') c++;
                else if (c % 2 == 1 && buf[i] == '\n') c++;
                else c = 0;

                if (c == 4) {
                    header = 0;
                    if (i < len - 1) {
                        memmove(buf, buf + i + 1, len - i - 1);
                        len = len - i - 1;
                    }
                }
            }
        }

        if (!header) {
            if (fwrite(buf, 1, len, f) == -1) {
                error("file write failed");
                rv =  FALSE;
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
        return FALSE;
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
        return FALSE;
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
    if (beg >= len) return TRUE;

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
        if (beg >= len) return TRUE;
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
    if (beg >= len) return TRUE;

    url->query = buf + beg;
    return TRUE;
}


static void Url_free_parsed (URL *url) {
    free(url->protocol);
}
