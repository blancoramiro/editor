/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_ES3
#include <GLFW/glfw3.h>

#include "edit.h"
#include "gap.h"
#include "scroll.h"
#include "global.h"

void merge_gap(void)
{

	GLfloat* buffer_aux;

	printf("MERGE_GAP");

	buffer_aux = (GLfloat*) malloc(sizeof(GLfloat) * curr_paragraph->gap_count + curr_paragraph->buffer_count);

//	for(i = curr_paragraph->gap_pos; curr_paragraph->gap_count; ++i)	
//
	free(buffer_aux);

}
