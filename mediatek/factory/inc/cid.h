
#ifndef __HTC_CID
#define __HTC_CID



int cid_modify(char *cid);

int cid_draw_text(int isSuccess, char *cid);
int cid_draw_item(int isSuccess, char *cid);
int cid_draw_device_info();
int get_current_cid(char *current_cid);
int cid_read_nv(char *cid);
#endif


