/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#include <stdio.h> //this needs to go

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include "edit.h"
#include "linenumber.h"
#include "global.h"

inline void linenumber_update(unsigned int lines_count)
{
	unsigned int i;
	//unsigned int lines_decimals_count = 1;
	//lines_decimals_count = 1;

	glUseProgram(program_linenumber);

	glUniform1f(grid_Y_offset_location_linenumber, grid_y_offset);
	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);

	printf("top line: %d lines_count: %d decimals: %d\n", top_line_number, lines_count, lines_decimals_count);
	printf("_lines_count: %d decimals: %d %p\n", lines_count, lines_decimals_count, grid_lines[0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
}
