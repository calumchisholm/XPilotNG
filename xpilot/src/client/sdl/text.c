/*
    glfont:  An example of using the SDL_ttf library with OpenGL.
    Copyright (C) 1997, 1998, 1999, 2000, 2001  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    The SDL_GL_* functions in this file are available in the public domain.

    Sam Lantinga
    slouken@libsdl.org
*/

/*
 * This code was created by Jeff Molofee '99 
 * (ported to Linux/SDL by Ti Leggett '01)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 * 
 * or for port-specific comments, questions, bugreports etc. 
 * email to leggett@eecs.tulane.edu
 */

/* $Id$ */
/* modified for xpilot by Erik Andersson deity@home.se */

#include <string.h>
#include <stdarg.h>
#include <GL/gl.h>
#include "SDL.h"

#ifdef HAVE_SDL_TTF
    #include "SDL_ttf.h"
#endif

#include "text.h"

#define NUM_TEXTURES 1

float modelview_matrix[16];
const int bufsize = 1024;
	
int LoadBMP(font_data *ft_font, const char * fname);
void pushScreenCoordinateMatrix(void);
void pop_projection_matrix(void);
int next_p2 ( int a );
void print(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *text, bool onHUD);
#ifdef HAVE_SDL_TTF
int FTinit(font_data *font, const char * fontname, int ptsize);
#endif
int next_p2 ( int a )
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

int LoadBMP(font_data *ft_font, const char * fname)
{
    /* Status indicator */
    int Status = 0;

    /* Create storage space for the texture */
    SDL_Surface *TextureImage;

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage = SDL_LoadBMP( fname ) ))
        {
	    ft_font->h = TextureImage->h/16;
	    ft_font->linespacing = ft_font->h*1.3;/*this is now recommended spacing*/
	    ft_font->W[0] = TextureImage->w/16;
	    /* Set the status to true */
	    Status = 1;

	    /* Create The Texture */
	    glGenTextures( NUM_TEXTURES, &ft_font->textures[0] );

	    /* Load in texture 1 */
	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, ft_font->textures[0] );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage->w,
			  TextureImage->h, 0, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage->pixels );
	    /* Nearest Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }

    /* Free up any memory we may have used */
    if ( TextureImage )
	SDL_FreeSurface( TextureImage );

    return Status;
}

GLuint SDL_GL_LoadTexture(SDL_Surface *surface, texcoord_t *texcoord)
{
    GLuint texture;
    int w, h;
    SDL_Surface *image;
    SDL_Rect area;
    Uint32 saved_flags;
    Uint8  saved_alpha;

    /* Use the surface width and height expanded to powers of 2 */
    w = next_p2(surface->w);
    h = next_p2(surface->h);
    texcoord->MinX = 0.0f;		 /* Min X */
    texcoord->MinY = 0.0f;		 /* Min Y */
    texcoord->MaxX = (GLfloat)surface->w / w;  /* Max X */
    texcoord->MaxY = (GLfloat)surface->h / h;  /* Max Y */

    image = SDL_CreateRGBSurface(
    		    SDL_SWSURFACE,
    		    w, h,
    		    32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF, 
			0x0000FF00, 
			0x00FF0000, 
			0xFF000000
#else
			0xFF000000,
			0x00FF0000, 
			0x0000FF00, 
			0x000000FF
#endif
    		   );
    if ( image == NULL ) {
    	    return 0;
    }

    /* Save the alpha blending attributes */
    saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
    saved_alpha = surface->format->alpha;
    if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    	    SDL_SetAlpha(surface, 0, 0);
    }

    /* Copy the surface into the GL texture image */
    area.x = 0;
    area.y = 0;
    area.w = surface->w;
    area.h = surface->h;
    SDL_BlitSurface(surface, &area, image, &area);

    /* Restore the alpha blending attributes */
    if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    	    SDL_SetAlpha(surface, saved_flags, saved_alpha);
    }

    /* Create an OpenGL texture for the image */
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
    		 0,
    		 GL_RGBA,
    		 w, h,
    		 0,
    		 GL_RGBA,
    		 GL_UNSIGNED_BYTE,
    		 image->pixels);
    SDL_FreeSurface(image); /* No longer needed */

    return texture;
}

