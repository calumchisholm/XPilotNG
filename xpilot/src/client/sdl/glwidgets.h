#ifndef GLWIDGETS_H
#define GLWIDGETS_H

#include "SDL.h"
#include "text.h"

/****************************************************/
/* BEGIN: Main GLWidget stuff	    	    	    */
/****************************************************/
/* Basically the init function will set default width, height in bounds
 * Then the caller should reshape and position the widget defaults to 0,0
 * (top left atm, EVEN sub-widgets so you MUST use the SetBounds_GLWidget
 * function)
 */
/* if this structure is changed, make sure that the generic functions below still work! */
typedef struct glwidget_struct GLWidget;
struct glwidget_struct {
    int     	    WIDGET;
    void    	    *wid_info;

    SDL_Rect	    bounds; /* atm this really is 'inner bounds' which
    	    	    	     * the children aren't allowed to exceed
			     */
    void    	    (*Draw)( GLWidget *widget );
    
    void    	    (*Close)( GLWidget *widget );
    void    	    (*SetBounds)( GLWidget *widget, SDL_Rect *b );
    
    void    	    (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data );
    void    	    *buttondata;
    void    	    (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data );
    void    	    *motiondata;
    void    	    (*hover)( int over, Uint16 x , Uint16 y , void *data );
    void    	    *hoverdata;

    GLWidget	    **list;
    GLWidget	    *children;
    GLWidget	    *next;   /* use to build widget lists */
};

GLWidget *Init_EmptyBaseGLWidget( void );
/*GLWidget *Init_BaseGLWidget( int WIDGET, void *wid_info, SDL_Rect bounds,
    	    	    	    void (*Draw)( GLWidget *widget ), void (*Close)( GLWidget *widget ),
			    void (*SetBounds)( GLWidget *widget, SDL_Rect *b ),
			    void (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data ), void *buttondata,
			    void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data ), void *motiondata,
			    void (*hover)( int over, Uint16 x , Uint16 y , void *data ), void *hoverdata,
			    GLWidget *children, GLWidget *next
			     );*/

extern GLWidget *MainWidget;

/* Two Methods Needed for widget management */
/* new types need to implement theese methods */
    
/* should free any resources committed by the init_foo function */
void Close_Widget ( GLWidget **widget );
void Close_WidgetTree ( GLWidget **widget );
/* to reshape the widget, and automagically reshape and place sub-widgets */
void SetBounds_GLWidget(GLWidget *wid, SDL_Rect *b );
/* Initializes the appropriate config widget (if implemented), returns NULL otherwise */
GLWidget *Init_OptionWidget( font_data *font, xp_option_t *opt );

bool AppendGLWidgetList( GLWidget **list, GLWidget *widget );
void PrependGLWidgetList( GLWidget **list, GLWidget *widget );
bool DelGLWidgetListItem( GLWidget **list, GLWidget *widget );

void DrawGLWidgets( void );
GLWidget *FindGLWidget( Uint16 x,Uint16 y );

/****************************************************/
/* END: Main GLWidget stuff 	    	    	    */
/****************************************************/

/****************************************************/
/* widget-specific stuff is below   	    	    */
/****************************************************/

/***********************/
/* Begin:  ArrowWidget */
/***********************/
/* Basically a triangle that stays lit while mousebutton 1 is down
 * And each time it is drawn lit, it calls (*action).
 * Button two causes it set 'tap' and calls (*action),
 * (*action) needs to reset it for it to work again. (still haven't
 * decided whether automatic reset is better)
 */
#define ARROWWIDGET 0
typedef enum {RIGHTARROW,UPARROW,LEFTARROW,DOWNARROW} ArrowWidget_dir_t;
typedef struct {
    ArrowWidget_dir_t	direction;
    bool    	    	press;/*this is set/unset automagically (set:call action each draw)*/
    bool    	    	tap;/*action needs to clear this (action called once)*/
    bool    	    	locked;/*Won't call action for any reason*/
    void    	    	(*action)(void *data);
    void    	    	*actiondata;
} ArrowWidget;
GLWidget *Init_ArrowWidget( ArrowWidget_dir_t direction, int width, int height,
    	    	    	    void (*action)( void *data ), void *actiondata );
/*********************/
/* End:  ArrowWidget */
/*********************/

/***********************/
/* Begin: SlideWidget  */
/***********************/
#define SLIDEWIDGET 1
typedef struct {
    bool    sliding;/*Don't slide*/
    bool    locked;/*Don't slide*/
    void    (*release)( void *releasedata );
    void    *releasedata;
} SlideWidget;
GLWidget *Init_SlideWidget( bool locked,
    	     void (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data ), void *motiondata,
	     void (*release)( void *releasedata),void *releasedata );
/*********************/
/* End: SlideWidget  */
/*********************/

