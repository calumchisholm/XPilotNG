typedef enum {
    IMG_STATE_UNINITIALIZED,
    IMG_STATE_ERROR,
    IMG_STATE_READY
} image_state_e;

/*
 * This structure holds the information needed to paint an image with OpenGL.
 * One image may contain multiple frames that represent the same object in
 * different states. If rotate flag is true, the image will be rotated
 * when it is created to generate num_frames-1 of new frames.
 * Width is the cumulative width of all frames. Frame_width is the width
 * of any single frame. Because OpenGL requires the dimensions of the images
 * to be powers of 2, data_width and data_height are the nearest powers of 2
 * corresponding to width and height respectively.
 */
typedef struct {
    int             name;         /* OpenGL texture "name" */
    char            *filename;    /* the name of the image file */
    int             num_frames;   /* the number of frames */
    bool            rotate;       /* should this image be rotated */
    image_state_e   state;        /* the state of the image */
    int             width;        /* width of the whole image */
    int             height;       /* height of the whole image */
    int             data_width;   /* width of the image data */
    int             data_height;  /* height of the image data */
    int             frame_width;  /* width of one image frame */
    unsigned int    *data;        /* the image data */
} image_t;

#define IMG_HOLDER_FRIEND 0
#define IMG_HOLDER_ENEMY  1
#define IMG_BALL          2
#define IMG_SHIP_SELF     3
#define IMG_SHIP_FRIEND   4
#define IMG_SHIP_ENEMY    5
#define IMG_BULLET        6
#define IMG_BULLET_OWN    7
#define IMG_BASE_DOWN     8
#define IMG_BASE_LEFT     9
#define IMG_BASE_UP       10
#define IMG_BASE_RIGHT    11
#define IMG_FUELCELL      12
#define IMG_FUEL          13
#define IMG_ALL_ITEMS     14
#define IMG_CANNON_DOWN   15
#define IMG_CANNON_LEFT   16
#define IMG_CANNON_UP     17
#define IMG_CANNON_RIGHT  18
#define IMG_SPARKS        19
#define IMG_PAUSED        20
#define IMG_REFUEL        21
#define IMG_WORMHOLE      22
#define IMG_MINE_TEAM     23
#define IMG_MINE_OTHER    24
#define IMG_CONCENTRATOR  25
#define IMG_PLUSGRAVITY   26
#define IMG_MINUSGRAVITY  27
#define IMG_CHECKPOINT	  28
#define IMG_METER	  29
#define IMG_ASTEROIDCONC  30
#define IMG_WALL_TEXTURE  31
#define IMG_DECOR_TEXTURE 32
#define IMG_BALL_TEXTURE  33

int Images_init(void);
void Images_cleanup(void);
int Bitmap_add(char *filename, int count, bool scalable);
void Image_paint(int index, int x, int y, int frame);
void Image_paint_area(int index, int x, int y, int frame, irec *r);
image_t *Image_get(int index);
