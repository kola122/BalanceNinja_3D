//=============================================================================
//
// タイム処理 [timer.h]
// Author : 
//
//=============================================================================
#pragma once


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TIMER_DIGIT			(2)			// 桁数




//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTime(void);
void UninitTime(void);
void UpdateTime(void);
void DrawTime(void);

void AddTime(int add);
int GetTime(void);


