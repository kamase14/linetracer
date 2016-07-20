#include "h8-3069-int.h"
#include "h8-3069-iodef.h"
#include "lcd.h"
#include "timer.h"
#include "sci2.h"

#define AVE_COUNT  5
#define ERROR     50 // gosa
#define TURNLEFT   0
#define TURNRIGHT  1
#define STRAIGHT   2

void ad_init();
void ad_start(unsigned char ch, unsigned char int_sw);
void ad_scan(unsigned char ch_grp, unsigned char int_sw);
void ad_stop(void);
void int_adi(void);
int ad_status(void);

int adbuf[2][10];
int adbufcounter;

void ad_init()
     /* A/D 変換器を使うための初期化関数 */
{
  unsigned char dmy;

  ADCR = 0x7e;  /* 外部トリガ変換開始の禁止 */
  dmy = ADCSR;  /* 変換終了フラグクリアのためのダミーリード */
  ADCSR = 0x08; /* 単一スキャン, 割り込み禁止, A/D変換停止, */
                /* 134ステート変換時間, チャネル 0 選択     */
  adbufcounter = 0;
}

void ad_start(unsigned char ch, unsigned char int_sw)
     /* A/D変換をスタートさせる関数                                */
     /* ch:入力チャネル番号(0-7), int_sw:変換終了割り込み許可(0,1) */
     /* 割り込み許可の場合, 別途割り込みハンドラが必要             */
     /* また, CPUを割り込み許可状態にしないと割り込みは発生しない  */
     /* 割り込み処理には数usの時間がかかることを考慮すること       */
{
  if (int_sw != 0) int_sw = 0x40;
  else int_sw = 0;
  ADCSR = (ADCSR & 0xd8) | (ch & 0x07);   /* 変換停止, チャネルの書き換え */
  ADCSR = (ADCSR & 0x1f) | 0x20 | int_sw; /* エンドフラグクリア, 変換開始 */
}

void ad_scan(unsigned char ch_grp, unsigned char int_sw)
     /* A/D変換をスキャンモードでスタートさせる関数                */
     /* ch_grp:入力チャネルグループ指定(0,1)                       */
     /*   0:AN0-AN3 を自動的にスキャンして結果をレジスタに格納     */
     /*   1:AN4-AN7 を自動的にスキャンして結果をレジスタに格納     */
     /* int_sw:グループスキャン変換終了割り込み許可(0,1)           */
     /* 割り込み許可の場合, 別途割り込みハンドラが必要             */
     /* また, CPUを割り込み許可状態にしないと割り込みは発生しない  */
     /* 割り込み処理には数usの時間がかかることを考慮すること       */
{
  if (int_sw != 0) int_sw = 0x40; /* 変換終了時に割り込み通知 */
  else int_sw = 0;                /* 割り込みを使用しない     */
  if (ch_grp != 0) ch_grp = 7;    /* AN4-7の自動スキャン */
  else ch_grp = 3;                /* AN0-3の自動スキャン */
  ADCSR = ADCSR & 0x5f;           /* エンドフラグのクリアと変換の停止 */
  ADCSR = 0x38 | int_sw | ch_grp; /* 自動スキャンスタート */
}


void ad_stop(void)
     /* A/D変換を停止させる関数                  */
     /* 変換終了フラグをクリアし、変換停止させる */
{
  ADCSR = ADCSR & 0x5f;  /* A/Dエンドフラグのクリア, 変換停止 */
}


#pragma interrupt
void int_adi(){

  ad_stop();

  adbuf[0][adbufcounter] = ADDRBH;
  adbuf[1][adbufcounter] = ADDRCH;

  adbufcounter++;
  adbufcounter %= 10;

  ENINT();
}


int ad_status(void){
  int counter;
  int an1_ave,an2_ave;
  int r;
  int count;

  an1_ave = an2_ave = 0;

  for(counter=0; counter<AVE_COUNT; counter++){
    an1_ave += adbuf[0][(adbufcounter+counter)%10];
    an2_ave += adbuf[1][(adbufcounter+counter)%10];
  }

  an1_ave = an1_ave / AVE_COUNT;
  an2_ave = an2_ave / AVE_COUNT;

  an1_ave%=1000;
  an2_ave%=1000;

 if(an1_ave > 200 && an2_ave > 200){
   PBDR = 0x0F; //tomaru
 }else if(an1_ave/40 > an2_ave/40){
   PBDR = 0x0D; //migi
 }else if(an1_ave/40 < an2_ave/40){
   PBDR = 0x07; //hidari
 }else{
   PBDR = 0x05; //mae
 }

 lcd_cursor(0,0);
 lcd_printch('0' + an1_ave/100);
 lcd_cursor(1,0);
 lcd_printch('0' + (an1_ave - (an1_ave/100)*100)/10);
 lcd_cursor(2,0);
 lcd_printch('0' + an1_ave%10);

 lcd_cursor(0,1);
 lcd_printch('0' + an2_ave/100);
 lcd_cursor(1,1);
 lcd_printch('0' + (an2_ave - (an2_ave/100)*100)/10);
 lcd_cursor(2,1);
 lcd_printch('0' + an2_ave%10);
 
 count = 0;
 while(count/1000){
   count++;
 }

 PBDR = 0x00;
 PBDR = 0xFF;

 return 0;
}



