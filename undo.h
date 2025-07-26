/*
 *
 *  https://ramiroblan.co
 *  mail: r@miroblan.co
 *
 */

#define MAX_UNDO 20

typedef enum
{
	INSERT_CHAR,
	DELETE_CHAR,
	INSERT_PARA,
	DELETE_PARA 
}
UNDO_ACTION;

typedef struct
{
	PARA* paragraph;
	UNDO_ACTION action;	
}
UNDO;

UNDO undo_list[MAX_UNDO];

unsigned int undo_start, undo_end;
