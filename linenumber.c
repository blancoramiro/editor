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
	GLfloat i = 0.;
	unsigned int 

	lines_decimals_count = 0;

	glUseProgram(program_linenumber);

	glUniform1f(grid_Y_offset_location_linenumber, grid_y_offset);
	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);

	printf("lines_count: %d decimals: %d\n", lines_count, lines_decimals_count);
	while(lines_count /= 10) ++lines_decimals_count;
	//for(; i < (lines_decimals_count+1)*grid.height; ++i)
	//for(i = lines_decimals_count; i < grid.height; i+=lines_decimals_count)
	for(i = lines_decimals_count; i < grid.height; ++i)
	{
		//printf("____%d %f\n", (int)i, linenumbers_tex[(int)i]);
		//linenumbers_tex[(int)i] = (i+45.)/256.f;
		linenumbers_tex[(int)i] = (i+35.)/256.f;
		//linenumbers_tex[(int)i] = 0.f;
		//++i;
		//linenumbers_tex[(int)i] = (52.)/256.f;
		//linenumbers_tex[(int)i] = 0.f;
		//++i;
		//++i;
		//linenumbers_tex[(int)i] = 0.f;
	}
	printf("_lines_count: %d decimals: %d %p\n", lines_count, lines_decimals_count, grid_lines[0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
}
