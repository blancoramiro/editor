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
	if(curr_paragraph->gap_count)
	{

		GLfloat* buffer_aux;
		int i, buff_size;

		printf("MERGE_GAP");

		buff_size = curr_paragraph->buffer_count - curr_paragraph->gap_pos;

		buffer_aux = (GLfloat*) malloc(sizeof(GLfloat) * buff_size);
		printf("buff_size: %d\n\n", buff_size);
		printf("buff_count: %d\n\n", curr_paragraph->buffer_count);

		for(i = curr_paragraph->gap_pos; i < curr_paragraph->buffer_count; ++i) buffer_aux[i - curr_paragraph->gap_pos] = curr_paragraph->buffer[i];

		for(i = 0; i < curr_paragraph->gap_count; ++i) curr_paragraph->buffer[curr_paragraph->gap_pos++] = curr_paragraph->gap[i];	

		for(i = 0; i < buff_size; ++i) curr_paragraph->buffer[curr_paragraph->gap_pos++] = buffer_aux[i];

		curr_paragraph->buffer_count += curr_paragraph->gap_count;
		curr_paragraph->gap_count = 0;
		curr_paragraph->gap_pos = 0;
		curr_paragraph->gap_del = 0;

		free(buffer_aux);
	}

}
