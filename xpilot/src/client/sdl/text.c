#include <string.h>
#include <stdarg.h>
#include <GL/gl.h>
#include "SDL.h"

/*FreeType Headers*/
#ifdef HAVE_FREETYPE2
    #include <ft2build.h>
    #include <freetype/freetype.h>
    #include <freetype/ftglyph.h>
    #include <freetype/ftoutln.h>
    #include <freetype/fttrigon.h>
#endif

#include "text.h"

#define NUM_TEXTURES 2

GLuint  texture[NUM_TEXTURES]; /* Storage For Our Font Texture             */
int LoadBMP(font_data *ft_font, const char * fname);
void pushScreenCoordinateMatrix(void);
void pop_projection_matrix(void);
/*char *mytok(char *s, const char *delim);

static char *mytok(char *s, const char *delim)
{
    static char buf[1024];
    strncpy(buf,s,1023);
    
    
}*/

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
	    ft_font->w = TextureImage->w/16;
	    /* Set the status to true */
	    Status = 1;

	    /* Create The Texture */
	    glGenTextures( NUM_TEXTURES, &texture[0] );

	    /* Load in texture 1 */
	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage->w,
			  TextureImage->h, 0, GL_BGR,
			  GL_UNSIGNED_BYTE, TextureImage->pixels );
	    
	    /* Nearest Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        }

    /* Free up any memory we may have used */
    if ( TextureImage )
	SDL_FreeSurface( TextureImage );

    return Status;
}

#ifdef HAVE_FREETYPE2
int next_p2 ( int a );
int make_dlist ( FT_Face face, char ch, GLuint list_base, GLuint * tex_base, GLuint *char_width);
int FTinit(font_data *ft_font, const char * fname, unsigned int w, unsigned int h);

int next_p2 ( int a )
{
	int rval=1;
	while(rval<a) rval<<=1;
	return rval;
}

/* Create a display list coresponding to the give character. */
int make_dlist ( FT_Face face, char ch, GLuint list_base, GLuint * tex_base, GLuint *char_width)
{
    
    int i,j;
    GLubyte *expanded_data;
    FT_Bitmap bitmap;
    
    /* The first thing we do is get FreeType to render our character
     * into a bitmap.  This actually requires a couple of FreeType commands:
     */
 
    /*Load the Glyph for our character.*/
    if(FT_Load_Glyph( face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT )) {
	error("FT_Load_Glyph failed");
	return 1;
    }

    /*Move the face's glyph into a Glyph object.*/
    FT_Glyph glyph;
    if(FT_Get_Glyph( face->glyph, &glyph )) {
	error("FT_Get_Glyph failed");
	return 2;
    }

    /*Convert the glyph to a bitmap.*/
    FT_Glyph_To_Bitmap( &glyph, ft_render_mode_normal, 0, 1 );
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    /*This reference will make accessing the bitmap easier*/
    bitmap=bitmap_glyph->bitmap;

    /* Use our helper function to get the widths of
     * the bitmap data that we will need in order to create
     * our texture.
     */
    int width = next_p2( bitmap.width );
    int height = next_p2( bitmap.rows );

    /* Allocate memory for the texture data. */
    /*GLubyte* expanded_data = new GLubyte[ 2 * width * height];*/
    expanded_data = (GLubyte *) malloc( 2 * width * height * sizeof(GLubyte));

    /* Here we fill in the data for the expanded bitmap.
     * Notice that we are using two channel bitmap (one for
     * luminocity and one for alpha), but we assign
     * both luminocity and alpha to the value that we
     * find in the FreeType bitmap. 
     * We use the ?: operator so that value which we use
     * will be 0 if we are in the padding zone, and whatever
     * is the the Freetype bitmap otherwise.
     */
    for(j=0; j <height;j++) {
	for(i=0; i < width; i++){
	    expanded_data[2*(i+j*width)]= expanded_data[2*(i+j*width)+1] = 
			(i>=bitmap.width || j>=bitmap.rows) ?
			0 : bitmap.buffer[i + bitmap.width*j];
	}
    }


    /*Now we just setup some texture paramaters.*/
    glBindTexture( GL_TEXTURE_2D, tex_base[(GLubyte)ch]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    /* Here we actually create the texture itself, notice
     * that we are using GL_LUMINANCE_ALPHA to indicate that
     * we are using 2 channel data.
     */
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height,
	  0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data );

    /*With the texture created, we don't need to expanded data anymore*/
    /*delete [] expanded_data;*/
    free(expanded_data);

    /*So now we can create the display list*/
    glNewList(list_base+ch,GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D,tex_base[(GLubyte)ch]);

    /* first we need to move over a little so that
     * the character has the right amount of space
     * between it and the one before it.
     */
    glTranslatef(bitmap_glyph->left,0,0);

    /* Now we move down a little in the case that the
     * bitmap extends past the bottom of the line 
     * (this is only true for characters like 'g' or 'y'.
     */
    glPushMatrix();
    glTranslatef(0,bitmap_glyph->top-bitmap.rows,0);

    /* Now we need to account for the fact that many of
     * our textures are filled with empty padding space.
     * We figure what portion of the texture is used by 
     * the actual character and store that information in 
     * the x and y variables, then when we draw the
     * quad, we will only reference the parts of the texture
     * that we contain the character itself.
     */
    float	x=(float)bitmap.width / (float)width,
		y=(float)bitmap.rows / (float)height;

    *char_width = face->glyph->advance.x >> 6;
    /* Here we draw the texturemaped quads.
     * The bitmap that we got from FreeType was not 
     * oriented quite like we would like it to be,
     * so we need to link the texture to the quad
     * so that the result will be properly aligned.
     */
    glBegin(GL_QUADS);
        glTexCoord2d(0,0); glVertex2f(0,bitmap.rows);
        glTexCoord2d(0,y); glVertex2f(0,0);
        glTexCoord2d(x,y); glVertex2f(bitmap.width,0);
        glTexCoord2d(x,0); glVertex2f(bitmap.width,bitmap.rows);
    glEnd();
    glPopMatrix();
    glTranslatef(*char_width ,0,0);

    /*Finish the display list*/
    glEndList();
    return 0;
}

