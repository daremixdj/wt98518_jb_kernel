#ifndef CUST_FM_H
#define CUST_FM_H

/*
FM BAND width
0x01: US/Europe band  87.5MHz ~ 108MHz
0x02: Japan band      76MHz   ~ 90MHz
0x03: Japan wideband  76MHZ   ~ 108MHz
*/
#define FM_BAND_WIDTH  0x01 

/*
FM Seek/scan width
0x01: 100KHz
0x02: 200KHz
*/
#define FM_SPACE_WIDTH   0x01

/*
FM factory mode test freq
Unit:100KHz
*/
#if 0//zhaoqf_sh delete 
uint16_t fm_freq_list[] = {875, 882, 1037, 1018};
#endif

#endif /* CUST_FM_H */
