
#ifndef FTM_REPORT_SAVE_H
#define FTM_REPORT_SAVE_H
#include <sys/statfs.h>

typedef struct{
	char id;
	char value;
}report_save_struct;

extern int report_create(item_t *item, char ispcba);
extern int report_writeByID(int id, int value);
extern int report_readByID(int id);

extern int saveBoardTestSuccessFlag(int result);
extern int isBoardTestSuccess(char ispcba);


#endif