/* The init function will create a font of
 * of the height h from the file fname.
 */
int FTinit(font_data *ft_font, const char * fname, unsigned int w, unsigned int h)
{
    unsigned char i;
    
    if (ft_font == NULL) return 1;
    /*Allocate some memory to store the texture ids.*/
    ft_font->textures = (GLuint *) malloc(128 * sizeof(GLuint));

    ft_font->h=h;
    ft_font->w=w; /*atm we have to guess this =( */

    /*Create and initilize a freetype font library.*/
    FT_Library library;
    if (FT_Init_FreeType( &library )) { 
	error("FT_Init_FreeType failed");
	return 2;
    }

    /* The object in which Freetype holds information on a given
     * font is called a "face".
     */
    FT_Face face;

    /* This is where we load in the font information from the file.
     * Of all the places where the code might die, this is the most likely,
     * as FT_New_Face will die if the font file does not exist or is somehow broken.
     */
    if (FT_New_Face( library, fname, 0, &face )) {
	error("FT_New_Face failed (there is probably a problem with your font file)");
	return 3;
    }

    /* For some twisted reason, Freetype measures font size
     * in terms of 1/64ths of pixels.  Thus, to make a font
     * h pixels high, we need to request a size of h*64.
     * (h << 6 is just a prettier way of writting h*64)
     */
    FT_Set_Char_Size( face, ((int)ft_font->w) << 6, ((int)ft_font->h) << 6, 96, 96);
    /*FT_Set_Pixel_Sizes( face, (int)ft_font->w, (int)ft_font->h );*/

    /* Here we ask opengl to allocate resources for
     * all the textures and displays lists which we
     * are about to create. 
     */ 
    ft_font->list_base=glGenLists(128);
    ft_font->char_width=(GLuint *) malloc(128 * sizeof(GLuint));
    glGenTextures( 128, ft_font->textures );

    /*This is where we actually create each of the fonts display lists.*/
    for(i=0;i<128;i++)
	if (make_dlist(face,i,ft_font->list_base,ft_font->textures,&ft_font->char_width[i])) return 3;


    /* We don't need the face information now that the display
     * lists have been created, so we free the assosiated resources.
     */
    FT_Done_Face(face);

    /*Ditto for the library.*/
    FT_Done_FreeType(library);
    
    return 0;
}
#endif