#ifdef HAVE_SDL_TTF
int FTinit(font_data *font, const char * fontname, int ptsize)
{
    int i;
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0x00 };
    SDL_Color black = { 0x00, 0x00, 0x00, 0 };
    SDL_Color *forecol;
    SDL_Color *backcol;
    GLenum gl_error;
    texcoord_t texcoords;
    int minx = 0,miny = 0,maxx = 0,maxy = 0;
    GLuint height;

    /* We might support changing theese later */
    /* Look for special rendering types */
    renderstyle = TTF_STYLE_NORMAL;
    rendertype = RENDER_LATIN1;
    /* Default is black and white */
    forecol = &white;
    backcol = &black;
    
    /* Initialize the TTF library */
    /*if ( TTF_Init() < 0 ) {
    	fprintf(stderr, "Couldn't initialize TTF: %s\n",SDL_GetError());
    	return(2);
    }*/
    font->ttffont = TTF_OpenFont(fontname, ptsize);
    if ( font->ttffont == NULL ) {
    	fprintf(stderr, "Couldn't load %d pt font from %s: %s\n", ptsize, fontname, SDL_GetError());
    	return(2);
    }
    TTF_SetFontStyle(font->ttffont, renderstyle);
    font->list_base=glGenLists(next_p2(NUMCHARS));
    /* Get the recommended spacing between lines of text for this font */
    font->linespacing = TTF_FontLineSkip(font->ttffont);
    font->h = ptsize;

    for( i = 0; i < NUMCHARS; i++ ) {
	SDL_Surface *glyph = NULL;
	forecol = &white;
	
    	glyph = TTF_RenderGlyph_Blended( font->ttffont, i, *forecol );
    	if(glyph) {
	    glGetError();
    	    font->textures[i] = SDL_GL_LoadTexture(glyph, &texcoords);
    	    if ( (gl_error = glGetError()) != GL_NO_ERROR )
	    	printf("Warning: Couldn't create texture: 0x%x\n", gl_error);
	    
    	    font->W[i] = glyph->w;
    	    height = glyph->h;
    	    TTF_GlyphMetrics( font->ttffont, i, &minx,&maxx,&miny,&maxy,NULL);
   	}    
    	SDL_FreeSurface(glyph);
		
    	glNewList(font->list_base+i,GL_COMPILE);

    	glBindTexture(GL_TEXTURE_2D, font->textures[i]);
    	glTranslatef(1,0,0);
    	glPushMatrix();
    	glBegin(GL_TRIANGLE_STRIP);
    	    glTexCoord2f(texcoords.MinX, texcoords.MaxY);
	    glVertex2i(0 , miny);
     	    glTexCoord2f(texcoords.MaxX, texcoords.MaxY);
	    glVertex2i(font->W[i] , miny);
    	    glTexCoord2f(texcoords.MinX, texcoords.MinY);
	    glVertex2i(0 ,miny+height );
   	    glTexCoord2f(texcoords.MaxX, texcoords.MinY);
	    glVertex2i(font->W[i] , miny+height);
    	glEnd();
    	glPopMatrix();
    	glTranslatef((font->W[i]>3)?font->W[i]:(font->W[i] = 3) + 1,0,0);
	/*one would think this should be += 2... I guess they overlap or the edge
	 * isn't painted
	 */
	font->W[i] += 1;

    	glEndList();
    }
    
    /*TTF_CloseFont(font->ttffont);*/
    /*TTF_Quit();*/
    return 0;
}
#endif

