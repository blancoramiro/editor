/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#define INITIAL_CHARS_BUFFER_SIZE 2000000
#define INITIAL_GRID_PARAS 300

#define BLINKS_LIMIT 7
#define BLINKS_START 0

#define LINE_ADD 1
#define LINE_REMOVE -1

#define SCROLL_BAR_WIDTH 20

#define PARA_BUFFER_SIZE 1024
#define PARA_BUFFER_GROWTH 256

#define FPS_AVG_COUNT 1

typedef struct
{
        int width, height;
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
	unsigned int gap_del;

}
PARA;

typedef struct
{
        GLfloat width, height, cell_width, cell_height;
}
GRID;

void cursor_reset(void);

void get_top_paragraph_scroll(void);
