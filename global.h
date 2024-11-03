/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

extern GLuint program_scroll,
       program_linenumber,
       grid_Y_offset_location,
       grid_Y_offset_location_linenumber,
       grid_location_linenumber,
       linenumber_tex,
       scroll_bar_location;

extern PARA *top_paragraph_scroll;

extern unsigned int lines_scroll_top_diff,
	lines_scroll_top_diff,
	lines_count,
	lines_decimals_count,
	lines_scroll;

extern GLfloat* linenumbers_tex;

extern GRID grid;

extern GLfloat grid_y_offset;

extern W_SIZE window_size;

//extern SCROLL_BAR scroll_bar;

extern PARA *curr_paragraph;

extern PARA **grid_lines;

extern GLfloat *line_numbers;

extern unsigned int top_line_number;
