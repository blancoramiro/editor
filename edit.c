/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <time.h>
struct timespec start, stop;

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

extern const char* vertex_shader_text;
extern const char* fragment_shader_text;
extern const char* fragment_shader_scroll_text;

GLFWwindow* window;

GLuint  vertex_buffer,
	vertex_buffer_scroll,
	vertex_shader,
	fragment_shader,
	fragment_shader_scroll,
	program,
	program_scroll,
	vpos_location,
	vpos_location_scroll,
	mouse_location,
	grid_location,
	scroll_bar_location,
	cursor_location,
	cursor_blink_location,
	grid_Y_offset_location,
	chars_location,
	selection_location;

GLuint  font_tex,
	char_tex,
	select_tex;

GLfloat mouse_pos[2];
struct 
{
	unsigned int mouse, shift, start_pos;
}
selection_state = {0, 0, 0};

FILE* file_tex;
char rgba[4];
unsigned char bmp_header[54]; // Each BMP file begins by a 54-bytes header
unsigned int bmp_dataPos; // Position in the file where the actual data begins
unsigned int bmp_width, bmp_height;
unsigned int bmp_image_size; // = width*height*3
unsigned char* bmp_data; // Actual RGB data

int width, height;

//GRID grid = {0., 0., 40., 38.};
GRID grid = {0., 0., 7., 9.};
//grid = {0., 0., 7., 9.};

SCROLL_BAR scroll_bar = {400., 700.};

double scroll_bar_grab = 0;

W_SIZE window_size = {0, 0};

int fd;

int i, j, k, l, m, n;

double curr_frame_time, last_frame_time, fps_avg_count = 0, fps_avg;
unsigned int fps_int, fps_dec;

unsigned int blink_state = 1, blink_count = 0;

PARA* curr_paragraph = NULL;
PARA paragraphs_head = { NULL, NULL, NULL, 1, 0, 0, NULL, 0, 0 };
PARA** grid_lines = NULL;

PARA* top_paragraph_scroll = NULL;
unsigned int lines_scroll_top_diff = 0;

unsigned int paragraphs_count = 1; // Start with one line
unsigned int lines_count = 1; // Start with one line
unsigned int lines_scroll = 0;
unsigned int paragraph_cursor = 0;
unsigned int grid_paragraph_count = INITIAL_GRID_PARAS;

unsigned int chars_buffer_size = INITIAL_CHARS_BUFFER_SIZE;
GLfloat* chars_tex;
GLfloat* selection_tex;

#ifndef __EMSCRIPTEN__
const char* paste_buff;
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
} ;

static void print_to_screen(const char* buff)
{
#ifdef __EMSCRIPTEN__
	emscripten_console_log(buff);
#else
	printf("%s", buff);
#endif
}

static void error_callback(int error, const char* description)
{
	print_to_screen("Error!");
	print_to_screen(description);
}

static inline void load_char_tex(void)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid.width, grid.height, GL_RED, GL_FLOAT, chars_tex);
	glUniform1i(chars_location, 1);
}

void load_selection_tex(void)
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, select_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, selection_tex);
	glUniform1i(selection_location, 2);
}

void cursor_reset(void)
{
	blink_count = 0;
	blink_state = 1;
	glUniform1f(cursor_blink_location, 1.);
}

static void cursor_position(GLfloat x, GLfloat y)
{

	float aux[] = {x, y};
	glUniform2fv(cursor_location, 1, aux);
}

static inline void update_lines_count(short action)
{

	lines_count += action;
	curr_paragraph->lines_count += action;
	printf("lines count: %d\n", lines_count);
}


static inline void update_all_paragraphs_lines_count(void)
{
	PARA* paragraph_i = &paragraphs_head;

	lines_count = 0;

	do
	{
		lines_count += paragraph_i->lines_count = paragraph_i->buffer_count ? ceil(paragraph_i->buffer_count / grid.width) : 1;
		paragraph_i = paragraph_i->next;
	}
	while(paragraph_i);
	printf("lines count: %d\n", lines_count);
}

void get_top_paragraph_scroll(void) 
{
	top_paragraph_scroll = &paragraphs_head;
	lines_scroll_top_diff = lines_scroll;
	while(lines_scroll_top_diff >= top_paragraph_scroll->lines_count)
	{
		lines_scroll_top_diff -= top_paragraph_scroll->lines_count;
		top_paragraph_scroll = top_paragraph_scroll->next;
	}
}

