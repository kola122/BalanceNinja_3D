//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "model.h"
#include "tester.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "timer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER			"data/MODEL/body.obj"			// 読み込むモデル名

// 9 Parts
#define	MODEL_PLAYER_HEAD		"data/MODEL/head.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_LEFT_ARM	"data/MODEL/larm.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_LEFT_HAND	"data/MODEL/lhand.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_LEFT_LEG	"data/MODEL/lleg.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_LEFT_FOOT	"data/MODEL/lfoot.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT_ARM	"data/MODEL/rarm.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT_HAND	"data/MODEL/rhand.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT_LEG	"data/MODEL/rleg.obj"			// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT_FOOT	"data/MODEL/rfoot.obj"			// 読み込むモデル名


#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(17.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(9)								// プレイヤーのパーツの数



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static TESTER		g_Tester;						// プレイヤー

static TESTER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用


static float		g_RotationForce;

static float		g_WindForce;
static float		g_WindForceMax;


static BOOL			WindChanged;

static LIGHT		g_Light;


// プレイヤーの階層アニメーションデータ


// プレイヤーの頭を左右に動かしているアニメデータ
static INTERPOLATION_DATA move_tbl_left_arm[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};
static INTERPOLATION_DATA move_tbl_left_hand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(-5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};

static INTERPOLATION_DATA move_tbl_right_arm[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(5.5f, 26.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};
static INTERPOLATION_DATA move_tbl_right_hand[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, XM_2PI / 6), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	{ XMFLOAT3(5.5f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
};

static INTERPOLATION_DATA* g_MoveTblAdr[] =
{
	move_tbl_left_arm,
	move_tbl_left_hand,
	move_tbl_right_arm,
	move_tbl_right_hand,
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitTester(void)
{
	g_Tester.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Tester.model);

	g_Tester.pos = XMFLOAT3(0.0f, PLAYER_OFFSET_Y, 0.0f);
	g_Tester.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Tester.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Tester.spd = 0.0f;			// 移動スピードクリア

	g_Tester.use = TRUE;			// TRUE:生きてる
	g_Tester.size = PLAYER_SIZE;	// 当たり判定の大きさ

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Tester.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Tester.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号


	g_RotationForce = 0.0f;

	g_WindForce = 0.0f;
	g_WindForceMax = 0.0f;

	g_Tester.parent = NULL;			// 本体（親）なのでNULLを入れる


	// 階層アニメーションの初期化
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = FALSE;

		// 位置・回転・スケールの初期設定
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// 親子関係
		g_Parts[i].parent = &g_Tester;		// ← ここに親のアドレスを入れる
		//	g_Parts[腕].parent= &g_Tester;		// 腕だったら親は本体（プレイヤー）
		//	g_Parts[手].parent= &g_Paerts[腕];	// 指が腕の子供だった場合の例

			// 階層アニメーション用のメンバー変数の初期化
		g_Parts[i].time = 0.0f;			// 線形補間用のタイマーをクリア
		g_Parts[i].tblNo = 0;			// 再生する行動データテーブルNoをセット
		g_Parts[i].tblMax = 0;			// 再生する行動データテーブルのレコード数をセット

		// パーツの読み込みはまだしていない
		g_Parts[i].load = FALSE;
	}

	g_Parts[0].use = TRUE;
	g_Parts[0].parent = &g_Tester;	// 親をセット
	//g_Parts[0].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
	//g_Parts[0].tblMax = sizeof(move_tbl_left) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	//g_Parts[0].load = TRUE;
	LoadModel(MODEL_PLAYER_HEAD, &g_Parts[0].model);

	g_Parts[1].use = TRUE;
	g_Parts[1].parent = &g_Tester;	// 親をセット
	g_Parts[1].tblNo = 0;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[1].tblMax = sizeof(move_tbl_left_arm) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[1].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_ARM, &g_Parts[1].model);


	g_Parts[2].use = TRUE;
	g_Parts[2].parent = &g_Parts[1];	// 親をセット
	g_Parts[2].tblNo = 1;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[2].tblMax = sizeof(move_tbl_left_hand) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[2].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_HAND, &g_Parts[2].model);

	g_Parts[3].use = TRUE;
	g_Parts[3].parent = &g_Tester;	// 親をセット
	LoadModel(MODEL_PLAYER_LEFT_LEG, &g_Parts[3].model);

	g_Parts[4].use = TRUE;
	g_Parts[4].parent = &g_Tester;	// 親をセット
	LoadModel(MODEL_PLAYER_LEFT_FOOT, &g_Parts[4].model);


	g_Parts[5].use = TRUE;
	g_Parts[5].parent = &g_Tester;	// 親をセット
	g_Parts[5].tblNo = 2;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[5].tblMax = sizeof(move_tbl_right_arm) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[5].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_ARM, &g_Parts[5].model);

	g_Parts[6].use = TRUE;
	g_Parts[6].parent = &g_Parts[5];	// 親をセット
	g_Parts[6].tblNo = 3;			// 再生するアニメデータの先頭アドレスをセット
	g_Parts[6].tblMax = sizeof(move_tbl_right_hand) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット
	g_Parts[6].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_HAND, &g_Parts[6].model);

	g_Parts[7].use = TRUE;
	g_Parts[7].parent = &g_Tester;	// 親をセット
	LoadModel(MODEL_PLAYER_RIGHT_LEG, &g_Parts[7].model);

	g_Parts[8].use = TRUE;
	g_Parts[8].parent = &g_Tester;	// 親をセット
	LoadModel(MODEL_PLAYER_RIGHT_FOOT, &g_Parts[8].model);



	// クォータニオンの初期化
	XMStoreFloat4(&g_Tester.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitTester(void)
{
	// モデルの解放処理
	if (g_Tester.load == TRUE)
	{
		UnloadModel(&g_Tester.model);
		g_Tester.load = FALSE;
	}

	// パーツの解放処理
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		if (g_Parts[i].load == TRUE)
		{
			// パーツの解放処理
			UnloadModel(&g_Parts[i].model);
			g_Parts[i].load = FALSE;
		}
	}



}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateTester(void)
{
	CAMERA* cam = GetCamera();

	//g_Tester.spd *= 0.7f;

	//if (g_RotationForce >= 0.0f && g_RotationForce <  0.001f) g_RotationForce =  0.001f;
	//if (g_RotationForce <= 0.0f && g_RotationForce > -0.001f) g_RotationForce = -0.001f;

	// 移動処理
	if (GetKeyboardPress(DIK_LEFT))
	{
		g_RotationForce -= 0.002f;
	}
	else if (GetKeyboardPress(DIK_RIGHT))
	{
		g_RotationForce += 0.002f;

	}
	else
	{
		g_RotationForce *= 0.7f;
	}

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Tester.pos.z = g_Tester.pos.x = 0.0f;
		g_Tester.spd = 0.0f;
		//roty = 0.0f;
	}