int fontinit(font_data *ft_font, const char * fname, unsigned int w/*this is mostly ballpark I guess*/, unsigned int h)
{
    GLuint loop; /* Loop variable               */
    float cx;    /* Holds Our X Character Coord */
    float cy;    /* Holds Our Y Character Coord */    
    
    if (strlen(fname)<5) return 1; /*not long enough to be a useful name*/
    if (strcasecmp(".bmp",&fname[strlen(fname)-4]) != 0)
    /* if its not a .bmp file lets try freetype if available */
    #ifdef HAVE_FREETYPE2
    	return FTinit(ft_font,fname,w,h);
    #else
    {
    	xpprintf("fontfile has to be foo.bmp\n");
    }
    #endif
    if(!LoadBMP(ft_font,fname))
    	return 1;
    
    /* Creating 256 Display List */
    ft_font->list_base  = glGenLists( 128 );
    ft_font->char_width = (GLuint *) malloc(128 * sizeof(GLuint));
    /* Select Our Font Texture */
    glBindTexture( GL_TEXTURE_2D, texture[0] );

    /* Loop Through All 256 Lists */
    for ( loop = 0; loop < 128; loop++ )
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
    	    ft_font->char_width[loop] = ft_font->w;
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
    glDeleteLists(ft_font->list_base,128);
    if (ft_font->char_width != NULL)
    	free(ft_font->char_width);
    if (ft_font->textures != NULL) {
    	glDeleteTextures(128,ft_font->textures);
    	free(ft_font->textures);
    }
    ft_font = NULL;
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


fontbounds fontprintsize(font_data *ft_font, const char *fmt, ...)
{
	
    unsigned int bufsize = 1024;
    unsigned int i=0,j,textlength;
    float len;
    fontbounds returnval;
    int start,end,toklen;
    
    returnval.width=0.0;
    returnval.height=0.0;
    
    if (ft_font == NULL) return returnval;
     float h=ft_font->h/.63f; 	    /*We make the height about 1.5* that of */

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
    	return returnval;
	error("Someone tried to measure a null string =(");
    }

    start = 0;
    for (;;) {
	
	for (end=start;end<textlength-1;++end)
	    if (text[end] == '\n') {
	    	break;
	    }
	
	toklen = end - start + 1;
	
	len = (end-start +1)*1.0;
	for (j=start;j<=end;++j)
	    len = len + ft_font->char_width[(GLubyte)text[j]];
	
    	if (len > returnval.width)
	    returnval.width = len;	

    	++i;
	
	if (end >= textlength - 1) break;
	
	start = end + 1;
    }
    
    returnval.height = h*i;
    
    return returnval;
}

/* The flagship function of the library - this thing will print
 * out text at window coordinates x,y, using the font ft_font.
 * The current modelview matrix will also be applied to the text.
 * Also it will return the height and width of the text written.
 */ 
fontbounds fontprint(font_data *ft_font, int XALIGN, int YALIGN, float x, float y, const char *fmt, ...)
{
	
    unsigned int bufsize = 1024;
    unsigned int i=0,j,textlength;
    fontbounds returnval,dummy;
    float xoff = 0.0,yoff = 0.0;
    int start,end,toklen;
    
    returnval.width = 0.0;
    returnval.height = 0.0;
    
     if (ft_font == NULL) return returnval;
    /* We want a coordinate system where things coresponding to window pixels.*/

    GLuint font=ft_font->list_base;
    float h=ft_font->h/.63f; 	    /*We make the height about 1.5* that of */

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
    	return returnval;
	error("Someone tried to print a null string =(");
    }
    
    returnval = fontprintsize(ft_font,text);
    
    yoff = (returnval.height/2.0f)*((float)YALIGN) - ft_font->h;

    pushScreenCoordinateMatrix();					
    
    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);	
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	

    glListBase(font);

    float modelview_matrix[16];	
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
	
	for (end=start;end<textlength-1;++end)
	    if (text[end] == '\n') {
	    	break;
	    }
	
	toklen = end - start + 1;
	
	dummy.width = (end-start +1)*1.0;
	for (j=start;j<=end;++j)
	    dummy.width = dummy.width + ft_font->char_width[(GLubyte)text[j]];
	
	xoff = - (dummy.width/2.0f)*((float)XALIGN);

    	glPushMatrix();
    	glLoadIdentity();
		
    	glTranslatef(x + xoff,y - h*i + yoff,0);
    	glMultMatrixf(modelview_matrix);

    	glCallLists(toklen, GL_UNSIGNED_BYTE, (GLubyte *) &text[start]);
		
    	glPopMatrix();

    	++i;
	
	if (end >= textlength - 1) break;
	
	start = end + 1;
    }
    
    glPopAttrib();		

    pop_projection_matrix();
    
    return returnval;
}