void update_chars_tex(void) 
{
	clock_gettime(CLOCK_REALTIME, &start);

	PARA* paragraph_i = top_paragraph_scroll;
	k = m = 0;

		printf("it is %d %d!!!!\n", lines_scroll_top_diff, top_paragraph_scroll->lines_count);

//	if(paragraph_i->lines_count > lines_scroll_top_diff)
//	{
	n = lines_scroll_top_diff * grid.width;
	i = paragraph_i->buffer_count - n;
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

			for(j = 0; j < paragraph_i->buffer_count; ++j) chars_tex[k + j] = paragraph_i->buffer[j];
CARRY_ON_PARAS:
			k += j;
			l = k+grid.width-ceil(paragraph_i->buffer_count % (int) grid.width);
			for(;k < l; ++k) chars_tex[k] = 0.;
		}

		if(curr_paragraph == paragraph_i)
		{
			cursor_position(paragraph_cursor % (int) grid.width+1, k/grid.width-1);
		}

		for(j = paragraph_i->lines_count; j >= 1; --j) grid_lines[m++] = paragraph_i;

		paragraph_i = paragraph_i->next;
		
	}

	while(paragraph_i && k < chars_buffer_size);

	for(;k < chars_buffer_size; ++k) chars_tex[k] = 65.f/256.f;

	for(;m < grid_paragraph_count; ++m) grid_lines[m] = NULL;

	load_char_tex();
	//sleep(5);
	//stop = clock();
	printf("__%d\n", clock_gettime(CLOCK_REALTIME, &stop));
	//printf(">%Lf\n", (long double)1000./((long double)(stop - start)/CLOCKS_PER_SEC));
	printf(">%lf\n", (double)((stop.tv_sec - start.tv_sec)));
	printf(">FPS: %lf\n", (double)1./((stop.tv_sec - start.tv_sec) + (stop.tv_nsec - start.tv_nsec)/1E9));
	//printf(">%Lf\n", (long double)CLOCKS_PER_SEC);
	//printf("%u\n", CLOCKS_PER_SEC);
}

static void new_paragraph(void)
{
	PARA* paragraph_aux;
	paragraph_cursor = 0;
	paragraph_aux = (PARA*) malloc(sizeof(PARA));
	paragraph_aux->buffer = (GLfloat*) malloc(sizeof(GLfloat)*PARA_BUFFER_SIZE);
	paragraph_aux->buffer_pos = 0;
	paragraph_aux->buffer_count = 0;
	paragraph_aux->lines_count = 1;
	paragraph_aux->gap = (GLfloat*) malloc(sizeof(GLfloat)*PARA_BUFFER_SIZE);
	paragraph_aux->gap_pos = 0;
	paragraph_aux->gap_count = 0;
	paragraph_aux->next = curr_paragraph->next;
	paragraph_aux->prev = curr_paragraph;
	curr_paragraph->next = paragraph_aux;
	curr_paragraph = paragraph_aux;
	++paragraphs_count;
	++lines_count;
	update_chars_tex();
	scroll_bar_update();
}

static void free_paragraph(PARA* paragraph_aux)
{
	free(paragraph_aux->buffer);
	free(paragraph_aux->gap);
	free(paragraph_aux);
}


