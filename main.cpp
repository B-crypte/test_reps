#include "mbed.h"
#include "stepmtr_control.h"
#include "lcd_control.h"
#include "detect_sign.h"

InterruptIn bord_SW(USER_BUTTON);
InterruptIn SW_4(A4, PullUp);
DigitalIn SW1(A1, PullUp);
DigitalIn SW2(A2, PullUp);
DigitalIn SW3(A3, PullUp);

bool fg_stage;
bool fg_mesr;

RecDataType disp_data[MAX_DATA];

//モード切替１（stage操作の可否)
void change_mode1()
{
	if (!fg_stage) {
		fg_stage = true;
	}
	else {
		fg_stage = false;
	}
}
//モード切替２（時間の計測）
void change_mode2()
{
	if (!fg_mesr) {
		fg_mesr = true;
	}
	else {
		fg_mesr = false;
	}
}
//メイン関数
int main()
{
	char buf[20];	//lcd出力文字列バッファ
	double otp;		//stageの移動光路
	int mode = 0;	//モード

	//フラグ初期化
	fg_stage = false;
	fg_mesr = false;
	//ボタンによる割り込み設定
	bord_SW.rise(change_mode1);
	SW_4.rise(change_mode2);
	//初期化関数
	lcd_init();
	init_Stage();
	//lcdを用いて動作開始を明示
	lcd_locate(1, 0);
	sprintf(buf, "start");
	lcd_print(buf);
	wait(1);
	//メインループ
	while (1) {
		mode = 0;
		if (fg_stage & !fg_mesr) {  /* stageの位置調整 */
			mode = 1;
			if (!SW1) {
				if (ChackStateStage(STOP)) SetMoveStage(1);
			}
			if (!SW2) {
				if (ChackStateStage(STOP)) SetMoveStage(-1);
			}
			if (!SW3) {
				if (ChackStateStage(STOP)) ResetStagePos();
			}
		}
		if (fg_mesr & !fg_stage) { /* 計測 */
			mode = 2;
			if (!SW1) {			//計測開始
				Init_mesr();	//計測の初期化
				Rec_Enable();	//記録の許可
			}
			if (!SW2) {			//データをターミナルへ出力 
				Rec_Disable();	//記録を停止
				Disp_RecData();	//記録データの表示
			}	
			Disp_TimeVol();		//計測中の様子をデバッグ
		}
		//実行中のプロセスを表示
		otp = GetStagePos();
		lcd_locate(1, 0);
		sprintf(buf, "mode:%d", mode);
		lcd_print(buf);
		lcd_locate(2, 0);
		sprintf(buf, "var= %.4lfmm", otp);
		lcd_print(buf);
		wait(0.01);
	}
}