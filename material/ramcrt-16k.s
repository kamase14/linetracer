/*   H8/3069版 ROM化用のスタートアップルーチン (2009/5/1 和崎) */
/*   .bss領域の 0 クリアはしない                               */
	.h8300h
	.section .text
	.global _start
/* 既に初期化はROM内で終わっているのでスタックをセットして終わり */
/* 注意!!!                                                        */
/* 初期化を全て行うと、DRAMのリフレッシュタイミングが途中で変わる */
/* ため、DRAM上のデータが壊れることがあるのでやってはいけない     */
_start:
/* 内蔵RAM上にスタックポインタをセット */
	mov.l   #_stack, sp    /* NMI対策、内蔵RAM：0xffd000 */
/* main()を呼び出す */
/*   実行が終了したらスリープ状態になる */
__go_main:
	jsr     @_main
	sleep
	bra     __go_main