static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos)
{

	glUseProgram(program);
	mouse_pos[0] = ceil(xpos/grid.cell_width);
	mouse_pos[1] = ceil(ypos/grid.cell_height);
	if(scroll_bar_grab) {
		printf("move scroll bar!\n");
		if(scroll_bar_grab > ypos)
			scroll_callback(window, 0, -1);
	}

	if(xpos < window_size.width-SCROLL_BAR_WIDTH)
	{
		if(selection_state.mouse) printf("I am selecting stuff!\n");
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		glUniform2fv(mouse_location, 1, mouse_pos);
		cursor_reset();
	}
	else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{

	double xpos, ypos;

	cursor_reset();
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		printf("PRESS\n");
		glfwGetCursorPos(window, &xpos, &ypos);
		if(xpos < window_size.width-SCROLL_BAR_WIDTH)
		{
			printf(">>>%p\n", grid_lines[(int)mouse_pos[1]]);
			selection_state.mouse = 1;
			selection_state.start_pos = mouse_pos[1] * grid.width + mouse_pos[0];
			cursor_position(mouse_pos[0], mouse_pos[1]);
		}
		else
		{
			printf("%f %f\n", xpos, ypos);
			printf("BAR ");
			if(window_size.height - ypos > scroll_bar.top)
			printf("move up\n");
			else if(window_size.height - ypos > scroll_bar.bottom)
			scroll_bar_grab = 1;
			else
			printf("move down\n");
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
	{
		printf("RELEASE\n");
		selection_state.mouse = 0;
		printf("_____%d\n", grid_lines[(int)mouse_pos[1]]->buffer_count);
		for(i = 0; i<grid_lines[(int)mouse_pos[1]]->buffer_count; ++i)
		selection_tex[(int) (mouse_pos[1] * grid.width + mouse_pos[0])] = 1./256.;
		//selection_tex[(int) (mouse_pos[0] + mouse_pos[1] * grid.width)] = 1./256.;
		load_selection_tex();
		scroll_bar_grab = 0;
	}
	//glfwGetCursorPos(window, &aux[0], &aux[1]);
	printf("%f_%f\n", mouse_pos[0], mouse_pos[1]);
}

static void window_size_callback(GLFWwindow* window, int window_width, int window_height)
{

	float width_minus_scrollbar = window_width-SCROLL_BAR_WIDTH;
	float width_grid_ratio = width_minus_scrollbar/grid.cell_width;
	float height_grid_ratio = window_height/grid.cell_height;

	cursor_reset();

	grid.width = floor(width_grid_ratio);
	grid.height = floor(height_grid_ratio);
	grid.full_size = grid.width*grid.height;
	grid.y_offset = (height_grid_ratio-grid.height)*grid.cell_height;

	glUniform4fv(grid_location, 1, (GLfloat*) &grid);


	SCROLL[0].x = 
	SCROLL[1].x =
	SCROLL[3].x = width_minus_scrollbar*2/window_width - 1.f;

	SCREEN[2].x = 
	SCREEN[4].x = 
	SCREEN[5].x = (width_minus_scrollbar-(width_grid_ratio-grid.width)*grid.cell_width)*2/window_width - 1.f;

	SCREEN[1].y = 
	SCREEN[2].y = 
	SCREEN[5].y =  grid.y_offset*2/window_height - 1.f;

	grid.y_offset = grid.cell_height - grid.y_offset;

	glUniform1f(grid_Y_offset_location, (GLfloat) grid.y_offset);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCROLL), SCROLL, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN), SCREEN, GL_STATIC_DRAW);

	glfwGetFramebufferSize(window, &window_width, &window_height);
	glViewport(0, 0, window_width, window_height);
	printf("%f %d\n", grid.height, grid_paragraph_count);
	if(grid.height > grid_paragraph_count)
	{
		grid_paragraph_count = grid.height;
		printf("REALLOC grid_lines %d\n", grid_paragraph_count);
		grid_lines = (PARA**) realloc(grid_lines, grid_paragraph_count * sizeof(PARA*));
	}
