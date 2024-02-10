/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

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
        GLfloat y_offset;
}
GRID;

void cursor_reset(void);

void get_top_paragraph_scroll(void);

void update_chars_tex(void);

void load_selection_tex(void);
