#ifndef GLWIDGETS_H
#define GLWIDGETS_H

#include "SDL.h"
#include "text.h"
#include "widget_wrappers.h"

/* Basically the init function will set default width, height in bounds
 * Then the caller should reshape and position the widget
 */
typedef struct {
    int     	    WIDGET;
    SDL_Rect	    bounds;
    void    	    *wid_info;
    void    	    (*Draw)(void *LI);
    void    	    (*GuiReg)(void *LI);
    void    	    (*GuiUnReg)(void *LI);
    void    	    (*button)( Uint8 button, Uint8 state , Uint16 x , Uint16 y, void *data);
    void    	    *buttondata;
    void    	    (*motion)( Sint16 xrel, Sint16 yrel, Uint16 x, Uint16 y, void *data);
    void    	    *motiondata;
    void    	    (*hover)( int over, Uint16 x , Uint16 y , void *data);
    void    	    *hoverdata;
    guiarea_t	    *guiarea;
    widget_list_t   *listPtr;
} GLWidget;
/* GENERIC */
void GuiReg(void *LI);
void GuiUnReg(void *LI);
void Close_Widget (GLWidget *widget);
void SetBounds_GLWidget(GLWidget *wid, SDL_Rect *b);/* new types need to edit this */

/* Below are typedefs and init functions for widgets */ 

/* Begin:  ArrowWidget*/
#define ARROWWIDGET 0

typedef enum {RIGHTARROW,UPARROW,LEFTARROW,DOWNARROW} ArrowWidget_dir_t;
typedef struct {
    void    	    (*action)(void *data);
    void    	    *actiondata;
    ArrowWidget_dir_t direction;
    bool    	    press;/*this is set/unset automagically (set:call action each draw)*/
    bool    	    tap;/*action needs to clear this (action called once)*/
    bool    	    locked;/*Won't call action for any reason*/
} ArrowWidget;
GLWidget *Init_ArrowWidget( widget_list_t *list, ArrowWidget_dir_t direction, int width, int height,
    	    	    	    void (*action)( void *data ), void *actiondata);
/* End:  ArrowWidget*/

/* Begin:  LabeledRadiobuttonWidget*/
#define LABELEDRADIOBUTTONWIDGET 1
typedef struct {
    void    	    (*action)(bool state, void *actiondata);
    void    	    *actiondata;
    bool    	    state;
    string_tex_t    *ontex;
    string_tex_t    *offtex;
} LabeledRadiobuttonWidget;

GLWidget *Init_LabeledRadiobuttonWidget(widget_list_t *list, string_tex_t *ontex, string_tex_t *offtex, void (*action)(bool state, void *actiondata), void *actiondata, bool start_state);
/* End: LabeledRadiobuttonWidget */

/* Begin: IntChooserWidget */
#define INTCHOOSERWIDGET 2
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
} IntChooserWidget;

GLWidget *Init_IntChooserWidget(widget_list_t *list, const char *name, font_data *font, int *value, int *min, int *max);
/* End: IntChooserWidget */

/* Begin: DoubleChooserWidget */
#define DOUBLECHOOSERWIDGET 3
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
} DoubleChooserWidget;

GLWidget *Init_DoubleChooserWidget(widget_list_t *list, const char *name, font_data *font, double *value, double *min, double *max);
/* End: IntChooserWidget */

#endif
