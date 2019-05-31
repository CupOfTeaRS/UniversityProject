/*******************************************
	Materials.cpp

	Main scene and game functions
********************************************/

#include <sstream>
#include <string>
using namespace std;

#include <d3d10.h>
#include <d3dx10.h>
#include "Defines.h"
#include "CVector3.h"
#include "Camera.h"
#include "Light.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "CParseLevel.h"
#include "CParseAnimation.h"
#include "Materials.h"
#include "FMODManager.h"
#include "UIManager.h"
//#include "vld.h"
namespace gen
{

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// Control speed
const float CameraRotSpeed = 2.0f;
float CameraMoveSpeed = 800.0f;

// Amount of time to pass before calculating new average update time
const float UpdateTimePeriod = 1.0f;
float DisclaimerTimer = 0.0f;
float VocalTimer = 0.0f;
bool beginVocalCountdown = false;
float currentTime = 0.0f;
bool ReadyToPlay = false;
bool MenuIntroPlayed = false;
//-----------------------------------------------------------------------------
// Global system variables
//-----------------------------------------------------------------------------

// Get reference to global DirectX variables from another source file
extern ID3D10Device*           g_pd3dDevice;
extern IDXGISwapChain*         SwapChain;
extern ID3D10DepthStencilView* DepthStencilView;
extern ID3D10RenderTargetView* BackBufferRenderTarget;
extern ID3DX10Font*            OSDFont;

extern ID3D10Texture2D*        ShadowMap ;

extern ID3D10Effect* Effect ;

extern ID3D10DepthStencilView* ShadowDepthStencilView;
extern ID3D10ShaderResourceView* ShadowMapResourceView;

extern ID3D10RenderTargetView* ShadowBufferRenderTarget;
extern int ShadowMapSize ;
// Actual viewport dimensions (fullscreen or windowed)
extern TUInt32 ViewportWidth;
extern TUInt32 ViewportHeight;
// Current mouse position
extern TUInt32 MouseX;
extern TUInt32 MouseY;

// Messenger class for sending messages to and between entities
extern CMessenger Messenger;


//-----------------------------------------------------------------------------
// Global game/scene variables
//-----------------------------------------------------------------------------

// Entity manager and level parser
CEntityManager EntityManager;
CAnimationManager AnimationManager;
UIManager InterfaceManager;
CParseLevel LevelParser( &EntityManager );

CParseAnimation AnimParser(&AnimationManager);
FMODManager SoundManager;
bool isGameMode1VS1 = true;
bool isPlayer1Taken = false;
// Other scene elements
const int NumLights = 2;
CLight*  Lights[NumLights];
CCamera* MainCamera;
CCamera* ShadowViewCamera;
ERenderMethod cameraViewMethod = PlainTexture;
bool SkipSetupStep = false;
//Sound testing 


// Sum of recent update times and number of times in the sum - used to calculate
// average over a given time period
float SumUpdateTimes = 0.0f;
int NumUpdateTimes = 0;
float AverageUpdateTime = -1.0f; // Invalid value at first
float currentHpToShowPlayer1 = 190.0f;
float currentHpToShowPlayer2 = 210.0f;
//-----------------------------------------------------------------------------
// Game Constants
//-----------------------------------------------------------------------------

// Lighting
const SColourRGBA AmbientColour( 0.15f, 0.15f, 0.15f, 0.0f );
D3DXVECTOR4 BackgroundColour = D3DXVECTOR4(0.2f, 0.2f, 0.3f, 1.0f) ;
CVector3 LightCentre( 0.0f, -3099.0f, 50.0f );
const float LightOrbit = 170.0f;
const float LightOrbitSpeed = 0.2f;
const float dmgTimeFreezeMax = UpdateTimePeriod;
float dmgTimeFreezeTimer = 0.0f;
bool isStoppingTime = false;

bool TimeStopper(TFloat32 updateTime);
//-----------------------------------------------------------------------------
// Scene management
//-----------------------------------------------------------------------------

// Creates the scene geometry
bool SceneSetup()
{
	//////////////////////////////////////////////
	// Prepare render methods
	if (!SkipSetupStep)
	{
		//Initialization of methods for rendering
		InitialiseMethods();
		//Setting up both view camera and shadow drop camera 
		MainCamera = new CCamera(CVector3(0.0f, 50, -150), CVector3(ToRadians(4), 0, 0));
		MainCamera->SetNearFarClip(2.0f, 300000.0f);
		ShadowViewCamera = new CCamera(CVector3(400.0f, 1400.0f, 1900.0f), CVector3(0, 0, 0));
		ShadowViewCamera->SetNearFarClip(20.0f, 300000.0f);
		ShadowViewCamera->Matrix().FaceTarget(CVector3(400.0f, 0, 0));
		// Sunlight
		Lights[0] = new CLight(CVector3(130000.0f, 90000.0f, 250000.0f), SColourRGBA(0.5f, 0.5f, 0.2f) * 1200000, 1200000.0f); // Colour is multiplied by light brightness

		//forgot to delete 																													   // Light orbiting area
		Lights[1] = new CLight(CVector3(-19990.0f, 0.0f, 0.0f), SColourRGBA(0.0f, 0.2f, 1.0f) * 50, 100.0f);
		SkipSetupStep = true;
	}
	


	
	//////////////////////////////////////////////
	// Read templates and entities from XML file
	//The animation parser is the most important part of the project as it reads and imports all the movements of sprites you will see
	AnimParser.ParseFile("Animations.xml");
	LevelParser.ParseFile( "Entities.xml" );

	

	

	return true;
}


// Release everything in the scene
void SceneShutdown()
{
	// Release render methods
	ReleaseMethods();

	// Release lights
	for (int light = NumLights - 1; light >= 0; --light)
	{
		delete Lights[light];
	}

	// Release camera
	delete MainCamera;

	// Destroy all entities
	EntityManager.DestroyAllEntities();
	EntityManager.DestroyAllTemplates();
}


//-----------------------------------------------------------------------------
// Game loop functions
//-----------------------------------------------------------------------------

void PreRenderScene()
{
	EntityManager.PreRenderAllEntities();
}
// Draw one frame of the scene
void RenderScene( float updateTime )
{
	// Setup the viewport - defines which part of the back-buffer we will render to (usually all of it)
	D3D10_VIEWPORT vp;

	vp.Width = ShadowMapSize;
	vp.Height = ShadowMapSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports(1, &vp);

	g_pd3dDevice->OMSetRenderTargets(0, 0, ShadowDepthStencilView);
	g_pd3dDevice->ClearDepthStencilView(ShadowDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0);

	// Update camera aspect ratio based on viewport size - for better results when changing window size
	ShadowViewCamera->SetAspect(static_cast<TFloat32>(ViewportWidth) / ViewportHeight);
	ShadowViewCamera->CalculateMatrices();
	SetCamera(ShadowViewCamera);
	EntityManager.ShadowRenderAllEntities();

	vp.Width  = ViewportWidth;
	vp.Height = ViewportHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	g_pd3dDevice->RSSetViewports( 1, &vp );


	
	// Select the back buffer and depth buffer to use for rendering
	g_pd3dDevice->OMSetRenderTargets( 1, &BackBufferRenderTarget, DepthStencilView );
	
	// Clear previous frame from back buffer and depth buffer
	g_pd3dDevice->ClearRenderTargetView( BackBufferRenderTarget, &BackgroundColour[0] );
	g_pd3dDevice->ClearDepthStencilView( DepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );

	


	
	
	 //Set camera and light data in shaders
	MainCamera->SetAspect(static_cast<TFloat32>(ViewportWidth) / ViewportHeight);
	MainCamera->CalculateMatrices();
	
	SetAmbientLight( AmbientColour );
	SetLights( &Lights[0] );
	
	// Render entities and draw on-screen text
	
		SetCamera(MainCamera);
		EntityManager.BucketRenderAllEntities();
		EntityManager.CollisionCalculator();
	
	

	
	//Render shadow map to main window 

	
	RenderSceneText( updateTime );

	//Effect->GetVariableByName("ShadowMap")->AsShaderResource()->SetResource(0);
    // Present the backbuffer contents to the display
	SwapChain->Present( 0, 0 );
}


// Render a single text string at the given position in the given colour, may optionally centre it


// Render on-screen text each frame
void RenderSceneText( float updateTime )
{
	// Accumulate update times to calculate the average over a given period
	SumUpdateTimes += updateTime;
	++NumUpdateTimes;
	if (SumUpdateTimes >= UpdateTimePeriod)
	{
		AverageUpdateTime = SumUpdateTimes / NumUpdateTimes;
		SumUpdateTimes = 0.0f;
		NumUpdateTimes = 0;
	}
	currentTime += updateTime;
	// Write FPS text string
	/*

	/////////////THE TEXT FOR THE APPLICATION IS RENDERED THROUGH HERE

	As this is the main point for the program, it gets access to text update and renders all the necessary texts when they are needed
	*/
	//Initial menu indicators
	if (InterfaceManager.introTimer > 0.5f && InterfaceManager.RenderPressToContinue)
	{
		InterfaceManager.RenderText("PRESS START TO CONTINUE", 420, 800, 1.0f, 0.1f, 0.1f, OSDFontLarge);
	}
	if (InterfaceManager.RenderPressToModeSelect)
	{
		InterfaceManager.RenderText("PLAYER VS PLAYER", 480, 260, 0.1f, 0.1f, 0.1f, OSDFontLarge);
	}
	if (InterfaceManager.RenderPressToPlayerSelect)
	{
		InterfaceManager.RenderText("PLAYER 1", 60, 360, 0.1f, 0.1f, 1.0f, OSDFontLarge);
		InterfaceManager.RenderText("PLAYER 2", ViewportWidth - 200, 360, 1.0f, 0.1f, 0.1f, OSDFontLarge);
	}
	if (InterfaceManager.Player1Ready)
	{
		InterfaceManager.RenderText("READY", 60, 760, 0.1f, 0.1f, 1.0f, OSDFontLarge);
	}
	if (InterfaceManager.Player2Ready)
	{
		InterfaceManager.RenderText("READY",ViewportWidth - 200, 760, 1.0f, 0.1f, 0.1f, OSDFontLarge);
	}
	//At the beginning of each round this counter delays the fierce attacks of the opponent and lets you to get you concentration
	if (EntityManager.CountDownToStart > 0 && !EntityManager.DoubleUltCollisionEventFinish)
	{
		EntityManager.CountDownToStart -= updateTime * 5 ;
		InterfaceManager.RenderText(to_string((int)EntityManager.CountDownToStart), 630, 50, 1.0f, 1.0f, 1.0f, OSDFontLarge);
	}
	//Ready to play variable means the end of all menus and beginning of the actual gameplay
	if (ReadyToPlay)
	{


		stringstream outText;
		if (AverageUpdateTime >= 0.0f)
		{
			outText << "Frame Time: " << AverageUpdateTime * 1000.0f << "ms" << endl << "FPS:" << 1.0f / AverageUpdateTime;
			InterfaceManager.RenderText(outText.str(), 2, 2, 0.0f, 0.0f, 0.0f, OSDFont);
			InterfaceManager.RenderText(outText.str(), 0, 0, 1.0f, 1.0f, 0.0f, OSDFont);
			outText.str("");
			outText << "Time From Launch: " << currentTime << "ms";
			InterfaceManager.RenderText(outText.str(), 4, 40, 1.0f, 1.0f, 0.0f, OSDFont);
			outText.str("");
		}

		//No monsters in this version, PvP focused 
		for (int i = 0; i < EntityManager.m_Entities.size(); i++)
		{
			if (EntityManager.m_Entities[i]->isAMonster || i == 0)
			{
				EntityManager.m_Entities[i]->RenderEntityUI(updateTime);
			}
		}
		//The interface is rendered only during combat, and when it stops rendering it means that the battle is over
		//This part is responsible for filling the HP bars of both Player1 and Player2
		//The || characters represent life total, and when grouped together imitate the progress bar
		//Players are initially set not to full life to imitate the battle already going on
		if (!InterfaceManager.isPlayerDead)
		{
			stringstream outTextP;
			stringstream outTextE;
			stringstream outTextB;
			if (currentHpToShowPlayer1 > InterfaceManager.player1Hp)
			{
				currentHpToShowPlayer1 -= updateTime * 100;
			}
			for (int i = 0; i < (int)currentHpToShowPlayer1; i += 4)
			{
				outTextP << "|||||||";
			}
			float r = (float)(InterfaceManager.player1MaxHp - currentHpToShowPlayer1) / InterfaceManager.player1MaxHp;
			float b = 1.0f - r;
			InterfaceManager.RenderText(outTextP.str(), 170, 65, r, 0.1f, b, OSDFontMedium, false);

			for (int i = 0; i < (int)InterfaceManager.ultEnergyDrainMeterPlayer1; i += 5)
			{
				outTextE << "||";
			}
			float rE = ((1000 - InterfaceManager.ultEnergyDrainMeterPlayer1) / 1000);
			float bE = 1 - r;
			InterfaceManager.RenderText(outTextE.str(), 100, 870, rE, 0.1f, bE, OSDFontMedium, false);

			for (int i = 0; i < (int)InterfaceManager.blockTimeRemainsPlayer1; i += 5)
			{
				outTextB << "||";
			}
			InterfaceManager.RenderText(outTextB.str(), 0, 110, 0.75f, 0.75f, 0.2f, OSDFontMedium, false);
		}
		if (isGameMode1VS1)
		{
			if (!InterfaceManager.isPlayerDead)
			{
				stringstream outTextP;
				stringstream outTextE;
				stringstream outTextB;
				if (currentHpToShowPlayer2 > InterfaceManager.player2Hp)
				{
					currentHpToShowPlayer2 -= updateTime * 100;
				}
				for (int i = 0; i < (int)currentHpToShowPlayer2; i += 4)
				{
					outTextP << "|||||||";
				}
				float r = (float)(InterfaceManager.player2MaxHp - currentHpToShowPlayer2) / InterfaceManager.player2MaxHp;
				float b = 1.0f - r;
				InterfaceManager.RenderText(outTextP.str(), ViewportWidth - 510, 65, b, 0.1f, r, OSDFontMedium, false);

				for (int i = 0; i < (int)InterfaceManager.ultEnergyDrainMeterPlayer2; i += 5)
				{
					outTextE << "||";
				}
				float rE = ((1000 - InterfaceManager.ultEnergyDrainMeterPlayer2) / 1000);
				float bE = 1 - r;
				InterfaceManager.RenderText(outTextE.str(), ViewportWidth - 450, 870, bE, rE, bE, OSDFontMedium, false);

				for (int i = 0; i < (int)InterfaceManager.blockTimeRemainsPlayer2; i += 5)
				{
					outTextB << "||";
				}
				InterfaceManager.RenderText(outTextB.str(), 0, ViewportWidth - 200, 0.75f, 0.75f, 0.2f, OSDFontMedium, false);
				InterfaceManager.RenderText(to_string(EntityManager.player1LifeLeft), 530, 30, 1.0f, 0.8f, 0.8f, OSDFontLarge, false);
				InterfaceManager.RenderText(to_string(EntityManager.player2LifeLeft), ViewportWidth - 550, 30, 1.0f, 0.3f, 0.2f, OSDFontLarge, false);
			}

		}
		// The secret interaction helper
		//Secter interaction between opponents is triggered when they both land their Ultimate Abilities on each other
		if (EntityManager.DoubleUltCollisionEvent)
		{
			string P1 = "Press 2  to win";
			string P2 = "Press 3 to win";
			InterfaceManager.RenderText(P1, 100, 300, 0.1f, 0.1f, 1.0f, OSDFontLarge);
			InterfaceManager.RenderText(P2, ViewportWidth - 300, 300, 1.0f, 0.1f, 0.1f, OSDFontLarge);
			InterfaceManager.RenderText(to_string(EntityManager.player1ButtonPressCounter), 100, 400, 1.0f, 0.1f, 0.1f, OSDFontLarge);
			InterfaceManager.RenderText(to_string(EntityManager.player2ButtonPressCounter), ViewportWidth - 100, 400, 0.1f, 0.1f, 1.0f, OSDFontLarge);
		}
	}
}


// Update the scene between rendering
void UpdateScene( float updateTime )
{
	if (!ReadyToPlay)
	{
		ReadyToPlay = MainMenu(updateTime);
		if (ReadyToPlay)
		{
			SceneSetup();
		}
		return;
	}
	
	// Call all entity update functions
	//Time stopper is deprecated, but it allows to delay time and helps with Special Interaction
	if (!TimeStopper(updateTime))
	{
		EntityManager.UpdateAllEntities(updateTime);
		InterfaceManager.UpdateUI(updateTime);
	}
	
		VocalTimer += updateTime;

		if (VocalTimer > 172)
		{
			SoundManager.PlayGlobal(VocalPercussionSound, false);
			VocalTimer = 0;
		}
	
	

	
		
	// Deprecated tester function 
	static bool RotateLight = true;
	static float LightBeta = 0.0f;
	if (RotateLight)
	{
		Lights[1]->SetPosition( LightCentre + LightOrbit * CVector3(cos(LightBeta), 0, sin(LightBeta)) );
		LightBeta -= updateTime * LightOrbitSpeed;
		CEntity* sun = EntityManager.GetEntity("Sun");
		
	}

	
	
	
}
//Main menu is the sequence you will see when you launch the game, including the video, which I will explain further, ready checks and transitioning to combat
bool MainMenu(TFloat32 updateTime)
{
	//Dislaimer is the very first image you see, where I express my thanks in doing the project
	DisclaimerTimer += updateTime;
	if (DisclaimerTimer < 5.0f)
	{
		return false;
	}
	//The sound and images representing the video are not synced code wise, but they go nice along, so if they are asynchronous, it will not affect anything
	if (!MenuIntroPlayed)
	{
		SoundManager.PlayMenuSound(SonochiSound);//Intro music
		MenuIntroPlayed = true;
	}
	return InterfaceManager.RenderMenu(updateTime); // the menu presists until this function returns true
}
bool TimeStopper(TFloat32 updateTime)
{
	if (!isStoppingTime)
	{
		return false;
	}
	else
	{
		dmgTimeFreezeTimer += updateTime * 15;
		if (dmgTimeFreezeTimer >= dmgTimeFreezeMax)
		{
			dmgTimeFreezeTimer = 0.0f;
			isStoppingTime = false;
			return false;
		}
		return true;
	}
		
}


} // namespace gen