int fontinit(font_data *ft_font, const char * fname, unsigned int size)
{
    GLuint loop; /* Loop variable               */
    float cx;    /* Holds Our X Character Coord */
    float cy;    /* Holds Our Y Character Coord */    
    
    if (strlen(fname)<5) return 1; /*not long enough to be a useful name*/
    if (strcasecmp(".bmp",&fname[strlen(fname)-4]) != 0)
    /* if its not a .bmp file lets try freetype if available */
    #ifdef HAVE_SDL_TTF
    	return FTinit(ft_font,fname,size);
    #else
    {
    	xpprintf("fontfile has to be foo.bmp\n");
    }
    #endif
    if(!LoadBMP(ft_font,fname))
    	return 1;
    
    ft_font->list_base  = glGenLists( NUMCHARS );

    /* Loop Through All NUM_CHARS Lists */
    for ( loop = 0; loop < NUMCHARS; loop++ )
        {
	    /* NOTE:
	     *  BMPs are stored with the top-leftmost pixel being the
	     * last byte and the bottom-rightmost pixel being the first
	     * byte. So an image that is displayed as
	     *    1 0
	     *    0 0
	     * is represented data-wise like
	     *    0 0
	     *    0 1
	     * And because SDL_LoadBMP loads the raw data without
	     * translating to how it is thought of when viewed we need
	     * to start at the bottom-right corner of the data and work
	     * backwards to get everything properly. So the below code has
	     * been modified to reflect this. Examine how this is done and
	     * how the original tutorial is done to grasp the differences.
	     *
	     * As a side note BMPs are also stored as BGR instead of RGB
	     * and that is why we load the texture using GL_BGR. It's
	     * bass-ackwards I know but whattaya gonna do?
	     */
    	    ft_font->W[loop] = ft_font->W[0];
	    /* X Position Of Current Character */
	    cx = 1 - ( float )( loop % 16 ) / 16.0f;
	    /* Y Position Of Current Character */
	    cy = 1 - ( float )( loop / 16 ) / 16.0f;

            /* Start Building A List */
	    glNewList( ft_font->list_base + ( 255 - loop ), GL_COMPILE );
	      /* Use A Quad For Each Character */
	      glBegin( GL_QUADS );
	        /* Texture Coord (Bottom Left) */
	        glTexCoord2f( cx - 0.0625, cy );
		/* Vertex Coord (Bottom Left) */
		glVertex2i( 0, 0 );

		/* Texture Coord (Bottom Right) */
		glTexCoord2f( cx, cy );
		/* Vertex Coord (Bottom Right) */
		glVertex2i( 16, 0 );

		/* Texture Coord (Top Right) */
		glTexCoord2f( cx, cy - 0.0625f );
		/* Vertex Coord (Top Right) */
		glVertex2i( 16, 16 );

		/* Texture Coord (Top Left) */
		glTexCoord2f( cx - 0.0625f, cy - 0.0625f);
		/* Vertex Coord (Top Left) */
		glVertex2i( 0, 16 );
	      glEnd( );

	      /* Move To The Left Of The Character */
	      glTranslated( 10, 0, 0 );
	    glEndList( );
        }
    return 0;
}

void fontclean(font_data *ft_font)
{
    if (ft_font == NULL) return;
    glDeleteLists(ft_font->list_base,next_p2(NUMCHARS));
    if (ft_font->textures != NULL) {
    	glDeleteTextures(NUMCHARS,ft_font->textures);
    }
}

/* A fairly straight forward function that pushes
 * a projection matrix that will make object world 
 * coordinates identical to window coordinates.
 */
void pushScreenCoordinateMatrix(void)
{
	glPushAttrib(GL_TRANSFORM_BIT);
	GLint	viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0],viewport[2],viewport[1],viewport[3]);
	glPopAttrib();
}

/* Pops the projection matrix without changing the current
 * MatrixMode.
 */
void pop_projection_matrix(void)
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}


fontbounds nprintsize(font_data *ft_font, int length, const char *fmt, ...)
{
    unsigned int i=0,j,textlength;
    float len;
    fontbounds returnval;
    int start,end,toklen;
    
    returnval.width=0.0;
    returnval.height=0.0;
    
    if (ft_font == NULL) return returnval;

    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */

    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    if (!(textlength = MIN(strlen(text),length))) {
    	return returnval;
    }

    start = 0;
    for (;;) {
	
	for (end=start;end<textlength;++end)
	    if (text[end] == '\n') {
	    	break;
	    }
	
	toklen = end - start;
	
	len = 0.0;
	for (j=start;j<=end-1;++j)
	    len = len + ft_font->W[(GLubyte)text[j]];
	
    	if (len > returnval.width)
	    returnval.width = len;	

    	++i;
	
	if (end >= textlength - 1) break;
	
	start = end + 1;
    }
    /* i should be atleast 1 if we get here...*/
    returnval.height = ft_font->h + (i-1)*ft_font->linespacing;
    
    return returnval;
}

fontbounds printsize(font_data *ft_font, const char *fmt, ...)
{
    fontbounds returnval;
    
    returnval.width=0.0;
    returnval.height=0.0;
    
    if (ft_font == NULL) return returnval;

    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */

    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    return nprintsize(ft_font, bufsize, text);
}

