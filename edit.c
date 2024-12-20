/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#include <stdio.h>
#include <stdlib.h>

//#define SHOW_FPS
#define CHAR_GRID

#ifdef SHOW_FPS
#include <time.h>
struct timespec fps_start, fps_stop;
#endif

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include <math.h>
#include <float.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "edit.h"
#include "scroll.h"
#include "gap.h"
#include "linenumber.h"

#include "shaders.c"

GLFWwindow *window;

GLuint  vertex_buffer,
	vertex_buffer_scroll,
	vertex_buffer_linenumber,
	vertex_shader,
	fragment_shader_text,
	fragment_shader_scroll,
	fragment_shader_linenumber,
	program_text,
	program_scroll,
	program_linenumber,
	vpos_location_text,
	vpos_location_scroll,
	vpos_location_linenumber,
	mouse_location,
	grid_location,
	grid_location_linenumber,
	grid_Y_offset_location,
	grid_Y_offset_location_linenumber,
	grid_X_offset_location,
	scroll_bar_location,
	cursor_location,
	cursor_blink_location,
	chars_location,
	selection_location,
	linenumber_location;

GLuint  font_tex,
	char_tex,
	select_tex,
	linenumber_tex;

GLfloat mouse_pos[2];

struct 
{
	unsigned int mouse, shift, start_pos;
}
selection_state = {0, 0, 0};

FILE *file_tex;
char rgba[4];
unsigned char bmp_header[132];
unsigned int bmp_dataPos;
unsigned int bmp_width, bmp_height;
unsigned int bmp_image_size;
unsigned char *bmp_data;

//GRID grid = {0., 0., 40., 38.};
//GRID grid = {0., 0., 7., 10.};
GRID grid = {1., 1., 20., 19.};
//GRID grid = {0., 0., 7., 9., 5., 5.};
//GRID grid = {0., 0., 1., 3.};
unsigned int grid_full_size;
GLfloat grid_x_offset = 5., grid_y_offset = 5.;

SCROLL_BAR scroll_bar = {400., 700.};

double scroll_bar_grab = 0;

W_SIZE window_size = {0, 0};

int fd;

int i, j, k, l, m, n;

double curr_frame_time, last_frame_time, fps_avg_count = 0, fps_avg;
unsigned int fps_int, fps_dec;

unsigned int blink_state = 1, blink_count = 0;

PARA *curr_paragraph = NULL;
PARA paragraphs_head = { NULL, NULL, NULL, 1, 0, 0, NULL, 0, 0 };
PARA **grid_lines = NULL;

PARA *top_paragraph_scroll = NULL;
unsigned int lines_scroll_top_diff = 0;

unsigned int curr_line = 0;

unsigned int paragraphs_count = 1; 
unsigned int lines_count = 1;
GLfloat *line_numbers;
unsigned int top_line_number = 1;
unsigned int lines_decimals_count = 0;

unsigned int lines_scroll = 0;
unsigned int paragraph_cursor = 0;
unsigned int grid_paragraph_count = INITIAL_GRID_PARAS;

unsigned int chars_buffer_size = INITIAL_CHARS_BUFFER_SIZE;

GLfloat *chars_tex;
GLfloat *selection_tex;
GLfloat *linenumbers_tex;

#ifndef __EMSCRIPTEN__
const char *paste_buff;
#endif

static struct
{
    float x, y;
}
SCREEN[6] =
{
        {-1.f,1.f},
        {-1.f,-1.f},
        {1.f,-1.f},
	{-1.f,1.f},
        {1.f,1.f},
        {1.f,-1.f}
},
SCROLL[6] =
{
        {-1.f,1.f},
        {-1.f,-1.f},
        {1.f,-1.f},
        {-1.f,1.f},
        {1.f,1.f},
        {1.f,-1.f}
},
LINECOL[6] =
{
        {-1.f,1.f},
        {-1.f,-1.f},
        {.5f,-1.f},
        {-1.f,1.f},
        {.5f,1.f},
        {.5f,-1.f}
} ;

static void print_to_screen(const char *buff)
{
#ifdef __EMSCRIPTEN__
	emscripten_console_log(buff);
#else
	printf("%s", buff);
#endif
}

static void error_callback(int error, const char *description)
{
	print_to_screen("Error!");
	print_to_screen(description);
}

static inline void load_char_tex(void)
{
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid.width, grid.height, GL_RED, GL_FLOAT, chars_tex);
}

static inline void load_linenumber_tex(void)
{
	glActiveTexture(GL_TEXTURE3); 
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid.width, grid.height, GL_RED, GL_FLOAT, linenumbers_tex);
}

