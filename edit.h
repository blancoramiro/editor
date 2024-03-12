/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#define INITIAL_CHARS_BUFFER_SIZE 2000000
#define INITIAL_GRID_PARAS 60

#define BLINKS_LIMIT 7
#define BLINKS_START 0

#define LINE_ADD 1
#define LINE_REMOVE -1

#define SCROLL_BAR_WIDTH 15

#define PARA_BUFFER_SIZE 1024
#define PARA_BUFFER_GROWTH 256

//#define SHOW_FPS
#define FPS_AVG_COUNT 1

typedef struct
{
        unsigned int width, height;
}
W_SIZE;

typedef struct paragraph_
{
	struct paragraph_* next;
	struct paragraph_* prev;
	GLfloat* buffer;
	unsigned int lines_count;
	unsigned int buffer_pos;
	unsigned int buffer_count;
	GLfloat* gap;
	unsigned int gap_pos;
	unsigned int gap_count;

}
PARA;

typedef struct
{
        GLfloat width, height, cell_width, cell_height;
        unsigned int full_size;
        GLfloat x_offset, y_offset;
}
GRID;

void cursor_reset(void);

void get_top_paragraph_scroll(void);

void update_chars_tex(void);

void load_selection_tex(void);