/***************************/
/* Begin: ScrollbarWidget  */
/***************************/
typedef enum {SB_VERTICAL, SB_HORISONTAL} ScrollWidget_dir_t;
/* note 0.0 <= pos && pos + size <= 1.0 */
#define SCROLLBARWIDGET 2
typedef struct {
    GLWidget	    	*slide;
    GLfloat 	    	pos;
    GLfloat 	    	size;
    Sint16  	    	oldmoves;
    ScrollWidget_dir_t	dir;
    void    	    (	*poschange)( GLfloat pos , void *poschangedata );
    void    	    	*poschangedata;
} ScrollbarWidget;
GLWidget *Init_ScrollbarWidget( bool locked, GLfloat pos, GLfloat size,ScrollWidget_dir_t dir,
    	    	    	    	void (*poschange)( GLfloat pos , void *data), void *data );
/*************************/
/* End:  ScrollbarWidget */
/*************************/

/**********************/
/* Begin: LabelWidget */
/**********************/
#define LABELWIDGET 3
typedef struct {
    string_tex_t    tex;
    int     	    *bgcolor;
    int     	    *fgcolor;
} LabelWidget;
GLWidget *Init_LabelWidget( const char *text , int *bgcolor, int *fgcolor );
/********************/
/* End: LabelWidget */
/********************/

/***********************************/
/* Begin: LabeledRadiobuttonWidget */
/***********************************/
#define LABELEDRADIOBUTTONWIDGET 4
typedef struct {
    bool    	    state;
    string_tex_t    *ontex;
    string_tex_t    *offtex;
    void    	    (*action)( bool state, void *actiondata );
    void    	    *actiondata;
} LabeledRadiobuttonWidget;
/* TODO : add some abstraction layer to init function */
GLWidget *Init_LabeledRadiobuttonWidget( string_tex_t *ontex, string_tex_t *offtex,
    	    	    	    	    	void (*action)(bool state, void *actiondata),
					void *actiondata, bool start_state);
/*********************************/
/* End: LabeledRadiobuttonWidget */
/*********************************/

/****************************/
/* Begin: BoolChooserWidget */
/****************************/
#define BOOLCHOOSERWIDGET 5
typedef struct {
    xp_option_t     *opt;
    GLWidget	    *buttonwidget;
    string_tex_t    nametex;
} BoolChooserWidget;

GLWidget *Init_BoolChooserWidget( font_data *font, xp_option_t *opt );
/**************************/
/* End: BoolChooserWidget */
/**************************/

/***************************/
/* Begin: IntChooserWidget */
/***************************/
#define INTCHOOSERWIDGET 6
typedef struct {
    string_tex_t    nametex;
    xp_option_t     *opt;
    int     	    valuespace;
    string_tex_t    valuetex;
    font_data 	    *font;
    GLWidget	    *leftarrow;
    GLWidget	    *rightarrow;
    int     	    direction;
} IntChooserWidget;

GLWidget *Init_IntChooserWidget( font_data *font, xp_option_t *opt );
/*************************/
/* End: IntChooserWidget */
/*************************/

/******************************/
/* Begin: DoubleChooserWidget */
/******************************/
#define DOUBLECHOOSERWIDGET 7
typedef struct {
    string_tex_t    nametex;
    xp_option_t     *opt;
    int     	    valuespace;
    string_tex_t    valuetex;
    font_data 	    *font;
    GLWidget	    *leftarrow;
    GLWidget	    *rightarrow;
    int     	    direction;
} DoubleChooserWidget;

GLWidget *Init_DoubleChooserWidget( font_data *font, xp_option_t *opt );
/****************************/
/* End: DoubleChooserWidget */
/****************************/

/**********************/
/* Begin: RadarWidget */
/**********************/
#define RADARWIDGET 8

extern GLWidget *Init_RadarWidget( void );
/********************/
/* End: RadarWidget */
/********************/

/**************************/
/* Begin: ScorelistWidget */
/**************************/
#define SCORELISTWIDGET 9

extern GLWidget *Init_ScorelistWidget( void );
/************************/
/* End: ScorelistWidget */
/************************/

/**********************/
/* Begin: MainWidget  */
/**********************/
#define MAINWIDGET 10
typedef struct {
    GLWidget	*confmenu;
    font_data	*font;
} WrapperWidget;

GLWidget *Init_MainWidget( font_data *font );
/*******************/
/* End: MainWidget */
/*******************/

/**************************/
/* Begin: ConfMenuWidget  */
/**************************/
#define CONFMENUWIDGET 11
typedef struct {
    int     	list_height;
    GLWidget	*scrollbar;
} ConfMenuWidget;

GLWidget *Init_ConfMenuWidget( font_data *font, Uint16 x, Uint16 y );
/***********************/
/* End: ConfMenuWidget */
/***********************/

#endif