static inline void load_selection_tex(void)
{
	glActiveTexture(GL_TEXTURE2); 
	glBindTexture(GL_TEXTURE_2D, select_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, selection_tex);
}

inline void cursor_reset(void)
{
	blink_count = 0;
	blink_state = 1;
	glUniform1f(cursor_blink_location, 1.);
}

static inline void cursor_position(void)
{

	//float aux[] = {++x, y};
	float aux[] = { paragraph_cursor % (int)grid.width + 1, curr_line };
	glUniform2fv(cursor_location, 1, aux);
}

static inline void update_lines_count(short action)
{

	lines_count += action;
	curr_paragraph->lines_count += action;
}

static inline void update_all_paragraphs_lines_count(void)
{
	PARA *paragraph_i = &paragraphs_head;

	lines_count = 0;

	do
	{
		lines_count += paragraph_i->lines_count = paragraph_i->buffer_count ? ceil(paragraph_i->buffer_count / grid.width) : 1;
		paragraph_i = paragraph_i->next;
	}
	while(paragraph_i);
}

void get_top_paragraph_scroll(void) 
{
	top_line_number = 1;
	top_paragraph_scroll = &paragraphs_head;
	lines_scroll_top_diff = lines_scroll;
	while(lines_scroll_top_diff >= top_paragraph_scroll->lines_count)
	{
		lines_scroll_top_diff -= top_paragraph_scroll->lines_count;
		top_paragraph_scroll = top_paragraph_scroll->next;
		++top_line_number;
	}
}

static void update_grid_lines(void) 
{
	PARA *paragraph_i  = top_paragraph_scroll;
	k = m = n = 0;

	lines_decimals_count = 0;

	for(i = top_line_number + grid.height; i; i /= 10) ++lines_decimals_count; // Set to either grid.height or larger visible line

	i = paragraph_i->lines_count-lines_scroll_top_diff;
	if(i == paragraph_i->lines_count)
		for(j = lines_decimals_count; j > 0; --j ) linenumbers_tex[n++] = ((int)(top_line_number/(j*10))+34.)/256.f;
	else for(j = 0; j <= lines_decimals_count; ++j ) linenumbers_tex[n++] = 0.;
	goto FIRST_LINE_CONT;

	do
	{
		linenumbers_tex[m] = 0.;
FIRST_LINE_CONT:
		grid_lines[m] = paragraph_i;
		line_numbers[m] = top_line_number;
		++m;
		++n;
		--i;
	}
	while(i > 0);

	i = top_line_number;

	while(paragraph_i->next)
	{
		paragraph_i = paragraph_i->next;
		++i;
		j = paragraph_i->lines_count;
		linenumbers_tex[m] = (i+34.)/256.f;
		goto LINE_CONT;

		do
		{
			linenumbers_tex[m] = 0.;
LINE_CONT:
			grid_lines[m] = paragraph_i;
			line_numbers[m] = i;
			++m;
			--j;
		}
		while(j > 0);
	}

	for(;m < grid.height; ++m)
	{
		grid_lines[m] = NULL; //Make sure grid_paragraph_count is updated when array gets resized
		line_numbers[m] = 0;
		linenumbers_tex[m] = 0.;
	}
}

static void update_chars_tex(void) 
{
	PARA *paragraph_i = top_paragraph_scroll;
	k = m = 0;

//	if(paragraph_i->lines_count > lines_scroll_top_diff)
//	{
	n = lines_scroll_top_diff * grid.width;
	i = paragraph_i->buffer_count + paragraph_i->gap_count - n;
	for(j = 0; j < i; ++j) chars_tex[j] = paragraph_i->buffer[n + j];
	goto CARRY_ON_PARAS;
//	}

	do
	{

		if(!paragraph_i->buffer_count) //Empty line
		{
			for(j = 0; j < grid.width; ++j) chars_tex[k + j] = 0.;
			k += grid.width;
		}
		else
		{

			//for(j = 0; j < paragraph_i->buffer_count; ++j) chars_tex[k + j] = paragraph_i->buffer[j];
			//gap
			for(j = 0; j < paragraph_i->gap_pos; ++j) chars_tex[k + j] = paragraph_i->buffer[j];
			for(i = 0; i < paragraph_i->gap_count; ++i) chars_tex[k + j + i] = paragraph_i->gap[i];
			//k += i;
			for(; j < paragraph_i->buffer_count; ++j) chars_tex[k + j + i] = paragraph_i->buffer[j+paragraph_i->gap_del];
			j += i;
CARRY_ON_PARAS:
			k += j;
			l = k + grid.width-ceil((paragraph_i->buffer_count + paragraph_i->gap_count) % (int) grid.width);
			for(;k < l; ++k) chars_tex[k] = 0.;
		}

//		if(curr_paragraph == paragraph_i)
//		{
//			curr_line = k/grid.width-1;
//			cursor_position(paragraph_cursor % (int) grid.width+1, curr_line);
//		}

		//for(j = paragraph_i->lines_count; j >= 0; --j) grid_lines[m++] = paragraph_i;

		//paragraph_i = paragraph_i->next;
		
	}
	while((paragraph_i = paragraph_i->next) && k < grid_full_size);

	//for(;k < grid_full_size; ++k) chars_tex[k] = 65.f/256.f;
	for(;k < grid_full_size; ++k) chars_tex[k] = 0.f;

	update_grid_lines(); // Remove from  here

	load_char_tex();
}