bool render_text(font_data *ft_font, const char *text, string_tex_t *string_tex)
{
    if (!(ft_font)) return false;
    if (!(ft_font->ttffont)) return false;
    if (!(string_tex)) return false;
        
    SDL_Color white = { 0xFF, 0xFF, 0xFF, 0x00 };
    SDL_Color *forecol;
    SDL_Surface *glyph = NULL;
    forecol = &white;
    GLenum gl_error;
	
    glyph = TTF_RenderText_Blended( ft_font->ttffont, text, *forecol );
	
    if(glyph) {
	
    	glGetError();
    	string_tex->texture = SDL_GL_LoadTexture(glyph, &(string_tex->texcoords));
    	if ( (gl_error = glGetError()) != GL_NO_ERROR )
    	    printf("Warning: Couldn't create texture: 0x%x\n", gl_error);
	    
    	string_tex->width = glyph->w;
    	string_tex->height = glyph->h;

    	SDL_FreeSurface(glyph);
    
    } else return false;
    
    return true;
}

bool draw_text(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *text, bool savetex, string_tex_t *string_tex, bool onHUD)
{
    return draw_text_fraq(ft_font, color, XALIGN, YALIGN, x, y, text, 0.0f, 1.0f, 0.0f, 1.0f, savetex, string_tex, onHUD);
}

bool draw_text_fraq(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *text
    	    	    , float xstart
    	    	    , float xstop
    	    	    , float ystart
    	    	    , float ystop
		    , bool savetex, string_tex_t *string_tex, bool onHUD)
{
    bool remove_tex = false;    	
    if (!(ft_font)) return false;
    if (!(ft_font->ttffont)) return false;
        
    if (!string_tex) {
    	remove_tex = true;
    	string_tex = malloc(sizeof(string_tex_t));
    }
    
    if (render_text(ft_font,text,string_tex)) {
    
    	disp_text_fraq(string_tex, color, XALIGN, YALIGN, x, y, xstart, xstop, ystart, ystop, onHUD);
        
    	if (!savetex || remove_tex) {
    	    glDeleteTextures(1,&(string_tex->texture));/* IMPORTANT (?) */
    	    string_tex->texture = 0;
    	}
    }
    
    if (remove_tex) {
    	free(string_tex);
    	string_tex = NULL;
    }
    
    return true;
}

void disp_text(string_tex_t *string_tex, int color, int XALIGN, int YALIGN, int x, int y, bool onHUD)
{
    disp_text_fraq(string_tex, color, XALIGN, YALIGN, x, y, 0.0f, 1.0f, 0.0f, 1.0f, onHUD);
}

void disp_text_fraq(string_tex_t *string_tex, int color, int XALIGN, int YALIGN, int x, int y
    	    	    , float xstart
    	    	    , float xstop
    	    	    , float ystart
    	    	    , float ystop
    	    	    , bool onHUD)
{
    if (!(string_tex)) return;
    set_alphacolor(color);
    glBindTexture(GL_TEXTURE_2D, string_tex->texture);
    
    x -= string_tex->width/2.0f*XALIGN;
    y -= string_tex->height/2.0f*YALIGN;
    
    if (onHUD) pushScreenCoordinateMatrix();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_STRIP);
    	    glTexCoord2f(xstart*string_tex->texcoords.MaxX  , ystop*string_tex->texcoords.MaxY	);
	    glVertex2i( x + xstart*string_tex->width	    , y  + ystart*string_tex->height 	);

     	    glTexCoord2f(xstop*string_tex->texcoords.MaxX   , ystop*string_tex->texcoords.MaxY	);
	    glVertex2i( x + xstop*string_tex->width	    , y  + ystart*string_tex->height 	);

    	    glTexCoord2f(xstart*string_tex->texcoords.MaxX  , ystart*string_tex->texcoords.MaxY );
	    glVertex2i( x + xstart*string_tex->width	    , y + ystop*string_tex->height	); 

   	    glTexCoord2f(xstop*string_tex->texcoords.MaxX   , ystart*string_tex->texcoords.MaxY );
	    glVertex2i( x + xstop*string_tex->width 	    , y + ystop*string_tex->height	); 
    glEnd();
    glDisable(GL_TEXTURE_2D);
    if (onHUD) pop_projection_matrix();
}

