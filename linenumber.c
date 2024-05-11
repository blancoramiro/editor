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
	lines_decimals_count = 1;

	glUseProgram(program_linenumber);
	glUniform1f(grid_Y_offset_location_linenumber, (GLfloat) grid.y_offset);
	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);

	printf("lines_count: %d decimals: %d\n", lines_count, lines_decimals_count);
	while(lines_count /= 10) ++lines_decimals_count;
	printf("lines_count: %d decimals: %d\n", lines_count, lines_decimals_count);


	//grid_paragraph_count

}