static void new_paragraph(void)
{
	PARA *paragraph_aux;
	paragraph_cursor = 0;
	paragraph_aux = (PARA*) malloc(sizeof(PARA));
	paragraph_aux->buffer = (GLfloat*) malloc(sizeof(GLfloat)*PARA_BUFFER_SIZE);
	paragraph_aux->buffer_pos = 0;
	paragraph_aux->buffer_count = 0;
	paragraph_aux->lines_count = 0;
	paragraph_aux->gap = (GLfloat*) malloc(sizeof(GLfloat)*PARA_BUFFER_SIZE);
	paragraph_aux->gap_pos = 0;
	paragraph_aux->gap_count = 0;
	paragraph_aux->gap_del = 0;
	paragraph_aux->next = curr_paragraph->next;
	paragraph_aux->prev = curr_paragraph;
	curr_paragraph->next = paragraph_aux;
	curr_paragraph = paragraph_aux;
	++paragraphs_count;
}

static void free_paragraph(PARA *paragraph_aux)
{
	free(paragraph_aux->buffer);
	free(paragraph_aux->gap);
	free(paragraph_aux);
}


static void mouse_position_callback(GLFWwindow *window, double xpos, double ypos)
{

	glUseProgram(program_text);
	mouse_pos[0] = ceil((xpos - grid_x_offset)/grid.cell_width);
	mouse_pos[1] = ceil(ypos/grid.cell_height) - 1;
	if(scroll_bar_grab) {
		if(scroll_bar_grab > ypos)
			scroll_callback(window, 0, -1);
	}

	if(xpos < window_size.width-SCROLL_BAR_WIDTH && xpos > grid_x_offset)
	{
#ifndef __EMSCRIPTEN__
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif
		glUniform2fv(mouse_location, 1, mouse_pos);
		cursor_reset();
	}
#ifndef __EMSCRIPTEN__
	else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{

	int mouse_Y_scroll;
	double xpos, ypos;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		glfwGetCursorPos(window, &xpos, &ypos);
		mouse_Y_scroll = mouse_pos[1] + lines_scroll;
		if(xpos < window_size.width-SCROLL_BAR_WIDTH)
		{
			merge_gap();
			if(grid_lines[mouse_Y_scroll])
			{
				selection_state.mouse = 1;
				selection_state.start_pos = mouse_Y_scroll * grid.width + mouse_pos[0];

				curr_paragraph = grid_lines[mouse_Y_scroll];
				for(i = 1;  grid_lines[mouse_Y_scroll - i] == curr_paragraph; ++i);
				paragraph_cursor = mouse_pos[0] + grid.width * --i - 1;
				printf("line: %d paragraph selected: %p cursor: %d\n", mouse_Y_scroll, (void*)grid_lines[mouse_Y_scroll], paragraph_cursor);

				//cursor_position(mouse_pos[0] - 1, mouse_pos[1]);
			}
			cursor_reset();
		}
		else
		{
			//if(window_size.height - ypos > scroll_bar.top)
			//else
			if(window_size.height - ypos > scroll_bar.bottom)
			scroll_bar_grab = 1;
			//else
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		selection_state.mouse = 0;
		//for(i = 0; i<grid_lines[(int)mouse_pos[1]]->buffer_count; ++i)
		//selection_tex[(int) (mouse_pos[1] * grid.width + mouse_pos[0])] = 1./256.;
		//selection_tex[(int) (mouse_pos[0] + mouse_pos[1] * grid.width)] = 1./256.;
		scroll_bar_grab = 0;
	}
	//glfwGetCursorPos(window, &aux[0], &aux[1]);
}

static void resize_editor(void)
{

	printf("w_w: %d w_h: %d\n", window_size.width, window_size.height);

	float width_minus_scrollbar = window_size.width-SCROLL_BAR_WIDTH;
	float width_grid_ratio = (width_minus_scrollbar-grid_x_offset)/grid.cell_width;
	float height_grid_ratio = window_size.height/grid.cell_height;

	update_all_paragraphs_lines_count();

	cursor_reset();
	printf(">>>>%d<<<<<\n", lines_decimals_count);
	grid_x_offset = lines_decimals_count * grid.cell_width;

	grid.width = floor(width_grid_ratio);
	grid.height = floor(height_grid_ratio);
	grid_full_size = grid.width*grid.height;
	grid_y_offset = (height_grid_ratio-grid.height)*grid.cell_height;

	glUniform4fv(grid_location, 1, (GLfloat*) &grid);

	SCROLL[0].x = 
	SCROLL[1].x =
	SCROLL[3].x = width_minus_scrollbar*2/window_size.width - 1.f;

	SCREEN[0].x = 
	SCREEN[1].x = 
	SCREEN[3].x = 
	LINECOL[2].x = 
	LINECOL[4].x =
	LINECOL[5].x = grid_x_offset * 2.f/window_size.width - 1.f;


	SCREEN[2].x = 
	SCREEN[4].x = 
	SCREEN[5].x = (width_minus_scrollbar-(width_grid_ratio-grid.width)*grid.cell_width)*2/window_size.width - 1.f;

	SCREEN[1].y = 
	SCREEN[2].y = 
	SCREEN[5].y =
	LINECOL[1].y = 
	LINECOL[2].y = 
	LINECOL[5].y =  grid_y_offset*2/window_size.height - 1.f;

	grid_y_offset = grid.cell_height - grid_y_offset;

	glUniform1f(grid_Y_offset_location, (GLfloat) grid_y_offset);
	glUniform1f(grid_X_offset_location, (GLfloat) grid_x_offset);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_linenumber);
	glBufferData(GL_ARRAY_BUFFER, sizeof(LINECOL), LINECOL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCROLL), SCROLL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN), SCREEN, GL_STATIC_DRAW);

	//glfwGetFramebufferSize(window, &window_size.width, &window_size.height);
	glViewport(0, 0, window_size.width, window_size.height);
	if(grid.height != grid_paragraph_count)
	{
		printf("Reallocing_____>\n");
		grid_paragraph_count = grid.height;
		grid_lines = (PARA**) realloc(grid_lines, grid_paragraph_count * sizeof(PARA*));
		line_numbers = (GLfloat*) realloc(line_numbers, grid_paragraph_count * sizeof(unsigned int));
	}