//	printf("ffff %f %f %u %u\n", grid.width, grid.height, grid.full_size, chars_buffer_size);
//	while(grid.full_size > chars_buffer_size)
//	{
//		printf("dddd %f %f %u %u\n", grid.width, grid.height, grid.full_size, chars_buffer_size);
//		chars_tex = (GLfloat*) realloc(chars_tex, chars_buffer_size+INITIAL_CHARS_BUFFER_SIZE);
//		//for(i = chars_buffer_size; i<(chars_buffer_size+INITIAL_CHARS_BUFFER_SIZE); ++i) chars_tex[i] = 0.;
//		chars_buffer_size += INITIAL_CHARS_BUFFER_SIZE;
//		printf("____ %f %f %u %u\n", grid.width, grid.height, grid.full_size, chars_buffer_size);
//		if(chars_tex == NULL)
//		{
//			print_to_screen("Expanding mem failed!\n");
//			exit(1);
//		}
//	}
	window_size.width = window_width;
	window_size.height = window_height;
	//update_lines_count();
	update_all_paragraphs_lines_count();

	// Init texture
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, chars_tex);

	update_chars_tex();
	load_selection_tex();
	scroll_bar_update();
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	switch(codepoint)
	{
		case 32 ... 126: 
			curr_paragraph->buffer[paragraph_cursor] = (codepoint-32)/256.;
			++paragraph_cursor;
			if(!(++curr_paragraph->buffer_count%(int)grid.width))
			{
				printf("going into newline!\n");
				update_lines_count(LINE_ADD);
				scroll_bar_update();
			}
			update_chars_tex();
			break;
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	int i;

	PARA* paragraph_aux;

	cursor_reset();
	if(action == GLFW_PRESS || action == GLFW_REPEAT)
	switch(key)
	{
		case GLFW_KEY_UP: 
			if(curr_paragraph->prev)
			{
				curr_paragraph = curr_paragraph->prev;
				printf("cp %u\n", paragraph_cursor);
				if(curr_paragraph->buffer_count < paragraph_cursor) paragraph_cursor = curr_paragraph->buffer_count;
				cursor_reset();
				update_chars_tex();
			}
			break;
		case GLFW_KEY_DOWN: 
			if(curr_paragraph->next)
			{
				curr_paragraph = curr_paragraph->next;
				printf("cp %u\n", paragraph_cursor);
				if(curr_paragraph->buffer_count < paragraph_cursor) paragraph_cursor = curr_paragraph->buffer_count;
				cursor_reset();
				update_chars_tex();
			}
			break;
		case GLFW_KEY_RIGHT: 
			if(curr_paragraph->buffer_count)
			{
				++paragraph_cursor;
				cursor_reset();
				update_chars_tex();
			}
			break;
		case GLFW_KEY_LEFT: 
			if(curr_paragraph->buffer_count)
			{
				--paragraph_cursor;
				cursor_reset();
				update_chars_tex();
			}
			break;
		case GLFW_KEY_LEFT_SHIFT: 
#ifndef __EMSCRIPTEN__
			paste_buff = glfwGetClipboardString(window);
			//printf("Paste full: %s\n", paste_buff);
			i = 0;
			while(paste_buff[i] != '\0')
			{
				//printf("%d Paste: %u\n", i, paste_buff[i]);
				if(paste_buff[i] == 13 || paste_buff[i] == 10) new_paragraph(); // CR/LF
				else
				{
					curr_paragraph->buffer[paragraph_cursor] = (paste_buff[i]-32)/256.;
					++curr_paragraph->buffer_count;
					++paragraph_cursor;
				}
				++i;
			}
			update_lines_count(5);
			update_chars_tex();
			scroll_bar_update();
#endif
			break;
		case GLFW_KEY_PAGE_UP:
			lines_scroll -= grid.height;
			//update_lines_count();
			update_chars_tex();
			scroll_bar_update();
			break;
		case GLFW_KEY_PAGE_DOWN:
			lines_scroll += grid.height;
			//update_lines_count();
			update_chars_tex();
			scroll_bar_update();
			break;
		case GLFW_KEY_END: 
			paragraph_cursor = curr_paragraph->buffer_count;
			update_chars_tex();
			break;
		case GLFW_KEY_BACKSPACE: 
			if(curr_paragraph->buffer_count)
			{
				--paragraph_cursor;
				--curr_paragraph->buffer_count;
				chars_tex[paragraph_cursor] = 0;
				if(!(paragraph_cursor%(int)grid.width))
				{
					printf("removing newline!\n");
					update_lines_count(LINE_REMOVE);
					scroll_bar_update();
				}
			}
			else
			{
				if(curr_paragraph->prev)
				{
				printf("___%u\n", curr_paragraph->buffer_count);
					curr_paragraph->prev->next = curr_paragraph->next;
					if(curr_paragraph->next) curr_paragraph->next->prev = curr_paragraph->prev;
					paragraph_aux = curr_paragraph;
					free_paragraph(curr_paragraph);
					curr_paragraph = curr_paragraph->prev;
					paragraph_cursor = curr_paragraph->buffer_count;
					update_lines_count(LINE_REMOVE);
					printf("fleto linea\n");
				}
			}
			update_chars_tex();
			break;
		case GLFW_KEY_ENTER: 
			//chars_tex[paragraph_cursor] = 0;
			//paragraph_cursor += 160-paragraph_cursor%160;
			new_paragraph();
			break;
		case GLFW_KEY_ESCAPE: 
			//for(i = 0; i < chars_buffer_size; ++i) selection_tex[i] = 0.;
			//load_selection_tex();
			printf("\\\\\n");
			scroll_bar_update();
			break;
		case GLFW_KEY_TAB: 
#ifdef __EMSCRIPTEN__
			//call_alert();
#endif
			//scroll_bar_update();
			for(i = 0; i < grid_paragraph_count; ++i) 
			{
				printf("____[%d]: %p\n", i, grid_lines[i]);
			}
		printf("lc: %u top: %f bottom: %f\n", lines_count, scroll_bar.top, scroll_bar.bottom);
			break;
		case GLFW_KEY_RIGHT_SHIFT: 
#ifdef __EMSCRIPTEN__
			//copy_to_clipboard((char)chars_tex[0], paragraph_cursor);
#endif
			break;
	}
	//update_chars_tex();
	//cursor_position();
}

