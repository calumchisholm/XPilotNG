#ifndef TEXT_H
#define TEXT_H

#include <GL/gl.h>
#include <GL/glu.h>

#include "xpclient.h"
#include "sdlpaint.h"

#define LEFT 0
#define DOWN 0
#define CENTER 1
#define RIGHT 2
#define UP 2

#define NUMCHARS 256

typedef struct {
    GLfloat MinX;
    GLfloat MinY;
    GLfloat MaxX;
    GLfloat MaxY;
} texcoord_t;    

typedef struct {
    GLuint textures[NUMCHARS]; /* texture indexes for the characters */
    GLuint W[NUMCHARS]; /* holds paint width fr each character */
    GLuint list_base; /* start of the texture list for this font */
    GLuint h; /* char height */
    GLuint linespacing; /* proper line spacing according to FT */
    void *ttffont;
} font_data;

typedef struct {
    GLuint texture;
    texcoord_t texcoords;
    int width;
    int height;
    int font_height;
} string_tex_t;

int renderstyle;
enum {
	RENDER_LATIN1,
	RENDER_UTF8,
	RENDER_UNICODE
} rendertype;

typedef struct {
    	float width;
	float height;
} fontbounds;

/* The init function will create a font of
 * of the height h from the file fname.
 */
int fontinit(font_data *ft_font, const char * fname, unsigned int size);

/* Free all the resources assosiated with the font.*/
void fontclean(font_data *ft_font);

/* loads a SDL surface onto a GL texture */
GLuint SDL_GL_LoadTexture(SDL_Surface *surface, texcoord_t *texcoord);

/* Calcs the bounding width,height for the text if it were printed
 * to screen with given font
 */
fontbounds nprintsize(font_data *ft_font, int length, const char *fmt, ...);
fontbounds printsize(font_data *ft_font, const char *fmt, ...);

/* 
 * NOTE: passing color 0x00000000 causes the painting to *not* set color,
 * it does *not* mean the text will be drawn with color 0x00000000, you
 * should check for that before calling this function.
 */
void HUDnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt, ...);
void mapnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt,...);
void HUDprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *fmt, ...);
void mapprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *fmt,...);

bool draw_text(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *text, bool savetex, string_tex_t *string_tex, bool onHUD);
bool draw_text_fraq(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *text
    	    	    , float xstart
    	    	    , float xstop
    	    	    , float ystart
    	    	    , float ystop
		    , bool savetex, string_tex_t *string_tex, bool onHUD);
bool render_text(font_data *ft_font, const char *text, string_tex_t *string_tex);
void disp_text(string_tex_t *string_tex, int color, int XALIGN, int YALIGN, int x, int y, bool onHUD);
void disp_text_fraq(string_tex_t *string_tex, int color, int XALIGN, int YALIGN, int x, int y
    	    	    , float xstart
    	    	    , float xstop
    	    	    , float ystart
    	    	    , float ystop
		    , bool onHUD);
void free_string_texture(string_tex_t *string_tex);

font_data gamefont;
font_data messagefont;
font_data mapfont;

string_tex_t score_object_texs[MAX_SCORE_OBJECTS];

/*typedef struct {
    int id;
    string_tex_t string_tex;
    void *next;
} name_tex_t;

name_tex_t *others_name_texs;*/

#define MAX_METERS 12
string_tex_t meter_texs[MAX_METERS];
string_tex_t message_texs[2*MAX_MSGS];
#define MAX_HUD_TEXS 10 
string_tex_t HUD_texs[MAX_HUD_TEXS+MAX_SCORE_OBJECTS];
#endif