//	while(grid_full_size > chars_buffer_size)
//	{
//		chars_tex = (GLfloat*) realloc(chars_tex, chars_buffer_size+INITIAL_CHARS_BUFFER_SIZE);
//		//for(i = chars_buffer_size; i<(chars_buffer_size+INITIAL_CHARS_BUFFER_SIZE); ++i) chars_tex[i] = 0.;
//		chars_buffer_size += INITIAL_CHARS_BUFFER_SIZE;
//		if(chars_tex == NULL)
//		{
//			print_to_screen("Expanding mem failed!\n");
//			exit(1);
//		}
//	}

	//cursor_position(paragraph_cursor % (int)grid.width, curr_line);

	// Init texture. Move to subteximage ?
	//load_char_tex();
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, chars_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, chars_tex);

        //glActiveTexture(GL_TEXTURE3);
	//linenumber_update(lines_count);

//	glUseProgram(program_linenumber);
//	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);
//        glBindTexture(GL_TEXTURE_2D, linenumber_tex);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex);
	//update_chars_tex();

}

static inline void window_size_callback(GLFWwindow *window, int width,  int height)
{
	window_size.width = width;
	window_size.height = height;
	resize_editor();
}

static inline void insert_one_char(unsigned int codepoint)
{
	if(curr_paragraph->gap_count)
	{
		curr_paragraph->gap[curr_paragraph->gap_count++] = (codepoint-14)/256.;
		++paragraph_cursor;
	}
	else if(paragraph_cursor < curr_paragraph->buffer_count)
	{
		//if(!curr_paragraph->gap_count)
		curr_paragraph->gap_pos = paragraph_cursor;
		curr_paragraph->gap[curr_paragraph->gap_count++] = (codepoint-14)/256.;
		++paragraph_cursor;
	}
	else
	{
		curr_paragraph->buffer[paragraph_cursor++] = (codepoint-14)/256.;
		++curr_paragraph->buffer_count;
	}

	if(paragraph_cursor && !(paragraph_cursor % (int) grid.width))
	//if(paragraph_cursor && !((curr_paragraph->buffer_count + curr_paragraph->gap_count) % (int) grid.width))
	{
		update_lines_count(LINE_ADD);
		++curr_line;
		if(curr_line >= grid.height)
		{
			printf("OUT SCROLL %d\n", curr_line);
			lines_scroll = curr_line - grid.height + 1;
			get_top_paragraph_scroll();
		}
	}
}


