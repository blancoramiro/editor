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
	GLfloat i = 65.;

	lines_decimals_count = 1;

	glUseProgram(program_linenumber);

	glUniform1f(grid_Y_offset_location_linenumber, grid_y_offset);

	printf("lines_count: %d decimals: %d\n", lines_count, lines_decimals_count);
	while(lines_count /= 10) ++lines_decimals_count;
	for(; i < (lines_decimals_count+1)*grid.height; ++i)
	{
		linenumbers_tex[(int)i] = i/256.f;
		printf("____%d %f\n", (int)i, linenumbers_tex[(int)i]);
	}
	printf("lines_count: %d decimals: %d\n", lines_count, lines_decimals_count);

	//glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count+1, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid.width, grid_paragraph_count, GL_RED, GL_FLOAT, linenumbers_tex);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grid.width, 10, GL_RED, GL_FLOAT, linenumbers_tex);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid.width, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex);


	//grid_paragraph_count

}
