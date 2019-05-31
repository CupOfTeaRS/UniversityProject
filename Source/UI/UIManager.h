#pragma once

#include <sstream>>
#include <string>
using namespace std;

#include <d3d10.h>
#include <d3dx10.h>
#include "Defines.h"
#include "Camera.h"



namespace gen {


	// Get reference to global DirectX variables from another source file
	extern ID3D10Device*           g_pd3dDevice;
	extern IDXGISwapChain*         SwapChain;
	extern ID3D10DepthStencilView* DepthStencilView;
	extern ID3D10RenderTargetView* BackBufferRenderTarget;
	extern ID3DX10Font*            OSDFont;
	extern ID3DX10Font*            OSDFontLarge;
	extern ID3DX10Font*            OSDFontMedium;

	extern ID3D10Effect* Effect;

	// Actual viewport dimensions (fullscreen or windowed)
	extern TUInt32 ViewportWidth;
	extern TUInt32 ViewportHeight;



	class UIManager
	{
	private:
		TFloat32 updateTimer = 0.0f;
		TFloat32 defeatScreenDelay;
		TFloat32 defeatScreenDelayCounter = 0.0f;
		TFloat32 defeatArrowMoveCounter = 0.0f;
		TFloat32 ultMeterTimer = 0;
		TFloat32 ultMeterTimer2 = 0;
		int currentUltMaxFrame = 0;
	
		bool isDefeatScreenRendering = false;
		void RenderDefeatScreen(TFloat32 updateTime);
		void RenderPlayerUI(TFloat32 updateTime, bool isPlayerDead,bool isPlayerJotaro, bool isPlayerOnTheLeft);
	public:
		TUInt32 player1MaxHp = 0;
		TUInt32 player1Hp = 0;
		TUInt32 player2Hp = 0;
		TUInt32 player2MaxHp = 0;
		bool player1Jotaro;

		bool player2Jotaro;

		TFloat32 ultEnergyDrainMeterPlayer1;
		TFloat32 ultEnergyDrainMeterPlayer2;
		TFloat32 blockTimeRemainsPlayer1 ;
		TFloat32 blockTimeRemainsPlayer2;
		bool isCountDownToStart = false;
		bool isCountDownToStartFinished = false;
		TFloat32 countDownTimer = 0;
		UIManager();
		~UIManager();
		bool isPlayerDead = false;
		void CallForDefeatScreen(TFloat32 delay);
		void UpdateUI(TFloat32 updateTime);
		bool RenderMenu(TFloat32 updateTime);
		bool RenderModeSelect(TFloat32 updateTime);
		bool RenderPlayerSelect(TFloat32 updateTime);
		bool Player1Ready = false;
		bool Player2Ready = false;
		bool GameStart = false;
		int currentIntroFrame = 0;
		int currentModeSelectFrame = 0;
		int maxIntroFrames = 396;
		TFloat32 introTimer = 0;
		TFloat32 modeSelectTimer = 0;
		bool modeSelectBackwardCount = false;
		bool LockOnThisIntroFrame = false;
		bool IntroSkipped = false;
		bool RenderPressToContinue = false;
		bool RenderPressToModeSelect = false;
		bool RenderPressToPlayerSelect = false;
		bool RenderModeSelectMenu = false;
		bool RenderPlayerSelectMenu = false;
		
		void RenderText(const string& text, int X, int Y, float r, float g, float b,ID3DX10Font* font, bool centre = false);
		bool ChangeUIAnimFrame(const string& s_name,string fullfilename);
		bool CycleUltMeterReady(TFloat32 updateTime, bool isPlayerJotaro, bool isFacingRight);
		
	};

}//end of gen