static inline void character_callback(GLFWwindow *window, unsigned int codepoint)
{
//	switch(codepoint)
//	{
//		case 32 ... 126: 
	if(codepoint >= 32 && codepoint <= 126)
	{
		insert_one_char(codepoint);
		//cursor_position((paragraph_cursor) % (int) grid.width, curr_line);
		cursor_reset();
//			}
//			else
//			{
//				chars_tex[paragraph_cursor % (int)grid.width + curr_line * (int)grid.width] = (codepoint-32)/256.;
//				load_char_tex();
//			}
//			break;
	}
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	int i;

	PARA *paragraph_aux;

	if(action == GLFW_PRESS || action == GLFW_REPEAT) switch(key)
	{
		case GLFW_KEY_UP: 
			if (mods == GLFW_MOD_SHIFT)
			{
				grid.cell_height -= 1.;
				resize_editor();
			}
			else
			{
				merge_gap();
				if(paragraph_cursor > grid.width)
				{
					printf("cursor same para\n");
					//paragraph_cursor = paragraph_cursor - paragraph_cursor % (int)grid.width;
					paragraph_cursor -= grid.width;
					--curr_line;
				}
				else if(curr_paragraph->prev)
				{
					merge_gap();
					curr_paragraph = curr_paragraph->prev;
					if(curr_paragraph->buffer_count < paragraph_cursor) paragraph_cursor = curr_paragraph->buffer_count;
					--curr_line;
				}
			}
			break;
		case GLFW_KEY_DOWN: 
			if (mods == GLFW_MOD_SHIFT)
			{
				grid.cell_height += 1.;
				resize_editor();
			}
			else
			{
				merge_gap();
				if(paragraph_cursor + grid.width <= curr_paragraph->buffer_count)
				{
					printf("____________X\n");
					paragraph_cursor += grid.width;
					++curr_line;
				}
				//else if(grid.width - curr_paragraph->buffer_count % paragraph_cursor)
				else if(curr_paragraph->buffer_count - paragraph_cursor > grid.width)
				{
					printf("____________Y\n");
					paragraph_cursor = curr_paragraph->buffer_count;
					++curr_line;
				}
				else if(curr_paragraph->next)
				{
					printf("____________Z\n");
					merge_gap();
					curr_paragraph = curr_paragraph->next;
					if(curr_paragraph->buffer_count < paragraph_cursor) paragraph_cursor = curr_paragraph->buffer_count;
					++curr_line;
				}
				else printf("____________Q\n");
			}
			break;
		case GLFW_KEY_RIGHT: 
			if (mods == GLFW_MOD_SHIFT)
			{
				grid.cell_width += 1.;
				resize_editor();
			}
			else
			{
				merge_gap();
				if(paragraph_cursor < curr_paragraph->buffer_count)
				{
					++paragraph_cursor;
					if(!((paragraph_cursor) % (int)grid.width)) ++curr_line;
				}
			}
			break;
		case GLFW_KEY_LEFT: 
			if (mods == GLFW_MOD_SHIFT)
			{
				grid.cell_width -= 1.;
				resize_editor();
			}
			else
			{
			merge_gap();
				if(paragraph_cursor)
				{
					if(!((paragraph_cursor) % (int)grid.width)) --curr_line;
					--paragraph_cursor;
				}
			}
			break;
		case GLFW_KEY_LEFT_SHIFT: 
			break;
#ifndef __EMSCRIPTEN__
			paste_buff = glfwGetClipboardString(window);
			i = 0;
			while(paste_buff[i] != '\0')
			{
				if(paste_buff[i] == 13 || paste_buff[i] == 10) new_paragraph(); // CR/LF
				else
				{
					curr_paragraph->buffer[paragraph_cursor] = (paste_buff[i]-14)/256.;
					++curr_paragraph->buffer_count;
					++paragraph_cursor;
				}
				++i;
			}
			//update_lines_count();
			//update_all_paragraphs_lines_count();
#endif
			break;
		case GLFW_KEY_PAGE_UP:
			lines_scroll -= grid.height;
			//update_lines_count();
			break;
		case GLFW_KEY_PAGE_DOWN:
			lines_scroll += grid.height;
			//update_lines_count();
			break;
		case GLFW_KEY_END: 
			merge_gap();
			paragraph_cursor = curr_paragraph->buffer_count;
			break;
		case GLFW_KEY_HOME: 
			merge_gap();
			paragraph_cursor = 0;
			curr_line -= curr_paragraph->lines_count;
			break;
		case GLFW_KEY_BACKSPACE: 
			if(curr_paragraph->gap_count)
			{
				printf("backspace gap\n");
					if(!((paragraph_cursor)%(int)grid.width))
					{
						printf("remove line\n");
						--curr_line;
						update_lines_count(LINE_REMOVE);
					}
				--curr_paragraph->gap_count;
				--paragraph_cursor;
			}
			else
			{
				merge_gap();
				if(curr_paragraph->buffer_count && paragraph_cursor)
				{
					if(!((paragraph_cursor)%(int)grid.width))
					{
						printf("remove line\n");
						--curr_line;
						update_lines_count(LINE_REMOVE);
					}
					--curr_paragraph->buffer_count;
					--paragraph_cursor;
					if(paragraph_cursor != curr_paragraph->buffer_count)
					{
						printf("backspace middle\n");
						curr_paragraph->gap_pos = paragraph_cursor;
//						if(!((paragraph_cursor)%(int)grid.width))
//						{
//							printf("backspace remove line\n");
//						}
						++curr_paragraph->gap_del;
					}
					else
					{
						printf("backspace end\n");
					}
				}
				else
				{
					if(curr_paragraph->prev)
					{
						printf("backspace prev line\n");
						curr_paragraph->prev->next = curr_paragraph->next;
						if(curr_paragraph->next) curr_paragraph->next->prev = curr_paragraph->prev;
						paragraph_aux = curr_paragraph;
						curr_paragraph = curr_paragraph->prev;
						paragraph_cursor = curr_paragraph->buffer_count;
						free_paragraph(paragraph_aux);
						--curr_line;
						//update_lines_count(LINE_REMOVE);
					}
				}
			}
			break;
		case GLFW_KEY_ENTER: 
			merge_gap();
			//chars_tex[paragraph_cursor] = 0;
			//paragraph_cursor += 160-paragraph_cursor%160;
			if(paragraph_cursor != curr_paragraph->buffer_count)
			{
				printf("chop paragraph\n");
			}
			new_paragraph();
			update_lines_count(LINE_ADD);
			++curr_line;
			//if(lines_count/(lines_decimals_count+1)*10) resize_editor();
			break;
		case GLFW_KEY_ESCAPE: 
			//for(i = 0; i < chars_buffer_size; ++i) selection_tex[i] = 0.;
			//load_selection_tex();
			break;
		case GLFW_KEY_TAB: 
#ifdef __EMSCRIPTEN__
			//call_alert();
#endif
			for(i = 0; i < grid.height; ++i) 
			{
				printf("[%d]: %p %12f\n", i, (void*)grid_lines[i], line_numbers[i]);
			}
			printf("curr_paragraph: %p prev_paragraph: %p lines_count: %u curr_line: %u\ntop: %f bottom: %f\nparagraph_cursor: %d paragraph_count: %d\ngap_pos: %d gap_del: %d gap_count: %d top_line_number: %d\n", (void*)curr_paragraph, (void*)curr_paragraph->prev, lines_count, curr_line, scroll_bar.top, scroll_bar.bottom, paragraph_cursor, curr_paragraph->buffer_count, curr_paragraph->gap_pos, curr_paragraph->gap_del, curr_paragraph->gap_count, top_line_number);
			break;
		case GLFW_KEY_RIGHT_SHIFT: 
#ifdef __EMSCRIPTEN__
			//copy_to_clipboard((char)chars_tex[0], paragraph_cursor);
#endif
			break;
	}
	//update_chars_tex();
	//cursor_position();
//	cursor_position((paragraph_cursor) % (int) grid.width, curr_line);
//	cursor_reset();
}

