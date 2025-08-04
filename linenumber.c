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

        PARA *paragraph_i  = top_paragraph_scroll;
        int i, j, k, m = 0; 

        lines_decimals_count = 0;

        i = top_line_number + grid.height; 
        do { ++lines_decimals_count; } while(i /= 10);

	for(i = lines_decimals_count; i >= 0; --i)
	{
		k = 0;
		for(j = i; j <= grid.height*lines_decimals_count; j += lines_decimals_count )
		{
			linenumbers_tex[j] = (++k%10+34.)/256.f;
		}
		printf("\n");
	}

//
//        i = paragraph_i->lines_count-lines_scroll_top_diff;        //if(i == paragraph_i->lines_count-lines_scroll_top_diff) for(j = lines_decimals_count; j > 0; --j ) linenumbers_tex[n++] = ((int)(top_line_number/(j*10))+34.)/256.f;
//        //else for(j = 0; j <= lines_decimals_count; ++j ) linenumbers_tex[n++] = 0.;
//        goto FIRST_LINE_CONT;
//
//        do
//        {
//                linenumbers_tex[m] = 0.;
//FIRST_LINE_CONT:
//                grid_lines[m] = paragraph_i;
//                line_numbers[m] = top_line_number;
//                ++m;
//                //++n;
//                --i;
//        }
//        while(i > 0);
//
//        i = top_line_number;
//
//        while(paragraph_i->next)
//        {
//                paragraph_i = paragraph_i->next;
//                ++i;
//                j = paragraph_i->lines_count;
//                //linenumbers_tex[m] = (i+34.)/256.f;
//                goto LINE_CONT;
//
//                do
//                {
//                        linenumbers_tex[m] = 0.;
//LINE_CONT:
//                        grid_lines[m] = paragraph_i;
//                        line_numbers[m] = i; 
//                        ++m;
//                        --j;
//                }
//                while(j > 0);
//        }
//
//        for(;m < grid.height; ++m) 
//        {
//                grid_lines[m] = NULL; //Make sure grid_paragraph_count is updated when array gets resized
//                line_numbers[m] = 0; 
//                //linenumbers_tex[m] = 0.;
//        }
//



	glUseProgram(program_linenumber);

	glUniform1f(grid_Y_offset_location_linenumber, grid_y_offset);
	glUniform4fv(grid_location_linenumber, 1, (GLfloat*) &grid);
	glUniform4f(grid_location_linenumber, lines_decimals_count, grid.height, grid.cell_width, grid.cell_height);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, linenumber_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, lines_decimals_count, grid.height, 0, GL_RED, GL_FLOAT, linenumbers_tex); // User glTexSubImage2D!

}
