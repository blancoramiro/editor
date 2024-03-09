/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#include <stdio.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include "edit.h"
#include "scroll.h"
#include "global.h"

void scroll_bar_update()
{
	glUseProgram(program_scroll);
	if(lines_count < grid.height)
	{
		scroll_bar.top = 0;
		scroll_bar.bottom = window_size.height;
	}
	else
	{
		scroll_bar.top = window_size.height - window_size.height / lines_count * lines_scroll;
		scroll_bar.bottom = (lines_count - (int) grid.height - lines_scroll) * (window_size.height / lines_count);
		printf("ls: %u lc: %u bottom lc: %u chunk: %f top: %f bottom: %f\n", lines_scroll, lines_count, lines_count - (int) grid.height - lines_scroll, window_size.height / lines_count, scroll_bar.top, scroll_bar.bottom);
		
	}
	glUniform2fv(scroll_bar_location, 1, (GLfloat*) &scroll_bar);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cursor_reset();
//	printf("%u %f\n", lines_scroll, yoffset);
	lines_scroll += yoffset < 0 ? grid.height + lines_scroll < lines_count ? 1 : 0 : lines_scroll > 0 ? -1 : 0;
//	printf("%u %f\n", lines_scroll, yoffset);
	get_top_paragraph_scroll();
//	update_chars_tex();
//	load_selection_tex();
//	scroll_bar_update();
}