void free_string_texture(string_tex_t *string_tex)
{
    if (string_tex) {
    	glDeleteTextures(1,&(string_tex->texture));
    	string_tex->texture = 0;
    }
}

void print(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *text, bool onHUD)
{
    unsigned int i=0,j,textlength;
    fontbounds returnval,dummy;
    float xoff = 0.0,yoff = 0.0;
    int start,end,toklen;
    int X,Y;
    
    returnval.width = 0.0;
    returnval.height = 0.0;

    if (!(textlength = MIN(strlen(text),length))) return;
    
    GLuint font=ft_font->list_base;

    returnval = nprintsize(ft_font,length,text);
    
    yoff = (returnval.height/2.0f)*((float)YALIGN) - ft_font->h;

    glListBase(font);

    if (onHUD) pushScreenCoordinateMatrix();					
    
    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

    glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);

    /* This is where the text display actually happens.
     * For each line of text we reset the modelview matrix
     * so that the line's text will start in the correct position.
     * Notice that we need to reset the matrix, rather than just translating
     * down by h. This is because when each character is
     * draw it modifies the current matrix so that the next character
     * will be drawn immediatly after it. 
     */
    /* make sure not to use mytok until we are done!!! */
    start = 0;
    for (;;) {
	
	for (end=start;end<textlength;++end)
	    if (text[end] == '\n') {
	    	break;
	    }
	
	toklen = end - start;
	
	dummy.width = 0.0;
	for (j=start;j<=end-1;++j)
	    dummy.width = dummy.width + ft_font->W[(GLubyte)text[j]];
	
	xoff = - (dummy.width/2.0f)*((float)XALIGN);

    	glPushMatrix();
    	glLoadIdentity();
	
    	X = (int)(x + xoff);
    	Y = (int)(y - ft_font->linespacing*i + yoff);
	
    	if (color) set_alphacolor(color);
	if (onHUD) glTranslatef(X,Y,0);
	else glTranslatef(X*scale,Y*scale,0);
    	glMultMatrixf(modelview_matrix);

    	glCallLists(toklen, GL_UNSIGNED_BYTE, (GLubyte *) &text[start]);
		
    	glPopMatrix();
   	
	++i;
	
	if (end >= textlength - 1) break;
	
	start = end + 1;
    }
    glPopAttrib();		

    if (onHUD) pop_projection_matrix();

}

void mapnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt,...)
{
    unsigned int textlength;
    
    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */
    
    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    if (!(textlength = MIN(strlen(text),length))) {
    	return;
    }

    if (ft_font == NULL) return;

    print(ft_font, color, XALIGN, YALIGN, x, y, length, text, false);
}

void HUDnprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, int length, const char *fmt, ...)
{
    unsigned int textlength;
    
    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */
    
    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    if (!(textlength = MIN(strlen(text),length))) {
    	return;
    }
    
    if (ft_font == NULL) return;

    print( ft_font, color, XALIGN, YALIGN, x, y, length, text, true);
}

void mapprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *fmt,...)
{
    unsigned int textlength;
    
    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */
    
    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    if (!(textlength = strlen(text))) {
    	return;
    }

    if (ft_font == NULL) return;

    print(ft_font, color, XALIGN, YALIGN, x, y, bufsize, text, false);
}

void HUDprint(font_data *ft_font, int color, int XALIGN, int YALIGN, int x, int y, const char *fmt, ...)
{
    unsigned int textlength;
    
    char		text[bufsize];  /* Holds Our String */
    va_list		ap; 	    /* Pointer To List Of Arguments */
    
    if (fmt == NULL)	    	    /* If There's No Text */
    	*text=0;    	    	    /* Do Nothing */
    else {
    	va_start(ap, fmt);  	    /* Parses The String For Variables */
    	vsnprintf(text, bufsize, fmt, ap);    /* And Converts Symbols To Actual Numbers */
    	va_end(ap); 	    	    /* Results Are Stored In Text */
    }
    if (!(textlength = strlen(text))) {
    	return;
    }
    
    if (ft_font == NULL) return;

    print( ft_font, color, XALIGN, YALIGN, x, y, bufsize, text, true);
}
