
#ifndef __BM101_H__
#define __BM101_H__



#define  BM101_RESET_PIN  22




typedef struct{
	int signal;
	int rate;
} BM101_HEART_BEAT_INFO;





// proto type define
void Init_BM101(void);
void bm101_process(void);
void parse_payload(BM101_HEART_BEAT_INFO* phb_info);


#endif