#ifdef __EMSCRIPTEN__
int paste_char(char* c) {
	//printf("new line paste %c %i\n", *c, *c);
	if(*c == 13 || *c == 10) // CR/LF
	{
		new_paragraph();
	}
	curr_paragraph->buffer[paragraph_cursor] = (c[0]-32)/256.;
	++curr_paragraph->buffer_count;
	++paragraph_cursor;
	return 0;
}

static EM_BOOL on_web_display_size_changed(int event_type, const EmscriptenUiEvent *event, void *user_data)
{
	double w, h;
	emscripten_get_element_css_size( "#canvas", &w, &h);
	printf("____________%f %f\n", w, h);
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
	cursor_position(paragraph_cursor % (int) grid.width+1, grid.width-1);
	
#endif

#ifdef SHOW_FPS
	curr_frame_time = glfwGetTime();
	fps_avg += 1000./(curr_frame_time - last_frame_time);
	last_frame_time = curr_frame_time;
//	blink_count += curr_frame_time - last_frame_time;
//	if(blink_count > BLINK_TIME)
//	{
//		printf("FPSavg\n");
//		blink_count = 0;
//		chars_tex[(paragraph_cursor)] = chars_tex[(paragraph_cursor)] ? 0 : 95./256.;
//		printf("___%f\n", BLINK_TIME);
//		update_chars_tex();
//	}
	if(fps_avg_count == FPS_AVG_COUNT)
	{

		fps_avg = fps_avg/FPS_AVG_COUNT/1000.;
		//printf("FPSavg: %f\n", fps_avg);
		fps_int = fps_avg;
		fps_dec = (fps_avg-fps_int)*1000;
		chars_tex[grid.full_size-3] = 38./256.;
		chars_tex[grid.full_size-2] = 48./256.;
		chars_tex[grid.full_size-1] = 51./256.;
		i = 5;
		do
		{
			j = fps_dec / 10;
			chars_tex[grid.full_size-i] = (fps_dec-j*10+16)/256.;
			fps_dec = j;
			++i;
		}
		while(fps_dec);
		chars_tex[grid.full_size-i] = 14./256.;
		++i;
		do
		{
			j = fps_int / 10;
			chars_tex[grid.full_size-i] = (fps_int-j*10+16)/256.;
			fps_int = j;
			++i;
		}
		while(fps_int);

		fps_avg_count = 0;
		fps_avg = 0;
		update_chars_tex();

	} else ++fps_avg_count;
#endif

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program_scroll);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);
	glVertexAttribPointer(vpos_location_scroll, 2, GL_FLOAT, GL_FALSE, sizeof(SCROLL[0]), (void*) 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(SCREEN[0]), (void*) 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glfwSwapBuffers(window);
	

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

	window = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
	if (!window){
		print_to_screen("glfwCreateWindow() failed!");
		glfwTerminate();
		exit(1);
	}

	print_to_screen("glfwCreateWindow() success\n");

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	//glClearColor(.1f, .1f, .1f, 1.f);
	glClearColor(0.f, 0.f, 0.f, 0.f);

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCREEN), SCREEN, GL_STATIC_DRAW);

	glGenBuffers(1, &vertex_buffer_scroll);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_scroll);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SCROLL), SCROLL, GL_STATIC_DRAW);
	
	// Programs
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
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

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (GL_TRUE != success)
	{
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &success);
		glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
		print_to_screen("ERROR Fragment: ");
		print_to_screen(infoLog);
		print_to_screen("\n");
		exit(1);
	}

	fragment_shader_scroll = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_scroll, 1, &fragment_shader_scroll_text, NULL);
	glCompileShader(fragment_shader_scroll);
	glGetShaderiv(fragment_shader_scroll, GL_COMPILE_STATUS, &success);
	if (GL_TRUE != success)
	{
		glGetShaderiv(fragment_shader_scroll, GL_INFO_LOG_LENGTH, &success);
		glGetShaderInfoLog(fragment_shader_scroll, 512, NULL, infoLog);
		print_to_screen("ERROR Fragment: ");
		print_to_screen(infoLog);
		print_to_screen("\n");
		exit(1);
	}

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		printf("Error %s\n", infoLog);
	}

	program_scroll = glCreateProgram();
	glAttachShader(program_scroll, vertex_shader);
	glAttachShader(program_scroll, fragment_shader_scroll);

	glLinkProgram(program_scroll);

	glGetProgramiv(program_scroll, GL_LINK_STATUS, &success);
	if(!success) {
		glGetProgramInfoLog(program_scroll, 512, NULL, infoLog);
		printf("Error %s\n", infoLog);
	}

	// SCROLL
	glUseProgram(program_scroll);

	vpos_location_scroll = glGetAttribLocation(program_scroll, "vPos");
	glEnableVertexAttribArray(vpos_location_scroll);

	scroll_bar_location = glGetUniformLocation(program_scroll, "ScrollBar");

	// SCREEN
	glUseProgram(program);

	vpos_location = glGetAttribLocation(program, "vPos");
	glEnableVertexAttribArray(vpos_location);

	file_tex = fopen("assets/charmap-oldschool_white.bmp", "rb");

	if(fread(bmp_header, 1, 132, file_tex) != 132)
	{
		printf("Error reading font!\n");
		return 0;
	}

	bmp_dataPos = *(int*) & (bmp_header[0x0A]);
	bmp_image_size = *(int*) & (bmp_header[0x22]);
	bmp_width = *(int*) & (bmp_header[0x12]);
	bmp_height = *(int*) & (bmp_header[0x16]);

	printf("imagesize: %d width: %d height: %d\n", bmp_image_size, bmp_width, bmp_height);

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

        glGenTextures(1, &font_tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmp_width, bmp_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp_data);
	glUniform1i(glGetUniformLocation(program, "Texture"), 0);

        glGenTextures(1, &char_tex);
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, char_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	chars_location = glGetUniformLocation(program, "Chars");
	chars_tex = (GLfloat*) malloc(sizeof(GLfloat)*chars_buffer_size);
	for(i = 0; i < INITIAL_CHARS_BUFFER_SIZE; ++i) chars_tex[i] = 0.;
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, chars_tex);
	//load_char_tex();

        glGenTextures(1, &select_tex);
	glActiveTexture(GL_TEXTURE2); 
	glBindTexture(GL_TEXTURE_2D, select_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	selection_location = glGetUniformLocation(program, "Selection");
	selection_tex = (GLfloat*) malloc(sizeof(GLfloat)*chars_buffer_size);
	for(i = 0; i < INITIAL_CHARS_BUFFER_SIZE; ++i) selection_tex[i] = 0.;
	load_selection_tex();

	mouse_location = glGetUniformLocation(program, "Mouse");
	grid_location = glGetUniformLocation(program, "Grid");
	cursor_location = glGetUniformLocation(program, "Cursor");
	cursor_blink_location = glGetUniformLocation(program, "CursorBlink");
	glUniform1f(cursor_blink_location, (GLfloat) blink_state);
	grid_Y_offset_location = glGetUniformLocation(program, "GridYOffsset");
	glUniform1f(grid_Y_offset_location, (GLfloat) grid.y_offset);

	// INPUT
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwSetKeyCallback(window, key_callback);

	glfwSetCharCallback(window, character_callback);

	glfwSetCursorPosCallback(window, mouse_position_callback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetWindowSizeCallback(window, window_size_callback);

	// PARAS
	printf("%u\n", sizeof(GLfloat) * grid.full_size);
	top_paragraph_scroll = curr_paragraph = &paragraphs_head;
	paragraphs_head.buffer = (GLfloat*) malloc(sizeof(GLfloat) * PARA_BUFFER_SIZE);
	paragraphs_head.gap = (GLfloat*) malloc(sizeof(GLfloat) * PARA_BUFFER_SIZE);
	grid_lines = (void*) malloc(sizeof(PARA*) * INITIAL_GRID_PARAS);
			// REMOVE THIS
			for(i = 0; i < grid_paragraph_count; ++i) 
			{
				grid_lines[i] = NULL;
			}

#ifdef __EMSCRIPTEN__
	glfwGetWindowSize(window, &width, &height);
	window_size_callback(window, width, height);

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