#endif


	//Wind処理
	{
		if (GetTime() % 2 == 0)
		{
			if (!WindChanged)
			{
				g_WindForce = rand() % 10 * GetTime();
				g_WindForce /= 1000;

				WindChanged = TRUE;
			}
		}
		else
		{
			WindChanged = FALSE;
		}
	}


	//Rotation処理
	{
		float playerRotation = g_RotationForce + g_WindForce;

		g_Tester.rot.z += playerRotation;
	}



	//g_RotationForce *= 0.7;

	//// 影もプレイヤーの位置に合わせる
	//XMFLOAT3 pos = g_Tester.pos;
	//pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	//SetPositionShadow(g_Tester.shadowIdx, pos);


	// 階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// 使われているなら処理する
		if ((g_Parts[i].use == TRUE) && (g_Parts[i].tblMax > 0))
		{	// 線形補間の処理
			int nowNo = (int)g_Parts[i].time;			// 整数分であるテーブル番号を取り出している
			int maxNo = g_Parts[i].tblMax;				// 登録テーブル数を数えている
			int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
			INTERPOLATION_DATA* tbl = g_MoveTblAdr[g_Parts[i].tblNo];	// 行動テーブルのアドレスを取得

			XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
			XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
			XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

			XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
			XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
			XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

			float nowTime = g_Parts[i].time - nowNo;	// 時間部分である少数を取り出している

			Pos *= nowTime;								// 現在の移動量を計算している
			Rot *= nowTime;								// 現在の回転量を計算している
			Scl *= nowTime;								// 現在の拡大率を計算している

			// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
			XMStoreFloat3(&g_Parts[i].pos, nowPos + Pos);

			// 計算して求めた回転量を現在の移動テーブルに足している
			XMStoreFloat3(&g_Parts[i].rot, nowRot + Rot);

			// 計算して求めた拡大率を現在の移動テーブルに足している
			XMStoreFloat3(&g_Parts[i].scl, nowScl + Scl);

			// frameを使て時間経過処理をする
			g_Parts[i].time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
			if ((int)g_Parts[i].time >= maxNo)			// 登録テーブル最後まで移動したか？
			{
				g_Parts[i].time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
			}

		}

	}


	// ポイントライトのテスト
	{
		LIGHT* light = GetLightData(1);
		XMFLOAT3 pos = g_Tester.pos;
		pos.y += 60.0f;

		light->Position = pos;
		light->Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		light->Type = LIGHT_TYPE_POINT;
		light->Enable = TRUE;
		SetLightData(1, light);
	}


#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("Player X:%f Y:%f Z:% N:%f\n", g_Tester.pos.x, g_Tester.pos.y, g_Tester.pos.z/*, Normal.y*/);
	PrintDebugProc("Wind Force:%f\n", g_WindForce * 1000);
	PrintDebugProc("Rotation Force:%f\n", g_RotationForce * 1000);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawTester(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Tester.scl.x, g_Tester.scl.y, g_Tester.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Tester.rot.x, g_Tester.rot.y + XM_PI, g_Tester.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	//// クォータニオンを反映
	//quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Tester.Quaternion));
	//mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Tester.pos.x, g_Tester.pos.y, g_Tester.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Tester.mtxWorld, mtxWorld);


	// 縁取りの設定
	//SetFuchi(1);

	// モデル描画
	DrawModel(&g_Tester.model);



	// 階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		if (g_Parts[i].parent != NULL)	// 子供だったら親と結合する
		{
			mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
			// ↑
			// g_Tester.mtxWorldを指している
		}

		XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

		// 使われているなら処理する
		if (g_Parts[i].use == FALSE) continue;

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);


		// モデル描画
		DrawModel(&g_Parts[i].model);

	}

	//SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
TESTER* GetTester(void)
{
	return &g_Tester;
}

