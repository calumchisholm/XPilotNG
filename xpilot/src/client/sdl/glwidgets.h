#ifndef GLWIDGETS_H
#define GLWIDGETS_H

#include "SDL.h"
#include "text.h"
#include "widget_wrappers.h"

/****************************************************/
/* BEGIN: Main GLWidget stuff	    	    	    */
/****************************************************/

/* Basically the init function will set default width, height in bounds
 * Then the caller should reshape and position the widget defaults to 0,0
 * (top left atm, EVEN sub-widgets so you MUST use the SetBounds_GLWidget
 * function)
 */
typedef struct glwidget_struct GLWidget;
struct glwidget_struct {
    int     	    WIDGET;
    void    	    *wid_info;

    SDL_Rect	    bounds;
    void    	    (*Draw)(widget_list_t *LI);
    /* Keeping theese members atm since you might some time want
     * some enveloping widget, which does not register on the GUI
     * in a normal way (may go away soon)
     */
    void    	    (*GuiReg)(widget_list_t *LI);/* remove once the widget list is used to catch events */
    void    	    (*GuiUnReg)(widget_list_t *LI);/* remove once the widget list is used to catch events */
    
    void    	    (*Close)(GLWidget *widget);
    void    	    (*SetBounds)(GLWidget *widget, SDL_Rect *b);
    
    void    	    (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
    void    	    *buttondata;
    void    	    (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data);
    void    	    *motiondata;
    void    	    (*hover)( int over, Uint16 x , Uint16 y , void *data);
    void    	    *hoverdata;

    guiarea_t	    *guiarea;/* remove once bounds is used instead */
    widget_list_t   *listPtr;/* remove once the widget list is used to catch events */
    GLWidget	    *next;   /* use to build widget lists */
};

/* Two Methods Needed for widget management */
/* new types need to edit theese methods */
    
/* should free any resources committed by the init_foo function */
void Close_Widget (GLWidget *widget);
/* to reshape the widget, and automagically reshape and place sub-widgets */
void SetBounds_GLWidget(GLWidget *wid, SDL_Rect *b);
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
 * decided whether automati reset is better)
 */
#define ARROWWIDGET 0

typedef enum {RIGHTARROW,UPARROW,LEFTARROW,DOWNARROW} ArrowWidget_dir_t;
typedef struct {
    ArrowWidget_dir_t direction;
    bool    	    press;/*this is set/unset automagically (set:call action each draw)*/
    bool    	    tap;/*action needs to clear this (action called once)*/
    bool    	    locked;/*Won't call action for any reason*/
    void    	    (*action)(void *data);
    void    	    *actiondata;
} ArrowWidget;
GLWidget *Init_ArrowWidget( widget_list_t *list, ArrowWidget_dir_t direction, int width, int height,
    	    	    	    void (*action)( void *data ), void *actiondata);
/*********************/
/* End:  ArrowWidget */
/*********************/

/***********************************/
/* Begin: LabeledRadiobuttonWidget */
/***********************************/
#define LABELEDRADIOBUTTONWIDGET 1
typedef struct {
    bool    	    state;
    string_tex_t    *ontex;
    string_tex_t    *offtex;
    void    	    (*action)(bool state, void *actiondata);
    void    	    *actiondata;
} LabeledRadiobuttonWidget;
/* TODO : add some abstraction layer to init function */
GLWidget *Init_LabeledRadiobuttonWidget(widget_list_t *list, string_tex_t *ontex, string_tex_t *offtex, void (*action)(bool state, void *actiondata), void *actiondata, bool start_state);
/*********************************/
/* End: LabeledRadiobuttonWidget */
/*********************************/

/****************************/
/* Begin: BoolChooserWidget */
/****************************/
#define BOOLCHOOSERWIDGET 2
typedef struct {
    bool    	    *value;
    GLWidget	    *buttonwidget;
    string_tex_t    nametex;
    void    	    (*action)(void *data);
    void    	    *actiondata;
} BoolChooserWidget;

GLWidget *Init_BoolChooserWidget(widget_list_t *list, font_data *font, const char *name, bool *value,void (*action)(void *data),void *actiondata);
/**************************/
/* End: BoolChooserWidget */
/**************************/

/***************************/
/* Begin: IntChooserWidget */
/***************************/
#define INTCHOOSERWIDGET 3
typedef struct {
    string_tex_t    nametex;
    int     	    *value;
    int     	    *min;
    int     	    *max;
    int     	    valuespace;
    string_tex_t    valuetex;
    font_data 	    *font;
    GLWidget	    *leftarrow;
    GLWidget	    *rightarrow;
    int     	    direction;
    void    	    (*action)(void *data);
    void    	    *actiondata;
} IntChooserWidget;

GLWidget *Init_IntChooserWidget(widget_list_t *list, font_data *font, const char *name, int *value, int *min, int *max,void (*action)(void *data),void *actiondata);
/*************************/
/* End: IntChooserWidget */
/*************************/

/******************************/
/* Begin: DoubleChooserWidget */
/******************************/
#define DOUBLECHOOSERWIDGET 4
typedef struct {
    string_tex_t    nametex;
    double  	    *value;
    double  	    *min;
    double  	    *max;
    int     	    valuespace;
    string_tex_t    valuetex;
    font_data 	    *font;
    GLWidget	    *leftarrow;
    GLWidget	    *rightarrow;
    int     	    direction;
    void    	    (*action)(void *data);
    void    	    *actiondata;
} DoubleChooserWidget;

GLWidget *Init_DoubleChooserWidget(widget_list_t *list, font_data *font, const char *name, double *value, double *min, double *max,void (*action)(void *data),void *actiondata);
/****************************/
/* End: DoubleChooserWidget */
/****************************/

#endif
