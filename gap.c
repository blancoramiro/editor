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
		int i, j, buff_size;

		printf("MERGE_GAP");

		buff_size = curr_paragraph->buffer_count - curr_paragraph->gap_pos - curr_paragraph->gap_del;
		printf("buff size: %d\n", buff_size);

		buffer_aux = (GLfloat*) malloc(sizeof(GLfloat) * buff_size); /// Fix!!! remove malloc

		j = 0;

		for(i = curr_paragraph->gap_pos + curr_paragraph->gap_del; i < curr_paragraph->buffer_count; ++i) buffer_aux[j++] = curr_paragraph->buffer[i];

		for(i = 0; i < curr_paragraph->gap_count; ++i) curr_paragraph->buffer[curr_paragraph->gap_pos++] = curr_paragraph->gap[i];	

		for(i = 0; i < buff_size; ++i) curr_paragraph->buffer[curr_paragraph->gap_pos++] = buffer_aux[i];

		curr_paragraph->buffer_count = curr_paragraph->gap_pos;
		curr_paragraph->gap_count = 0;
		curr_paragraph->gap_pos = 0;
		curr_paragraph->gap_del = 0;

		free(buffer_aux);
	//}

}