#ifdef __EMSCRIPTEN__
int paste_char(char *c) {
	if(*c == 13 || *c == 10) // CR/LF
	{
		new_paragraph();
	}
	curr_paragraph->buffer[paragraph_cursor] = (c[0]-14)/256.;
	++curr_paragraph->buffer_count;
	++paragraph_cursor;
	return 0;
}

static EM_BOOL on_web_display_size_changed(int event_type, const EmscriptenUiEvent *event, void *user_data)
{
	double w, h;
	emscripten_get_element_css_size( "#canvas", &w, &h);
	printf("____%f %f\n", w, h);
	window_size_callback(window, (int) w, (int) h);
	return 0;
}

static void frame(void) {}

void draw(void) {

	glfwPollEvents();
#else
static void frame(void) {
	
	++blink_count;
	if(BLINKS_START > blink_count) glfwWaitEventsTimeout(.5);
	else if(blink_count <= BLINKS_LIMIT)
	{
		glUniform1f(cursor_blink_location, (GLfloat) blink_state);
		blink_state ^= 1;
		glfwWaitEventsTimeout(.5);
	}
	else glfwWaitEvents();

#endif

#ifdef SHOW_FPS
	//clock_gettime(CLOCK_REALTIME, &fps_start);
#endif
	// REMOVE MOST OF THESE FROM HERE
	update_chars_tex();
	cursor_position();
	load_selection_tex();
	scrollbar_update();
	linenumber_update(lines_count); //For now
	//load_linenumber_tex();
	//cursor_position(paragraph_cursor % (int) grid.width+1, grid.width-1);
	//sleep(5);
	//stop = clock();
	

//#ifdef SHOW_FPS
//	curr_frame_time = glfwGetTime();
//	fps_avg += 1000./(curr_frame_time - last_frame_time);
//	last_frame_time = curr_frame_time;
////	blink_count += curr_frame_time - last_frame_time;
////	if(blink_count > BLINK_TIME)
////	{
////		blink_count = 0;
////		chars_tex[(paragraph_cursor)] = chars_tex[(paragraph_cursor)] ? 0 : 95./256.;
////		update_chars_tex();
////	}
//	if(fps_avg_count == FPS_AVG_COUNT)
//	{
//
//		fps_avg = fps_avg/FPS_AVG_COUNT/1000.;
//		fps_int = fps_avg;
//		fps_dec = (fps_avg-fps_int)*1000;
//		chars_tex[grid_full_size-3] = 38./256.;
//		chars_tex[grid_full_size-2] = 48./256.;
//		chars_tex[grid_full_size-1] = 51./256.;
//		i = 5;
//		do
//		{
//			j = fps_dec / 10;
//			chars_tex[grid_full_size-i] = (fps_dec-j*10+16)/256.;
//			fps_dec = j;
//			++i;
//		}
//		while(fps_dec);
//		chars_tex[grid_full_size-i] = 14./256.;
//		++i;
//		do
//		{
//			j = fps_int / 10;
//			chars_tex[grid_full_size-i] = (fps_int-j*10+16)/256.;
//			fps_int = j;
//			++i;
//		}
//		while(fps_int);
//
//		fps_avg_count = 0;
//		fps_avg = 0;
//		update_chars_tex();
//
//	} else ++fps_avg_count;
//#endif

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program_scroll);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);
	glVertexAttribPointer(vpos_location_scroll, 2, GL_FLOAT, GL_FALSE, sizeof(SCROLL[0]), (void*) 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(program_linenumber);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_linenumber);
	glVertexAttribPointer(vpos_location_linenumber, 2, GL_FLOAT, GL_FALSE, sizeof(LINECOL[0]), (void*) 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(program_text);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(vpos_location_text, 2, GL_FLOAT, GL_FALSE, sizeof(SCREEN[0]), (void*) 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(window);
	
#ifdef SHOW_FPS
	//clock_gettime(CLOCK_REALTIME, &fps_stop);
	//printf(">FPS: %lf\n", (double)1./((fps_stop.tv_sec - fps_start.tv_sec) + (fps_stop.tv_nsec - fps_start.tv_nsec)/1E9));
#endif

}

int main(void) {

	int success;
	char infoLog[516];

	print_to_screen("main function started");

	glfwSetErrorCallback(error_callback);
	if (glfwInit() != GL_TRUE)
	{
		print_to_screen("glfwInit() failed!");
		glfwTerminate();
		exit(1);
	}
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	print_to_screen("glfwInit() success");

	window = glfwCreateWindow(1280, 720, "Simple Editor", NULL, NULL);
	if (!window){
		print_to_screen("glfwCreateWindow() failed!");
		glfwTerminate();
		exit(1);
	}

	print_to_screen("glfwCreateWindow() success\n");

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glClearColor(0.f, 0.f, 0.f, 0.f);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glGenBuffers(1, &vertex_buffer_scroll);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);

	glGenBuffers(1, &vertex_buffer_linenumber);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_linenumber);
	
	// Programs

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (GL_TRUE != success)
	{
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &success);
		glGetShaderInfoLog(vertex_shader, 512, NULL, infoLog);
		print_to_screen("Error Vertex: ");
		print_to_screen(infoLog);
		print_to_screen("\n");
		exit(1);
	}

	//	Linenumber
	build_fragment_shader(linenumber);
	linenumber_location = glGetUniformLocation(program_linenumber, "LineNumbers");
	grid_location_linenumber = glGetUniformLocation(program_linenumber, "Grid");
	grid_Y_offset_location_linenumber = glGetUniformLocation(program_linenumber, "GridYOffset");
	glUniform1i(glGetUniformLocation(program_linenumber, "Texture"), 0);
	glUniform1i(linenumber_location, 3);
	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);

	//	Scroll 
	build_fragment_shader(scroll);
	scroll_bar_location = glGetUniformLocation(program_scroll, "ScrollBar");

	//	Text
	build_fragment_shader(text);
	mouse_location = glGetUniformLocation(program_text, "Mouse");
	grid_location = glGetUniformLocation(program_text, "Grid");
	chars_location = glGetUniformLocation(program_text, "Chars");
	cursor_location = glGetUniformLocation(program_text, "Cursor");
	cursor_blink_location = glGetUniformLocation(program_text, "CursorBlink");
	glUniform1f(cursor_blink_location, (GLfloat) blink_state);
	grid_Y_offset_location = glGetUniformLocation(program_text, "GridYOffset");
	grid_X_offset_location = glGetUniformLocation(program_text, "GridXOffset");
	selection_location = glGetUniformLocation(program_text, "Selection");
	glUniform1i(glGetUniformLocation(program_text, "Texture"), 0);
	glUniform1i(selection_location, 2);
	glUniform1i(chars_location, 1);

	// Textures

	if(!(file_tex = fopen("assets/charmap-oldschool_white.bmp", "rb"))) return 1;

	if(fread(bmp_header, 1, 132, file_tex) != 132) return 1;

	bmp_dataPos = *(int*) & (bmp_header[0x0A]);
	bmp_image_size = *(int*) & (bmp_header[0x22]);
	bmp_width = *(int*) & (bmp_header[0x12]);
	bmp_height = *(int*) & (bmp_header[0x16]);

	bmp_data = (unsigned char*) malloc(sizeof(char)*bmp_image_size);

	// ARGB to RGBA. Mind endianess
	for(j=0; j<bmp_image_size; j+=4)
	{
		fread(&rgba[0], 1, 1, file_tex);
		fread(&rgba[3], 1, 1, file_tex);
		fread(&rgba[2], 1, 1, file_tex);
		fread(&rgba[1], 1, 1, file_tex);
		for(k=0; k<4; ++k) bmp_data[j+k] = rgba[k];
	}

	fclose(file_tex);

	glfwGetWindowSize(window, &window_size.width, &window_size.height);
	printf("Init: w_w: %d w_h: %d\n", window_size.width, window_size.height);

        glGenTextures(1, &font_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp_width, bmp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp_data);

        glGenTextures(1, &char_tex);
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	chars_tex = (GLfloat*) malloc(sizeof(GLfloat)*chars_buffer_size);
	for(i = 0; i < INITIAL_CHARS_BUFFER_SIZE; ++i) chars_tex[i] = 0.;
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 640, 480, 0, GL_RED, GL_FLOAT, chars_tex);
	//glUniform1i(chars_location, 1);

        glGenTextures(1, &select_tex);
	glActiveTexture(GL_TEXTURE2); 
	glBindTexture(GL_TEXTURE_2D, select_tex);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	selection_tex = (GLfloat*) malloc(sizeof(GLfloat)*chars_buffer_size);
	for(i = 0; i < INITIAL_CHARS_BUFFER_SIZE; ++i) selection_tex[i] = 0.;
	//glUniform1i(chars_location, 2);
	//load_selection_tex();

        glGenTextures(1, &linenumber_tex);
	glActiveTexture(GL_TEXTURE3); 
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	linenumbers_tex = (GLfloat*) malloc(sizeof(GLfloat)*chars_buffer_size); // HMMMMMM
	for(i = 0; i < INITIAL_CHARS_BUFFER_SIZE; ++i) linenumbers_tex[i] = 0.f;
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex);
	//load_linenumber_tex();

	// INPUT
	glfwSetKeyCallback(window, key_callback);

	glfwSetCharCallback(window, character_callback);

	glfwSetCursorPosCallback(window, mouse_position_callback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);

#ifndef __EMSCRIPTEN__
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif

	glfwSetWindowSizeCallback(window, window_size_callback);

	cursor_position();

	// PARAS
	top_paragraph_scroll = curr_paragraph = &paragraphs_head;
	paragraphs_head.buffer = (GLfloat*) malloc(sizeof(GLfloat) * PARA_BUFFER_SIZE);
	paragraphs_head.gap = (GLfloat*) malloc(sizeof(GLfloat) * PARA_BUFFER_SIZE);
	//grid_lines = (void*) malloc(sizeof(PARA*) * INITIAL_GRID_PARAS);
			// REMOVE THIS
//			for(i = 0; i < grid_paragraph_count; ++i) 
//			{
//				grid_lines[i] = NULL;
//			}

	//update_grid_lines();
	resize_editor();

#ifdef __EMSCRIPTEN__

	draw();

	emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 0, on_web_display_size_changed);
	emscripten_set_main_loop(frame, -1, 1);
#else
	while(1) frame();
#endif

	glfwDestroyWindow(window);

	glfwTerminate();

	return EXIT_SUCCESS;
}
