#include "UIManager.h"
#include "EntityManager.h"
#include "AnimationManager.h"
#include "Camera.h"
#include <chrono>
#include <thread>
#include <future>
#include "FMODManager.h"
using namespace std;

/********************************************************************
INTERFACE MANAGER

Is responsible for all the menus, indications, Rendering Text  functions and overall screen info.
Main menu and non-player non-scene objects are stored and used through here
*********************************************************************/
namespace gen {


	extern CEntityManager EntityManager;
	extern CCamera* MainCamera;
	extern CAnimationManager AnimationManager;
	extern const string MediaFolder;
	extern FMODManager SoundManager;
	UIManager::UIManager()
	{
		 ultEnergyDrainMeterPlayer1 = 0.0f;
		 blockTimeRemainsPlayer1 = 0.0f;
	}


	UIManager::~UIManager()
	{
	}
	//Rendering UI for two separate players,and if one is dead the continue screen
	void UIManager::UpdateUI(TFloat32 updateTime)
	{
		if (isDefeatScreenRendering)
			RenderDefeatScreen(updateTime);

		RenderPlayerUI(updateTime, isPlayerDead,player1Jotaro,true);
		RenderPlayerUI(updateTime, isPlayerDead, player2Jotaro, false);
	}
	void UIManager::RenderDefeatScreen(TFloat32 updateTime)
	{
		//The arrow that flies in the end with brown background
		defeatScreenDelayCounter += updateTime;

		EntityManager.GetEntity("TBCScreen")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX(), MainCamera->Matrix().GetY(), MainCamera->Matrix().GetZ() + 80));
		EntityManager.GetEntity("TBCScreen")->Matrix().FaceDirection(MainCamera->Matrix().ZAxis());
		if (defeatScreenDelay < defeatScreenDelayCounter && defeatScreenDelayCounter < 4.5)
		{
			EntityManager.GetEntity("TBCArrow")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() + (50 - defeatArrowMoveCounter * 20), MainCamera->Matrix().GetY() - 5, MainCamera->Matrix().GetZ() + 15));
			defeatArrowMoveCounter += updateTime;
		}

	}
	void UIManager::CallForDefeatScreen(TFloat32 delay)
	{
		defeatScreenDelay = delay;
		isDefeatScreenRendering = true;
	}
	//Rendering text 
	void UIManager::RenderText(const string& text, int X, int Y, float r, float g, float b, ID3DX10Font* font, bool centre)
	{
		RECT rect;
		if (!centre)
		{
			SetRect(&rect, X, Y, 0, 0);
			font->DrawText(NULL, text.c_str(), -1, &rect, DT_NOCLIP, D3DXCOLOR(r, g, b, 1.0f));
		}
		else
		{
			SetRect(&rect, X - 100, Y, X + 100, 0);
			font->DrawText(NULL, text.c_str(), -1, &rect, DT_CENTER | DT_NOCLIP, D3DXCOLOR(r, g, b, 1.0f));
		}
	}

	//Rendering the UI,namely:
	//                        Faces, which become bloody if taken damage
	//                        HP bars, which show how many blocks are available
	//                        Ult Meters, which show how many abilities can a Stando use and if ultimate is available
	void UIManager::RenderPlayerUI(TFloat32 updateTime, bool isPlayerDead, bool isPlayerJotaro, bool isPlayerOnTheLeft)
	{
		if (!isCountDownToStartFinished && isCountDownToStart)
		{
			countDownTimer += updateTime * 12 ;
			if (countDownTimer > 5)
			{
				isCountDownToStartFinished = true;
				EntityManager.CountDownToStart = 15;
			}
			
		}
		//Player1 Ui
		if (!isPlayerDead && isPlayerOnTheLeft )
		{
			EntityManager.GetEntity("PlayerFaceLeft")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() - 1.00f, MainCamera->Matrix().GetY() + 0.62f, MainCamera->Matrix().GetZ() + 2));
			EntityManager.GetEntity("PlayerHpBarLeft")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() - 1.64f, MainCamera->Matrix().GetY() + 1.5f, MainCamera->Matrix().GetZ() + 5));
			EntityManager.GetEntity("PlayerUltMeterLeft")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() - 2.1f, MainCamera->Matrix().GetY() - 2.25f, MainCamera->Matrix().GetZ() + 5));
		}
		//Player2 UI
		else if (!isPlayerDead && !isPlayerOnTheLeft )
		{
			EntityManager.GetEntity("PlayerFaceRight")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() + 1.00f, MainCamera->Matrix().GetY() + 0.62f, MainCamera->Matrix().GetZ() + 2));
			EntityManager.GetEntity("PlayerHpBarRight")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() + 1.64f, MainCamera->Matrix().GetY() + 1.5f, MainCamera->Matrix().GetZ() + 5));
			EntityManager.GetEntity("PlayerUltMeterRight")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX() + 2.1f, MainCamera->Matrix().GetY() - 2.25f, MainCamera->Matrix().GetZ() + 5));
		}
		//If either dies, we stop rendering the UI
		else if(isPlayerDead || EntityManager.DoubleUltCollisionTimer != 0.0f|| EntityManager.player1LifeLeft < 0 || EntityManager.player2LifeLeft < 0)
		{
			EntityManager.GetEntity("PlayerFaceLeft")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
			EntityManager.GetEntity("PlayerHpBarLeft")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
			EntityManager.GetEntity("PlayerUltMeterLeft")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
			EntityManager.GetEntity("PlayerFaceRight")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
			EntityManager.GetEntity("PlayerHpBarRight")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
			EntityManager.GetEntity("PlayerUltMeterRight")->Matrix().SetPosition(CVector3(-10000, -10000, -1000));
		}
	}
	bool UIManager::ChangeUIAnimFrame(const string& s_name, string fullfilename)
	{
		string fullFileName = MediaFolder + fullfilename;
		EntityManager.GetEntity(s_name)->Mesh->m_Materials->textures[0]->Release();
		if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &EntityManager.GetEntity(s_name)->Mesh->m_Materials->textures[0], NULL)))
		{
			string errorMsg = "Error loading texture " + fullFileName;
			SystemMessageBox(errorMsg.c_str(), "Mesh Error");
			return false;
		}
		return true;
	}
	//A function that shows visually of ultimate being ready
	bool UIManager::CycleUltMeterReady(TFloat32 updateTime,bool isPlayerJotaro, bool isRight)
	{
		ultMeterTimer += updateTime * 200;
		if (ultMeterTimer > 20)
		{
			currentUltMaxFrame++;
			if (currentUltMaxFrame > 3)
			{
				currentUltMaxFrame = 0;
			}
			string sName;
			if (isRight)
			{
				sName = "PlayerUltMeterLeft";
			}
			else
			{
				sName = "PlayerUltMeterRight";
			}
			string fullFileName = MediaFolder + AnimationManager.GetPlayerUITexturePath(Ult_Meter_Ready_Anim, currentUltMaxFrame, isPlayerJotaro, isRight);
			EntityManager.GetEntity(sName)->Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &EntityManager.GetEntity(sName)->Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}
			ultMeterTimer = 0;
		}
    }
	//The whole meni sequence is performed by this function
	bool UIManager::RenderMenu(TFloat32 updateTime)
	{
		introTimer += updateTime;
		if (introTimer >= 0.095f && !LockOnThisIntroFrame)
		{
			if(IntroSkipped)
			{
				currentIntroFrame = 320;
				RenderPressToContinue = true;
			 }
			//INTRO VIDEO
			//Is not actually a video, but many many separate video frames rendered to texture, which is positioned to fully cover the camera view
			//Unfortunately DirectX 10 is not capable of many things, and video API is one of them
			string fullFileName = MediaFolder + AnimationManager.GetMenuUIAnimFrame(currentIntroFrame, IntroAnim);
			EntityManager.GetEntity("IntroVideo")->Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &EntityManager.GetEntity("IntroVideo")->Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}
			currentIntroFrame++;
			if (currentIntroFrame >=386 || IntroSkipped)
			{
				currentIntroFrame = 320;
				string fullFileName = MediaFolder + AnimationManager.GetMenuUIAnimFrame(currentIntroFrame, IntroAnim);
				EntityManager.GetEntity("IntroVideo")->Mesh->m_Materials->textures[0]->Release();
				if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &EntityManager.GetEntity("IntroVideo")->Mesh->m_Materials->textures[0], NULL)))
				{
					string errorMsg = "Error loading texture " + fullFileName;
					SystemMessageBox(errorMsg.c_str(), "Mesh Error");
					return false;
				}
				RenderPressToContinue = true;
				LockOnThisIntroFrame = true;
			}
			introTimer = 0;
		}
		if (RenderPressToContinue)
		{
			if (KeyHit(Key_Space) || KeyHit(Key_Numpad0))
			{
				RenderPressToModeSelect = true;
				RenderModeSelectMenu = true;
				EntityManager.GetEntity("ModeSelect")->Matrix().SetPosition(CVector3(0, 48.5, -145));
				RenderPressToContinue = false;
			}
			
		}
		if (RenderModeSelectMenu)
		{
			return RenderModeSelect(updateTime);
		}
		if (RenderPlayerSelectMenu)
		{
			return RenderPlayerSelect(updateTime);
		}
		if (introTimer >= 0.5f && RenderPressToContinue)
		{

			if (introTimer > 1.0f)
			{
				introTimer = 0;
			}
		}
		if  (KeyHit(Key_Space) || KeyHit(Key_Numpad0))
		{
			if (!IntroSkipped)
			{
				SoundManager.ClearAllSounds();
				IntroSkipped = true;
				return false;
			}
			
			
			
		}
			
		else
			return false;
	}
	//Mode select, now we can only choose 1 VS 1
	bool UIManager::RenderModeSelect(TFloat32 updateTime)
	{
		if (KeyHit(Key_Space) || KeyHit(Key_Numpad0))
		{
			RenderPlayerSelectMenu = true;
			RenderPressToModeSelect = false;
			RenderPressToPlayerSelect = true;
			RenderModeSelectMenu = false;
			EntityManager.GetEntity("ModeSelect")->Matrix().SetX(1000000);
			EntityManager.GetEntity("JotaroSelect")->Matrix().SetPosition(CVector3(-2, 49, -145));
			EntityManager.GetEntity("DioSelect")->Matrix().SetPosition(CVector3(2, 49, -145));
		}
		modeSelectTimer += updateTime;
		if (modeSelectTimer >= 0.25)
		{
			string fullFileName = MediaFolder + AnimationManager.GetMenuUIAnimFrame(currentModeSelectFrame, ModeSelectAnim);
			EntityManager.GetEntity("ModeSelect")->Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &EntityManager.GetEntity("ModeSelect")->Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}
			if (currentModeSelectFrame == 0)
				currentModeSelectFrame = 1;
			else
				currentModeSelectFrame = 0;
			modeSelectTimer = 0;
		}
		return false;

		
	}
	//Player select, to show which player is on which side
	bool UIManager::RenderPlayerSelect(TFloat32 updateTime)
	{
		if (KeyHit(Key_Space))
		{
			Player1Ready = true;
			SoundManager.PlayPlayerSound(YareYareTauntSound, false, true);
		}
		if (KeyHit(Key_Numpad0))
		{
			Player2Ready = true;
			SoundManager.PlayPlayerSound(KonoDioDaSound, false, false);
		}
		if (Player1Ready && Player2Ready)
		{
			Player1Ready = false;
			Player2Ready = false;
			RenderPressToPlayerSelect = false;
			GameStart = true;
			EntityManager.GetEntity("JotaroSelect")->Matrix().SetPosition(CVector3(-2, 49, -145000000));
			EntityManager.GetEntity("DioSelect")->Matrix().SetPosition(CVector3(2, 49, 145000000));
			EntityManager.GetEntity("IntroVideo")->Matrix().SetPosition(CVector3(0, -100000, 0));
			SoundManager.PlayGlobal(VocalPercussionSound, false);
			return true;
		}
		return false;
	}
}//end of gen