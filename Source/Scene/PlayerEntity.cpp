/*******************************************
PlayerEntity.cpp

Player entity class	definition
The largest class in the program, as it handles the game itself from a perspective of a played
Handles:
         Animation sequence shifting one to another,
		 Player rotations and movement
		 Player UI rendering 
		 Damage and Collision detection
		 Block, Hp and Ult settings
		 Setting up collision frames for each player separately
		 Playback of animation
		 Player State control
********************************************/

#include "PlayerEntity.h"
#include "EntityManager.h"
#include "FMODManager.h"
#include "UIManager.h"
namespace gen
{
	extern ID3D10Device* g_pd3dDevice;
	extern CEntityManager EntityManager;
	extern CAnimationManager AnimationManager;
	extern const string MediaFolder;
	extern CMessenger Messenger;
	extern FMODManager SoundManager;
	extern CCamera* MainCamera;
	extern UIManager InterfaceManager;

	extern bool isGameMode1VS1;
	extern float currentHpToShowPlayer1;
	extern float currentHpToShowPlayer2;
	/*-----------------------------------------------------------------------------------------
	Ship Entity Class
	-----------------------------------------------------------------------------------------*/

	// Player constructor intialises car-specific data and passes parameters to base class constructor
	/*
	///////////////////////////////////////////////
	Player1 is always initialized as Jotaro, and Player2 as Dio, as this is a theme oriented demo of 2 rivals fighting each other with the help of powerful allies

	///////////////////////////////////////////////
	*/
	CPlayerEntity::CPlayerEntity
	(
		CEntityTemplate* PlayerTemplate,
		TEntityUID       UID,
		const string&    name /*= ""*/,
		const CVector3&  position /*= CVector3::kOrigin*/,
		const CVector3&  rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
		const CVector3&  scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
	) : CEntity(PlayerTemplate, UID, name, position, rotation, scale)
	{
		//The first player will always be occupied by Jotaro
		if (!EntityManager.isPlayer1Taken)
		{
			this->player = EntityManager.GetEntity(0);
			player->Matrix().SetPosition(CVector3(-70.0f, 10.0f, 2.0f));
			this->isPlayerJotaro = EntityManager.isPlayer1Jotaro;
			isPlayer1 = true;
			EntityManager.isPlayer1Taken = true;
			SetupPlayer(isPlayer1, isPlayerJotaro);
			SetupControls(isPlayer1);
			f_playAnimSeq(Intro);
		}
		//and the second one by Dio
		else
		{
			this->player = EntityManager.GetEntity(3);
			player->Matrix().SetPosition(CVector3(70.0f, 10.0f, 7.0f));
			faceDirectionRight = false;
			this->isPlayerJotaro = EntityManager.isPlayer2Jotaro;
			isPlayer1 = false;
			SetupPlayer(isPlayer1, isPlayerJotaro);
			if (!isPlayer1)
			{
				f_playAnimSeq(Intro_2);
			}
		}
		
		//Setting up controls in the function on the bottom,
		// Feel free to adjust to your preferences
		SetupControls(isPlayer1);
		//Setting hit frames depending on the player type
		//All the frames are set depending on the visual information provided by the player image :
		//if the player swings a fist or a leg, the hit frame is near it
		setHitFrames(isPlayerJotaro);
		
	}


	// Update the Player
	// Return false if the entity is to be destroyed
	bool CPlayerEntity::Update(TFloat32 updateTime)
	{

		
		if (currentAnimSequence != Ult_Num_1)
		{
			EntityManager.DoubleUltCollisionEvent = false;
		}
		///////////////////////////////////////////////////////
		//This part limits the players within a box in Camera FOV

		if (player->Matrix().GetX() < -90 )
		{
			player->Matrix().SetX(-90);
		}
		if (player->Matrix().GetX() > 90)
		{
			player->Matrix().SetX(90);
		}
		////////////////////////////////////////////////////////


		///////////////////////////////////////////////////////
		//Triggering the special event and following behaviour
		if (EntityManager.DoubleUltCollisionEvent && EntityManager.DoubleUltCollisionTimer > 9 && isPlayerJotaro && !DoubleUltPlayOnce)
		{
			currentStandoAnim = 0;
			currentStandoAnimSequence = Stando_Idle;
				f_playAnimSeq(Heavy_Att);
				DoubleUltPlayOnce = true;
				
		}
		/////////////////////////////////////////////////
		///////////////////////////////////////////////////////
		//Triggering the special event and following behaviour
		if (EntityManager.DoubleUltCollisionEventFinish)
		{
			isAttacking = false;
			f_Unsummon_Stando();
			if (!isPlayerJotaro)
			{
				if(player->Matrix().GetScaleX() > 1.5)
				player->Matrix().SetScale(CVector3(1.5, 1.5, 1.5));
			}
		}
		///////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		//Functions for not updating the player for some time
		if (isPlayer1 && EntityManager.doNotUpdatePlayer1)
		{
			return true;
		}
		if (!isPlayer1 && EntityManager.doNotUpdatePlayer2)
		{
			return true;
		}
		////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////
		//Special interaction for Dio The Warudo ability, which slows other player dramatically
		if (EntityManager.zaWarudoEnabled && !thisUnaffected)
		{
			updateTime /= 8.0f;
		}
		////////////////////////////////////////////////////////
		if(!isVictorious && currentAnimSequence != Is_Killed)
		MessageComponent();
		
		if(!EntityManager.DoubleUltCollisionEvent && EntityManager.CountDownToStart < 0)
		Controls(updateTime);

		RenderEntityUI(updateTime);
		
	      
		//Animation function call
	Animate(updateTime, faceDirectionRight, currentAnimSequence);

	//If Stando is next to the player, we animate it according to the user input
		if (isStandoSummoned && isPlayerJotaro)
		{
			Animate_Stando(updateTime, faceDirectionRight, currentStandoAnimSequence);
		}
		else if (isStandoSummoned && !isPlayerJotaro)
		{
			Animate_StandoDio(updateTime, faceDirectionRight, currentStandoAnimSequence);
		}
	
	    //If the stand was not previously initialized, we do it here
		if (isPlayer1)
		{
			EntityManager.player1Pos = player->Matrix().Position();
			if (stando != NULL)
			{
				EntityManager.standoPlayer1Pos = stando->Matrix().Position();
			}
		}
		else
		{
			EntityManager.player2Pos = player->Matrix().Position();
			if (stando != NULL)
			{
				EntityManager.standoPlayer2Pos = stando->Matrix().Position();
			}
		}
		//This segment controls after which cycle we will see to be continued
		if ((!isPlayerJotaro &&victoryPoseCounter == 12) || (isPlayerJotaro && victoryPoseCounter == 12) )
		{
			SoundManager.PlayGlobal(ToBeContinuedSound, false);
			InterfaceManager.CallForDefeatScreen(2.0F);
	}
	return true;
	}


	bool CPlayerEntity::Animate(TFloat32 updateTime, bool isFacingRight, int AnimationType)
	{
		if (!isPlayerJotaro && isPlayer1&& playerStats.hp >= 200)
		{
			InterfaceManager.ChangeUIAnimFrame("PlayerFaceLeft", AnimationManager.GetPlayerUITexturePath(FaceAnim, 0, isPlayerJotaro, isPlayer1));
		}
		if (!isPlayerJotaro && !isPlayer1&& playerStats.hp >= 200)
		{
			InterfaceManager.ChangeUIAnimFrame("PlayerFaceRight", AnimationManager.GetPlayerUITexturePath(FaceAnim, 0, isPlayerJotaro, isPlayer1));
		}
	
		if (isStandoSummoned )
		{
			DrainEnergy(updateTime);
		}
		
		DamageAccumulator(updateTime, isFacingRight);
		string fullFileName;
		if (isFacingRight)
		{
			fullFileName = MediaFolder + animsRight[AnimationType][currentAnim].second;
		}
		else if (!isFacingRight)
		{
			fullFileName = MediaFolder + animsLeft[AnimationType][currentAnim].second;
		}
		if(isDamaged && isTransparent)
		{
			fullFileName = AnimationManager.GetBlankTexturePath();
		}
		if (this->isPlayerJotaro == true)
		{
			SwitchAnimStateJotaro(updateTime, isFacingRight, AnimationType, fullFileName);
		}
		else  if(this->isPlayerJotaro == false)
		{
			SwitchAnimStateDio(updateTime, isFacingRight, AnimationType, fullFileName);
		}
		
		if (isAttacking)
		{
			for (int i = 0; i <= (playerAttackAnimsMax - AttackAnimsPlayerStart); i++)
			{
				if (i == currentAnimSequence - AttackAnimsPlayerStart)
				{
					for (int j = 0; j < hitFrames[i].second.size(); j++)
					{
						if (hitFrames[i].second[j] == currentAnim)
						{
							HitMessageComposer();
						}
					}
				}
			}
		}
		return true;
	}
	//Stando Animator for Jotaro
	bool CPlayerEntity::Animate_Stando(TFloat32 updateTime, bool isFacingRight, int AnimationType)
	{
		f_Create_Stando();
		string fullFileName;
		if (isFacingRight)
		{
			fullFileName = MediaFolder + animsRight[AnimationType][currentStandoAnim].second;
		}
		else if (!isFacingRight)
		{
			fullFileName = MediaFolder + animsLeft[AnimationType][currentStandoAnim].second;
		}
		//////////////////////////////////////////////
		//This function is used everywhere as it is because I was familiarising myself with rendering texture to file with no memory leaks 
		//////////////////////////////////////////////
		stando->Mesh->m_Materials->textures[0]->Release();
		if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &stando->Mesh->m_Materials->textures[0], NULL)))
		{
			string errorMsg = "Error loading texture " + fullFileName;
			SystemMessageBox(errorMsg.c_str(), "Mesh Error");
			return false;
		}
		if (AnimationType == Stando_Idle)
		{
			isStandoIdle = true;
			stando->Matrix().MoveY(sin(m_StandoFloating));
			m_StandoFloating += 0.01;
			return true;
		}
		//animCycle Delay is what time needs to pass for the animation frame to change
		if (standoAnimChangeTimer >= animCycleDelay)
		{
			isAttacking = true;
			switch (AnimationType)
			{
			
			case Stando_OraOraOra:

				if (currentStandoAnim == 2)
				{
					f_Rescale(true, stando);
				}
				else if (currentStandoAnim == 19)
				{
					f_Rescale(false, stando);
				}
				break;
			case Stando_Sp_2:
				if (currentStandoAnim == 7)
				{
					f_Rescalex2(true, stando);
				}
				else if (currentStandoAnim == 10)
				{
					f_Rescalex2(false, stando);
					Enlarged = false;
				}
				break;
			case Stando_Sp_3:
				if (currentStandoAnim == 3 || currentStandoAnim == 6)
				{
					m_Stando_Ult_Displacement += 10;
					f_Rescale(true, stando);
					SoundManager.PlayStandoSound(StandoBasicAttackSound, false);
				}
				else if (currentStandoAnim == 5 || currentStandoAnim == 7)
				{
					f_Rescale(false, stando);
					Enlarged = false;
				}
				break;
			case Stando_Sp_4:
				if (currentStandoAnim == 11 )
				{
					m_Stando_Ult_Displacement += 20;
					f_Rescalex2(true, stando);
				}
				else if (currentStandoAnim == 14)
				{
					f_Rescalex2(false, stando);
					Enlarged = false;
				}
				break;
			case Stando_Ult_1:
				if (currentAnim < 3)
				{
					currentStandoAnim = 0;
				}
				if (currentStandoAnim == 3 || currentStandoAnim == 18 || currentStandoAnim == 54 || currentStandoAnim == 89)
				{
					f_Rescale(true, stando);
				}
				else if (currentStandoAnim == 16 || currentStandoAnim == 51 || currentStandoAnim == 87 || currentStandoAnim == 104)
				{
					f_Rescale(false, stando);
				}
				if (EntityManager.DoubleUltCollisionEvent && currentStandoAnim > 100)
				{
					currentStandoAnim = 10;
				}
				break;
			default:
				break;
			}
			
				currentStandoAnim++;
			
			if (currentStandoAnim >= animsRight[AnimationType].size())
			{
				isUlting = false;
				if (isPlayer1)
				{
					EntityManager.isPlayer1Ulting = isUlting;
				}
				else
					EntityManager.isPlayer2Ulting = isUlting;
				currentStandoAnim = 0;
				thisDamageFrameStando = 9999;
				currentStandoAnimSequence = Stando_Idle;
				m_Stando_Ult_Displacement = 0;
				if (ultPointsAvailable == 0)
				{
					f_Unsummon_Stando();
				}
			}
			//resetting the anim timer
			standoAnimChangeTimer = 0.0f;
		}
		switch (currentStandoAnimSequence)
		{
		
			
		case Stando_OraOraOra: case Stando_Sp_2: case Stando_Sp_3: case Stando_Sp_4: 
			standoAnimChangeTimer += updateTime * AnimMultFast;
			break;
		case Stando_Ult_1:
		standoAnimChangeTimer += updateTime * AnimMultSplitSec;
		break;
		default:
			standoAnimChangeTimer += updateTime * AnimMultNormal;
			break;
		}
		//Here we check for the hit frames according to previously set, and after that we create a hit message to be sent to opposing player
		for (int i = 0; i <= (AttackAnimsPlayerFinish  - AttackAnimsPlayerStart); i++)
		{
			if (i == currentStandoAnimSequence - AttackAnimsPlayerStart)
			{
				for (int j = 0; j < hitFrames[i].second.size(); j++)
				{
					if (hitFrames[i].second[j] == currentStandoAnim)
					{
						HitMessageComposer();
						if (currentStandoAnimSequence == Stando_OraOraOra)
						{
							SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false,isPlayer1);
						}
					}
				}
			}
		}

		return true;
	}
	//Here lies the battle system, which controls the movements and invokes actions for the players according to previously chosen movement scheme
	//The controls are tuned in not to overlap with each other, not to break important sequences and do not transition until over
	void CPlayerEntity::Controls(TFloat32 updateTime) {
		
		if (isKnockedUp)
		{
			f_physGravity(updateTime / 2 );
			player->Matrix().MoveY(m_UpdwardVel * 0.33);
			
		}
		if (!animationLock && !this->buttonPressed) {

			if (KeyHeld(GoLeftKey) || KeyHeld(GoRightKey))
			{
				if (KeyHeld(GoRightKey))
				{
					if (!faceDirectionRight)
					{
						f_Turn(true);
					}
					else

						if (currentAnimSequence != Walking && currentAnimSequence != Turning)
						{
							currentAnimSequence = Walking;
							currentAnim = 0;
						}
					if(EntityManager.zaWarudoEnabled)
					player->Matrix().MoveX(m_HorizontalMoveSpeed / 3);
					else
						player->Matrix().MoveX(m_HorizontalMoveSpeed);
				
				}
				else if (KeyHeld(GoLeftKey))
				{
					if (faceDirectionRight)
					{
						f_Turn(false);
					}
					if (currentAnimSequence != Walking && currentAnimSequence != Turning)
					{
						currentAnimSequence = Walking;
						currentAnim = 0;
					}
					if(EntityManager.zaWarudoEnabled)
						player->Matrix().MoveX(-m_HorizontalMoveSpeed / 3);
					else
						player->Matrix().MoveX(-m_HorizontalMoveSpeed);
				}		
				if (KeyHit(Dash_FrontKey))
				{
					f_Dash(faceDirectionRight);
				}

				if ( KeyHit(Dash_BackKey))
				{
					f_Dash_BW(faceDirectionRight);
				}
				if (KeyHit(CrouchKey))
				{
					f_Crouch();
				}
				if (KeyHit(BlockKey) && !animationLock)
				{
					f_Block();
				}
				if (KeyHit(JumpKey))
				{
					f_playAnimSeq(Jump);
				}
				if (KeyHit(SummonKey) && !isStandoSummoned && ultPointsAvailable > 0)
				{
					f_playAnimSeq(Summon);
				}
				if (KeyHit(LightAttackKey))
				{
					 f_playAnimSeq(Light_Leg_Att);
				}
				if (KeyHit(MediumAttackKey))
				{
					 f_playAnimSeq(Medium_Walk_Att);
				}
				if (KeyHit(HeavyAttackKey) && (isStandoSummoned || !isPlayerJotaro))
				{
					f_playAnimSeq(Heavy_Walk_Att);
				}

			}
		
			else if (KeyHit(CrouchKey))
			{
				f_Crouch();
			}
			else if (KeyHit(BlockKey) && !animationLock)
			{
				f_Block();
			}
			else if (KeyHit(JumpKey))
			{
				f_playAnimSeq(Jump);
			}
			else if  (KeyHit(Dash_FrontKey))
			{
				f_Dash(faceDirectionRight);
			}
			else if (KeyHit(SummonKey) && !isStandoSummoned)
			{
				f_playAnimSeq(Summon);
			}
			else if (KeyHeld(Special3Key) && KeyHit(LightAttackKey) && isStandoSummoned )
			{
				f_playAnimSeq(Special_OraOraOra);
				
			}
			else if (KeyHeld(Special3Key) && KeyHit(HeavyAttackKey) && (isStandoSummoned || !isPlayerJotaro))
			{
				f_playAnimSeq(Special_Num_2);
				
			}
			else if (KeyHeld(Special4Key) && KeyHit(LightAttackKey) &&  isStandoSummoned)
			{
				f_playAnimSeq(Special_Num_3);
				
			}
			else if (KeyHeld(Special4Key) && KeyHit(HeavyAttackKey) && isStandoSummoned )
			{
				f_playAnimSeq(Special_Num_4);
				
			}
			else if ((KeyHeld(Special3Key) && (KeyHeld(Special4Key)) && (isStandoSummoned || !isPlayerJotaro)))
			{
				f_playAnimSeq(Ult_Num_1);
			}
			else if (KeyHeld(SummonKey) && isStandoSummoned)
			{
				f_Unsummon_Stando();
	
			}
			else if  (KeyHit(Dash_BackKey))
			{
				f_Dash_BW(faceDirectionRight);
			}
			else if (KeyHit(LightAttackKey))
			{
				 f_playAnimSeq(Light_Leg_Att);
			}
			else if (KeyHit(MediumAttackKey))
			{
				 f_playAnimSeq(Medium_Att);
			}
			else if (KeyHit(HeavyAttackKey) && isStandoSummoned )
			{
				f_playAnimSeq(Heavy_Att);
			}
			else
			{
				f_Idle();

			}
			
			if (KeyHeld(CrouchKey))
			{
				f_Crouch();
			}
			if (KeyHeld(BlockKey) && !animationLock)
			{
				f_Block();
			}
			if (KeyHeld(LightAttackKey) && KeyHeld(HeavyAttackKey) && isStandoSummoned)
			{
				f_playAnimSeq(Throw);
			}
			
		}
		if (buttonPressed && currentAnimSequence == Crouch)
		{
			if ((KeyHit(GoLeftKey) && faceDirectionRight) || ((KeyHit(GoRightKey) && !faceDirectionRight)))
			{
				f_Crouch_Turn();
			}
			if ((KeyHeld(GoLeftKey) && !faceDirectionRight) && KeyHit(HeavyAttackKey) || ((KeyHeld(GoRightKey) && faceDirectionRight)) && KeyHit(HeavyAttackKey))
			{
				if (isStandoSummoned && isPlayerJotaro)
				{
					f_playAnimSeq(Heavy_Crouch_Fr_Att);
				}
				
			}
			if (KeyHit(LightAttackKey))
			{
				 f_playAnimSeq(Light_Crouch_Att);
			}
			if (KeyHeld(BlockKey) && !isBlocking)
			{
				f_playAnimSeq(Crouch_Block);
			}
			if (KeyHit(MediumAttackKey) && (isStandoSummoned || !isPlayerJotaro))
			{
				 f_playAnimSeq(Medium_Crouch_Att);
			}
			if (KeyHit(HeavyAttackKey) && (isStandoSummoned || !isPlayerJotaro))
			{
				f_playAnimSeq(Heavy_Crouch_Att);
			}
		}
		if (animationLock && currentAnimSequence == Dash)
		{
			if (KeyHit(MediumAttackKey))
			{
				 f_playAnimSeq(Medium_Att);
			}
			if (KeyHit(HeavyAttackKey) && (isStandoSummoned || !isPlayerJotaro))
			{
				f_playAnimSeq(Heavy_Att);
			}
		}
		if (!KeyHeld(CrouchKey) && !KeyHeld(BlockKey) && buttonPressed)
		{
			if(currentAnimSequence == Crouch)
			f_Stand_Up();

			buttonPressed = false;
		}
		if (isInAir  )
		{

			if (KeyHeld(GoRightKey))
			{
				player->Matrix().MoveX(m_HorizontalMoveSpeed);
			}
			if (KeyHeld(GoLeftKey))
			{
				player->Matrix().MoveX(-m_HorizontalMoveSpeed);
			}
			if(currentAnimSequence != Is_Hit_Air)
				{
				if (KeyHit(SummonKey) && !isStandoSummoned && airAttackCount < airAttackLimit)
				{
					f_playAnimSeq(Summon_Air);
					airAttackCount++;
				}
				else if (KeyHeld(SummonKey) && isStandoSummoned)
				{
					f_Unsummon_Stando();

				}

				if (airAttackCount < airAttackLimit)
				{
					if (KeyHit(MediumAttackKey))
					{
						f_playAnimSeq(Medium_Air_Att);
					}
					if (KeyHit(HeavyAttackKey))
					{
						f_playAnimSeq(Heavy_Air_Att);
					}
				}
				if (KeyHeld(BlockKey) && !isBlocking)
				{
					f_playAnimSeq(Block_Air);
				}

			}
			
		}
			
		if (!KeyHeld(BlockKey))
		{
			isBlocking = false;
		}
		
		
	}

	//These functions are left stand-alone due to their unnatural behaviour when coupled in the f_playAnimSeq function below

	void CPlayerEntity::f_Idle()
	{
		isAttacking = false;
		if (currentAnimSequence != Idle)
		{
			currentAnimSequence = Idle;
			currentAnim = 0;
			buttonPressed = false;
		}
	}
	void CPlayerEntity::f_Turn(bool faceDirRight)
	{
		currentAnim = 0;
		currentAnimSequence = Turning;
		animationLock = true;
		faceDirectionRight = faceDirRight;

	}
	void CPlayerEntity::f_Crouch()
	{
		isAttacking = false;
		if (currentAnimSequence != Crouch)
		{
			currentAnim = 0;
			animationLock = true;
			currentAnimSequence = Crouch;
		}
		buttonPressed = true;
	}
	void CPlayerEntity::f_Block()
	{
		isAttacking = false;
		if (currentAnimSequence != Block)
		{
			currentAnim = 0;
			animationLock = true;
			currentAnimSequence = Block;
		}
		buttonPressed = true;
		isBlocking = false;
	}
	void CPlayerEntity::f_Stand_Up()
	{
		currentAnimSequence = Stand_Up;
		currentAnim = 0;
		buttonPressed = false;
		animationLock = true;
	}
	void CPlayerEntity::f_Crouch_Turn()
	{
		currentAnimSequence = Crouch_Turn;
		currentAnim = 0;
		animationLock = true;
	}
	void CPlayerEntity::f_Dash(bool faceDirRight)
	{
		currentAnim = 0;
		animationLock = true;
		if (faceDirRight)
		{
			currentAnimSequence = Dash;
		}
		else
			currentAnimSequence = Dash_Back;
	}
	void CPlayerEntity::f_Dash_BW(bool faceDirRight)
	{
		currentAnim = 0;
		animationLock = true;
		if (!faceDirRight)
		{
			currentAnimSequence = Dash;
		}
		else
			currentAnimSequence = Dash_Back;
	}
	
	//A general function for controlling BOTH 
	//                                        Ultimate points interface animations
	//                                         Player animations
	//                                          The settings are very sensitive in general, that is why each sequence is detailed
	// Also responsible for playback of necessary sounds when needed
	void CPlayerEntity:: f_playAnimSeq(int animType)
	{
		if (isPlayerJotaro)
		{
			if (animType >= Special_OraOraOra && animType <= Ult_Num_1)
			{
				switch (animType)
				{
				case Special_OraOraOra: case Special_Num_3:

					if (ultPointsAvailable < 1)
						return;

					ultPointsAvailable--;
					if(isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					else
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					break;
				case Special_Num_2: case Special_Num_4:
					if (ultPointsAvailable < 2)
						return;

					ultPointsAvailable -= 2;
					if (isPlayer1)
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					else
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					break;
				case Ult_Num_1:
					if (ultPointsAvailable != ultPointsMax)
						return;

					ultPointsAvailable = 0;
					if (isPlayer1)
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					else
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					break;
				}
			}
			currentAnim = 0;
			currentAnimSequence = animType;
			animationLock = true;

			
			switch (animType)
			{
		
			case Crouch_Block: case Block_Air:
				isBlocking = true;
				
				break;
			case Medium_Air_Att: case Heavy_Air_Att:
				airAttackCount++;
				isAttacking = true;
				SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false,isPlayer1);
				break;
			case Summon: case Summon_Air:
				if (!Enlarged && isPlayerJotaro)
					f_Rescale(true, player);
				SoundManager.PlayPlayerSound(Special_SummonSound, false, isPlayer1);
				break;
			case Jump:
				isInAir = true;
				m_UpwardVelocity = 2.0f;
				SoundManager.PlayPlayerSound(PlayerJumpSound, false, isPlayer1);
				break;
			case Light_Crouch_Att: case Light_Leg_Att: case Medium_Att:  case Medium_Walk_Att:
				isAttacking = true;
				SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
				break;
			case Heavy_Att: case Heavy_Walk_Att: case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att: case Medium_Crouch_Att:
				isAttacking = true;
				f_Unsummon_Stando();
				if(!EntityManager.DoubleUltCollisionEvent)
				SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
				break;
			case Throw:
				f_Unsummon_Stando();
				break;
			case Special_OraOraOra:
				m_Stando_Ult_Displacement = 20.0f;
				currentStandoAnimSequence = Stando_OraOraOra;
				currentStandoAnim = 0;
				isStandoIdle = false;
				break;
			case Special_Num_2:
				m_Stando_Ult_Displacement = 30.0f;
				currentStandoAnimSequence = Stando_Sp_2;
				currentStandoAnim = 0;
				isStandoIdle = false;
				SoundManager.PlayStandoSound(Stando_StarFingerSound, false);
				break;
			case Special_Num_3:
				m_Stando_Ult_Displacement = 10.0f;
				currentStandoAnimSequence = Stando_Sp_3;
				currentStandoAnim = 0;
				isStandoIdle = false;
				break;
			case Special_Num_4:
				m_Stando_Ult_Displacement = 5.0f;
				currentStandoAnimSequence = Stando_Sp_4;
				currentStandoAnim = 0;
				isStandoIdle = false;
				SoundManager.PlayStandoSound(Stando_StarBreakerSound, false);
				break;
			case Ult_Num_1:
				m_Stando_Ult_Displacement = 15.0f;
				currentStandoAnimSequence = Stando_Ult_1;
				currentStandoAnim = 0;
				isStandoIdle = false;
				isUlting = true;
				if (isPlayer1)
				{
					EntityManager.isPlayer1Ulting = isUlting;
				}
				else
					EntityManager.isPlayer2Ulting = isUlting;

				animCycleDelay = 50.0;
				if (isPlayer1)
				{
					EntityManager.doNotUpdatePlayer2 = true;
				}
				else
					EntityManager.doNotUpdatePlayer1 = true;

				SoundManager.PlayPlayerSound(YareYareTauntSound, true, isPlayer1);
				break;
			case Is_Hit_Light: case Is_Hit_Light_2: case Is_Hit_Crouch: case Is_Hit_Crouch_2:
				break;
			case Is_Hit_Medium: case Is_Hit_Medium_2:
				f_attackMoveDisplacer(-2.5);
				break;
			case Is_Hit_Hard: case Is_Hit_Hard_2:
				f_attackMoveDisplacer(-4.0f);
				break;
			case Is_Killed:
				/*SoundManager.PlayGlobal(ToBeContinuedSound, false);
				InterfaceManager.CallForDefeatScreen(2);*/
				if ((isPlayer1 && EntityManager.player1LifeLeft < 0) || (!isPlayer1 && EntityManager.player2LifeLeft < 0))
				{
					isDeadPlayer1 = true;
					
				}
				
				break;
			case Victory: case Victory_2:
				SoundManager.PlayPlayerSound(YareYareTauntSound, false, isPlayer1);
				isVictorious = true;
				EntityManager.player2LifeLeft = -1;
				break;
			default:
				break;
			}

			if (currentAnimSequence >= AttackAnimsPlayerStart && currentAnimSequence <= AttackAnimsPlayerFinish)
			{
				isAttacking = true;
			}
		}
		else
			f_playAnimSeqDio(animType);
			
		
	}
	//Same function as above, but tailored for Dio and his animation structure
	void CPlayerEntity::f_playAnimSeqDio(int animType)
	{
		if (animType >= Special_OraOraOra && animType <= Ult_Num_1)
		{
			switch (animType)
			{
			
			 case Special_Num_4: case Special_OraOraOra:
				if (ultPointsAvailable < 2)
					return;

				ultPointsAvailable -= 2;
				if (isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				else
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				break;
			case Special_Num_2:
				if (ultPointsAvailable < 1)
					return;

				ultPointsAvailable--;
				if (isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				else
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				break;
			case Special_Num_3:
				if (ultPointsAvailable < 3)
					return;

				ultPointsAvailable-=3;
				if (isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				else
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				break;
			case Ult_Num_1:
				if (ultPointsAvailable != ultPointsMax)
					return;

				ultPointsAvailable = 0;
				
				if (isPlayer1)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					InterfaceManager.ultEnergyDrainMeterPlayer1 = 0;
				}	
				else
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
					InterfaceManager.ultEnergyDrainMeterPlayer2 = 0;
				}
					
				break;
			}
		}
		currentAnim = 0;
		currentAnimSequence = animType;
		animationLock = true;


		switch (animType)
		{
		case Crouch_Block: case Block_Air:
			isBlocking = true;
			break;
		case Medium_Air_Att: case Heavy_Air_Att:
			airAttackCount++;
			isAttacking = true;
			SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
			break;
		case Summon: case Summon_Air:
			if (!Enlarged && isPlayerJotaro)
				f_Rescale(true, player);
			SoundManager.PlayPlayerSound(Special_SummonSound, false, isPlayer1);
			break;
		case Jump:
			isInAir = true;
			if (EntityManager.zaWarudoEnabled && !thisUnaffected)
				m_UpwardVelocity = 0.4;
			else
			m_UpwardVelocity = 2.0f;
	
			SoundManager.PlayPlayerSound(PlayerJumpSound, false, isPlayer1);
			break;
		case Light_Crouch_Att: case Light_Leg_Att: case Medium_Att:  case Medium_Walk_Att:
			isAttacking = true;
			SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
			break;
		case Heavy_Att:  case Heavy_Crouch_Fr_Att:
			isAttacking = true;
			f_Unsummon_Stando();
			SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
			break;
		case Heavy_Walk_Att: case Heavy_Crouch_Att:  case Medium_Crouch_Att:
			isAttacking = true;
			SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
			break;
		case Throw:
			f_Unsummon_Stando();
			break;
		case Special_OraOraOra:
			m_Stando_Ult_Displacement = 20.0f;
			currentStandoAnimSequence = Stando_OraOraOra;
			currentStandoAnim = 0;
			isStandoIdle = false;
			SoundManager.PlayPlayerSound(MudaMudaMudaSound, false, isPlayer1);
			break;
		case Special_Num_2:
			m_Stando_Ult_Displacement = 30.0f;
			isStandoIdle = false;
			isAttacking = true;
			SoundManager.PlayPlayerSound(WryyySound, false, isPlayer1);
			break;
		case Special_Num_3:
			thisUnaffected = true;
			isAttacking = true;
			f_Unsummon_Stando();
			SoundManager.PlayPlayerSound(ZaWarudoSound, false, isPlayer1);
			break;
		case Special_Num_4:
			isAttacking = true;
			f_Unsummon_Stando();

			break;
		case Ult_Num_1:
			isAttacking = true;
			isUlting = true;
			EntityManager.RoddaRolla = true;
			player->Matrix().MoveZ(10);
			player->Matrix().SetY(2300);
			if (isPlayer1)
			{
				EntityManager.isPlayer1Ulting = isUlting;
			}
			else
				EntityManager.isPlayer2Ulting = isUlting;
			player->Matrix().SetScale(CVector3(3.5, 2.5, 2.5));
			SoundManager.PlayPlayerSound(RodaRollaSound, true, isPlayer1);
			break;
		case Is_Hit_Light: case Is_Hit_Light_2: case Is_Hit_Crouch: case Is_Hit_Crouch_2:
			break;
		case Is_Hit_Medium: case Is_Hit_Medium_2:
			f_attackMoveDisplacer(-2.5);
			break;
		case Is_Hit_Hard: case Is_Hit_Hard_2:
			f_attackMoveDisplacer(-4.0f);
			break;
		case Is_Killed:
			/*SoundManager.PlayGlobal(ToBeContinuedSound, false);
			InterfaceManager.CallForDefeatScreen(2);*/
			if ((isPlayer1 && EntityManager.player1LifeLeft < 0) || (!isPlayer1 && EntityManager.player2LifeLeft < 0))
			{
				if (isPlayer1)
				{
					isDeadPlayer1 = true;
				}
				else
					isDeadPlayer2 = true;
			}
			break;
		case Victory: case Victory_2:
			isVictorious = true;
			break;
		default:
			break;
		}

		if (currentAnimSequence >= AttackAnimsPlayerStart && currentAnimSequence <= AttackAnimsPlayerFinish)
		{
			isAttacking = true;
		}
	}
	void CPlayerEntity::f_Create_Stando()
	{
		if (this->stando == NULL)
		{
			if(isPlayer1)
			this->stando = EntityManager.GetEntity("Stando");

			if (!isPlayer1)
				this->stando = EntityManager.GetEntity("Stando2");
			if(!isPlayerJotaro)
				this->stando->Matrix().Scale(CVector3(1.5, 1.5, 1.5));
				
		}

		f_Stando_Displacement();
		
		isStandoSummoned = true;
	}
	void CPlayerEntity::f_Unsummon_Stando()
	{
		stando->Matrix().SetPosition(CVector3(-5000, -5000, -5000));
		isStandoSummoned = false;
	}
	void CPlayerEntity::f_Rescale(bool Enlarge, CEntity* entity)
	{
		if (Enlarge && entity->Matrix().GetScaleX() < 2.5f)
		{
			entity->Matrix().SetScaleX(2.5f);
			
			Enlarged = true;
		}
		else if(!Enlarge && entity->Matrix().GetScaleX() > 2.0f)
		{ 
			entity->Matrix().SetScaleX(1.0f);
			

			Enlarged = false;
		}

	}
	void CPlayerEntity::f_Rescalex2(bool Enlarge, CEntity* entity)
	{
		if (Enlarge && entity->Matrix().GetScaleX() < 2.5f)
		{
			entity->Matrix().SetScaleX(2.5f);
			if (faceDirectionRight)
			{
				entity->Matrix().MoveX(5);
			}
			else
				entity->Matrix().MoveX(-5);
			Enlarged = true;
		}
		else if (!Enlarge && entity->Matrix().GetScaleX() > 1.0f)
		{
			entity->Matrix().SetScaleX(1.0f);
			if (faceDirectionRight)
			{
				entity->Matrix().MoveX(-5);
			}
			else
				entity->Matrix().MoveX(5);

			Enlarged = false;
		}
	}
	void CPlayerEntity::f_RescaleY(bool Enlarge, CEntity* entity, TFloat32 amount)
	{
		if (Enlarge && entity->Matrix().GetScaleY() < amount)
		{
			entity->Matrix().SetScaleX(amount);
			
				entity->Matrix().MoveY(5);
			
		}
		else if (!Enlarge && entity->Matrix().GetScaleY() > 1.0f)
		{
			entity->Matrix().SetScaleY(1.0f);
			
				entity->Matrix().MoveX(-5);
	
		}
	}
	void CPlayerEntity::f_attackMoveDisplacer(float displacementX)
	{
		if (EntityManager.zaWarudoEnabled && !thisUnaffected)
		{
			displacementX /= 15;
		}
		if (faceDirectionRight)
		{
			player->Matrix().MoveX(displacementX/3);
		}
		else if (!faceDirectionRight)
		{
			player->Matrix().MoveX(-displacementX/3);
		}
	}
	void CPlayerEntity::f_Stando_Displacement()
	{

		if (m_Stando_Ult_Displacement != 0 && currentStandoAnimSequence == Stando_Idle)
		{
			m_Stando_Ult_Displacement = 0;
		}

	
		if (faceDirectionRight)
		{
			if(isPlayer1)
			stando->Matrix().SetPosition(CVector3(player->Matrix().GetX() + (7 + m_Stando_Ult_Displacement), player->Matrix().GetY() + 3, player->Matrix().GetZ() + 10));
			else
				stando->Matrix().SetPosition(CVector3(player->Matrix().GetX() + (7 + m_Stando_Ult_Displacement), player->Matrix().GetY() + 3, player->Matrix().GetZ() + 15));
		}
		else if (!faceDirectionRight)
		{
			if (isPlayer1)
				stando->Matrix().SetPosition(CVector3(player->Matrix().GetX() - (7 + m_Stando_Ult_Displacement), player->Matrix().GetY() + 3, player->Matrix().GetZ() + 10));
			else
				stando->Matrix().SetPosition(CVector3(player->Matrix().GetX() - (7 + m_Stando_Ult_Displacement), player->Matrix().GetY() + 3, player->Matrix().GetZ() + 15));
		}
	}
	void CPlayerEntity::f_physGravity(TFloat32 updateTime)
	{
		int scaling = 2;
		if (EntityManager.zaWarudoEnabled && !thisUnaffected)
		{
			scaling = 15;
		}
		player->Matrix().MoveY(m_UpwardVelocity);
		m_UpwardVelocity -= m_Gravity * updateTime * scaling;
		if (player->Matrix().GetY() < m_GroundLevel)
		{
			m_UpwardVelocity = 0.0f;
			player->Matrix().SetY(m_GroundLevel);
			isInAir = false;
			isKnockedUp = false;
		}

	}
	//Message component for recieving messages, only 2 types for now: Damage and Victory
	void CPlayerEntity::MessageComponent()
	{
		SMessage msg;
		if (player != NULL)
		{
			TEntityUID thisUID;
			if (isPlayer1)
				thisUID = PlayerUID;
			else
				thisUID = Player2UID;
			if (Messenger.FetchMessage(thisUID, &msg))
			{
				bool rand = Random(0, 1);
				switch (msg.type)
				{
					//When a victory message is recieved, we cancel all else and start celebrating
				case Msg_Victory:

					SoundManager.PlayGlobal(VictorySound, false);
					if (playerStats.hp < 100)
						f_playAnimSeq(Victory);
					else
						f_playAnimSeq(Victory_2);
					break;
					//For the damage, a function below manages damage behaviour, including: Knockback, Knockup, Energy fill from damage  
				case Msg_Dmg:
					
					SwitchPlayerHpState(msg);
					break;
				
				default:
					break;
				}
			}
		}
	}
	//Setting hit frames for each sequence, is a vector of vectors of pairs, but I felt like it was easier to manage, and it proved to be worth the effort
	void CPlayerEntity::setHitFrames(bool isPlayerJotaro)
	{
		if (isPlayerJotaro)
		{
			playerAttackAnimsMax = AttackAnimsPlayerFinish;
		}
		else if (!isPlayerJotaro)
		{
			playerAttackAnimsMax = Ult_Num_1;
		}
		HitFrames hitframes;
		int countFromStart = 0;
		if (isPlayerJotaro)
		{
			for (int i = AttackAnimsPlayerStart; i <= playerAttackAnimsMax; i++)
			{
				hitframes.first = i;
				hitFrames.push_back(hitframes);
				switch (i)
				{
				case Light_Leg_Att:
					hitFrames[countFromStart].second.push_back(2);
					break;
				case Light_Crouch_Att:
					hitFrames[countFromStart].second.push_back(6);
					break;
				case Medium_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Medium_Walk_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Medium_Crouch_Att:
					hitFrames[countFromStart].second.push_back(3);
					break;
				case Medium_Air_Att:
					hitFrames[countFromStart].second.push_back(2);
					break;
				case Heavy_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Heavy_Walk_Att:
					hitFrames[countFromStart].second.push_back(6);
					break;
				case Heavy_Crouch_Att:
					hitFrames[countFromStart].second.push_back(5);
					break;
				case Heavy_Crouch_Fr_Att:
					hitFrames[countFromStart].second.push_back(6);
					break;
				case Heavy_Air_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Throw:
					hitFrames[countFromStart].second.push_back(6);
					hitFrames[countFromStart].second.push_back(16);
					break;
				case Stando_OraOraOra:
					hitFrames[countFromStart].second.push_back(3);
					hitFrames[countFromStart].second.push_back(6);
					hitFrames[countFromStart].second.push_back(9);
					hitFrames[countFromStart].second.push_back(12);
					hitFrames[countFromStart].second.push_back(15);
					break;
				case Stando_Sp_2:
					hitFrames[countFromStart].second.push_back(8);
					break;
				case Stando_Sp_3:
					hitFrames[countFromStart].second.push_back(4);
					hitFrames[countFromStart].second.push_back(8);
					break;
				case Stando_Sp_4:
					hitFrames[countFromStart].second.push_back(12);
					break;
				case Stando_Ult_1:
					for (int ult = 5; ult < 105; ult +=2)
					{
						if(ult < 105)
						hitFrames[countFromStart].second.push_back(ult);
					}
				default:
					break;
				}
				countFromStart++;
			}
		}
		else if (!isPlayerJotaro)
		{
			for (int i = AttackAnimsPlayerStart; i <= playerAttackAnimsMax; i++)
			{
				hitframes.first = i;
				hitFrames.push_back(hitframes);
				switch (i)
				{
				case Light_Leg_Att:
					hitFrames[countFromStart].second.push_back(1);
					break;
				case Light_Crouch_Att:
					hitFrames[countFromStart].second.push_back(1);
					break;
				case Medium_Att:
					hitFrames[countFromStart].second.push_back(3);
					hitFrames[countFromStart].second.push_back(6);
					break;
				case Medium_Walk_Att:
					hitFrames[countFromStart].second.push_back(3);
					hitFrames[countFromStart].second.push_back(6);
					break;
				case Medium_Crouch_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Medium_Air_Att:
					hitFrames[countFromStart].second.push_back(2);
					break;
				case Heavy_Att:
					hitFrames[countFromStart].second.push_back(2);
					break;
				case Heavy_Walk_Att:
					hitFrames[countFromStart].second.push_back(3);
					break;
				case Heavy_Crouch_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Heavy_Air_Att:
					hitFrames[countFromStart].second.push_back(4);
					break;
				case Throw:
					hitFrames[countFromStart].second.push_back(6);
					hitFrames[countFromStart].second.push_back(14);
					break;
				case Stando_OraOraOra:
					for (int i = 1; i < animsRight[Stando_OraOraOra].size(); i += 3)
					{
						hitFrames[countFromStart].second.push_back(i);
					}
					break;
				case Special_Num_4:
					hitFrames[countFromStart].second.push_back(14);
					hitFrames[countFromStart].second.push_back(16);
					hitFrames[countFromStart].second.push_back(20);
					hitFrames[countFromStart].second.push_back(30);
					break;
				case Stando_Sp_3:
					hitFrames[countFromStart].second.push_back(4);
					hitFrames[countFromStart].second.push_back(8);
					break;
				case Stando_Sp_4:
					hitFrames[countFromStart].second.push_back(12);
					break;
				case Ult_Num_1:
					hitFrames[countFromStart].second.push_back(1);
					hitFrames[countFromStart].second.push_back(2);
					for (int ult = 12; ult < 130; ult += 5)
					{
						hitFrames[countFromStart].second.push_back(ult);
					}
				default:
					break;
				}
				countFromStart++;
			}
		}
	}
	//Here we assemble the hit message depending on the attack type, and if we hit the enemy, we send it to him
	void CPlayerEntity::HitMessageComposer()
	{
		bool noSoundAttack = false;
		SMessage msg;
		msg.type = Msg_Dmg;
		if (isPlayer1)
			msg.from = PlayerUID;
		else
			msg.from = Player2UID;
		msg.knockbackVel = 1;
		msg.knockUpVel = 0.1;
		msg.isStoppingTime = true;

		if (isPlayerJotaro)
		{
			switch (currentAnimSequence)
			{
			case Light_Leg_Att: case Light_Crouch_Att:
				msg.dmg = 10;
				msg.knockbackVel = 1.0f;
				break;
			case Medium_Air_Att:
				msg.knockbackVel = 2.5f;
				msg.dmg = 10;
				break;
			case Medium_Att: case Medium_Walk_Att: case Medium_Crouch_Att:  case Special_Num_3:
				msg.dmg = 20;
				msg.knockbackVel = 2.5f;
				break;
			case Heavy_Att:
				msg.dmg = 35;
				msg.knockUpVel = 10;
				break;
			case Heavy_Air_Att:
				msg.knockbackVel = 5.0f;
				msg.dmg = 20;
				break;
			case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att: case Heavy_Walk_Att:
				msg.dmg = 30;
				msg.knockbackVel = 7.0f;
				break;
			case Throw:
				msg.isThrow = true;
				msg.dmg = 5;
				msg.knockbackVel = -2.0f;
				if (currentAnim == 16)
				{
					msg.dmg = 30;
					msg.knockbackVel = 20.0f;
				}
				break;
			case Special_Num_2:
				msg.dmg = 40;
				noSoundAttack = true;
				break;
			case Special_OraOraOra: 
				msg.dmg = 10;
				msg.isStoppingTime = false;
				break;
			case Ult_Num_1:
				msg.dmg = 17;
				break;
			case Special_Num_4:
				msg.dmg = 80;
				msg.knockbackVel = 20;
				noSoundAttack = true;
				break;
			}
		}
		else if (!isPlayerJotaro)
		{
			switch (currentAnimSequence)
			{
			case Light_Leg_Att: case Light_Crouch_Att:
				msg.dmg = 7.5;
				msg.knockbackVel = 1.0f;
				break;
			case Medium_Air_Att:
				msg.knockbackVel = 7.5f;
				msg.dmg = 7.5;
				break;
			case Medium_Att: case Medium_Walk_Att: 
				msg.dmg = 15;
				msg.knockbackVel = 2.5f;
				break;
			case Medium_Crouch_Att:
				msg.knockUpVel = 7.5f;
				msg.dmg = 7.5;
				break;
			case Heavy_Att:
				msg.dmg = 15;
				msg.knockUpVel = 10;
				break;
			case Heavy_Air_Att:
				msg.knockbackVel = 10.0f;
				msg.dmg = 15;
				break;
			case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att: case Heavy_Walk_Att:
				msg.dmg = 25;
				msg.knockbackVel = 5.0f;
				break;
			case Throw:
				msg.isThrow = true;
				msg.dmg = 5;
				msg.knockbackVel = -2.0f;
				if (currentAnim == 14)
				{
					msg.dmg = 30;
					msg.knockbackVel = 20.0f;
				}
				break;
			case Special_Num_2:
				msg.dmg = 60;
				noSoundAttack = true;
				break;
			case Special_OraOraOra: 
				msg.dmg = 10;
				break;
			case Ult_Num_1:
				if (currentAnim == 1 || currentAnim == 2)
				{
					msg.dmg = 20;
					if (faceDirectionRight)
						msg.knockbackVel = -70;
					else
						msg.knockbackVel = 70;

					msg.knockUpVel = 10;
				}
				else
				{
					msg.dmg =20;
					msg.knockbackVel = 2;
				}
		
				break;
			case Special_Num_4:
				msg.dmg = 30;
				msg.knockbackVel = 0.1;
				SoundManager.PlayPlayerSound(BloodSplatterSound, false,isPlayer1);
				noSoundAttack = true;
				break;
			}
		}
		if (!isGameMode1VS1)
		{
			for (int i = 0; i < EntityManager.m_Entities.size(); i++)
			{
				if (EntityManager.m_Entities[i]->isAMonster && (EntityManager.m_Entities[i]->isCollidingWithPlayer || EntityManager.m_Entities[i]->isCollidingWithPlayerStando) && EntityManager.m_Entities[i]->dmgImmunityTimer <= 0.0f)
				{
					if (player->Matrix().GetX() > EntityManager.m_Entities[i]->Matrix().GetX())
					{
						msg.isKnockbackedRight = false;
					}

					if (currentAnimSequence > Special_OraOraOra || currentAnimSequence < Ult_Num_1)
					{
						UltPointsAccumulator(msg.dmg / 2);
					}


					Messenger.SendMessage(EntityManager.m_Entities[i]->GetUID(), msg);
					Messenger.SendMessage(SystemUID, msg);

					if (currentAnimSequence >= Heavy_Att && currentAnimSequence <= Heavy_Walk_Att)
					{
						SoundManager.PlayMonsterSound(MonsterHardHitSound, false);
					}
					else if (currentStandoAnimSequence == Special_OraOraOra || currentStandoAnimSequence == Stando_Ult_1)
					{
						SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
					}
					else if (!noSoundAttack)
					{
						SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
					}

				}
			}
		}
		else if(thisDamageFrame != currentAnim )
		{
			if (isPlayer1)
			{
				if (EntityManager.player1CanHitplayer2 )
				{
					if (EntityManager.isPlayer1Ulting && EntityManager.isPlayer2Ulting)
					{
						EntityManager.DoubleUltCollisionEvent = true;
					}
					else
					{
						EntityManager.DoubleUltCollisionEvent = false;
					}
					if (player->Matrix().GetX() > EntityManager.player2Pos.x)
					{
						msg.isKnockbackedRight = false;
					}

					if (currentAnimSequence > Special_OraOraOra || currentAnimSequence < Ult_Num_1)
					{
						UltPointsAccumulator(msg.dmg / 2);
					}
					Messenger.SendMessage(Player2UID, msg);
					Messenger.SendMessage(SystemUID, msg);

					if (!EntityManager.DoubleUltCollisionEvent)
					{
						if (currentAnimSequence >= Heavy_Att && currentAnimSequence <= Heavy_Walk_Att)
						{
							SoundManager.PlayMonsterSound(MonsterHardHitSound, false);
						}
						else if (currentStandoAnimSequence == Special_OraOraOra || currentStandoAnimSequence == Stando_Ult_1)
						{
							SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
						}
						else if (!noSoundAttack)
						{
							SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
						}
					}
					
				}
			}
			if (!isPlayer1)
			{
				if (EntityManager.player2CanHitplayer1)
				{
					if (EntityManager.isPlayer1Ulting && EntityManager.isPlayer2Ulting)
					{
						EntityManager.DoubleUltCollisionEvent = true;
					}
					else
					{
						EntityManager.DoubleUltCollisionEvent = false;
					}
					if (player->Matrix().GetX() > EntityManager.player1Pos.x)
					{
						msg.isKnockbackedRight = false;
					}

					if (currentAnimSequence > Special_OraOraOra || currentAnimSequence < Ult_Num_1)
					{
						UltPointsAccumulator(msg.dmg / 2);
					}
					if (currentAnimSequence == Special_Num_4)
					{
						InterfaceManager.player2Hp += msg.dmg / 2;
					}
					Messenger.SendMessage(PlayerUID, msg);
					Messenger.SendMessage(SystemUID, msg);

					if (!EntityManager.DoubleUltCollisionEvent)
					{
						if (currentAnimSequence >= Heavy_Att && currentAnimSequence <= Heavy_Walk_Att)
						{
							SoundManager.PlayMonsterSound(MonsterHardHitSound, false);
						}
						else if (currentStandoAnimSequence == Special_OraOraOra || currentStandoAnimSequence == Stando_Ult_1)
						{
							SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
						}
						else if (!noSoundAttack)
						{
							SoundManager.PlayMonsterSound(MonsterBasicHitSound, false);
						}
					}
				}
			}
			
				thisDamageFrameStando = currentStandoAnim;
			
			thisDamageFrame = currentAnim;
		}
		

		


	}
	//Setting changes to the HP Bar frames, which show how many blocks we have left
	bool CPlayerEntity::RenderEntityUI(TFloat32 updateTime)
	{
		
		if (isPlayer1) {

			InterfaceManager.player1MaxHp = playerStats.hp_max;
			InterfaceManager.player1Hp = playerStats.hp;
			InterfaceManager.ChangeUIAnimFrame("PlayerHpBarLeft", AnimationManager.GetPlayerUITexturePath(HealthBarAnim, blockAvailable, isPlayerJotaro, isPlayer1));
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer1 >= 1000)
				InterfaceManager.CycleUltMeterReady(updateTime, isPlayerJotaro, isPlayer1);
		}
		else
		{
			InterfaceManager.player2MaxHp = playerStats.hp_max;
			InterfaceManager.player2Hp = playerStats.hp;
			InterfaceManager.ChangeUIAnimFrame("PlayerHpBarRight", AnimationManager.GetPlayerUITexturePath(HealthBarAnim, blockAvailable, isPlayerJotaro, isPlayer1));
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer2 >= 1000)
				InterfaceManager.CycleUltMeterReady(updateTime, isPlayerJotaro, isPlayer1);
		}
		return true;
    }  
	//The message is recieved and interpreted
	void CPlayerEntity::SwitchPlayerHpState(SMessage msg)
	{

		//The throw message passes through the block
		if (!msg.isThrow)
		{
			//but if the player is blocking and block timer didn`t go out, it will fizzle
			if (blockTimer > 0)
			{
				SoundManager.PlayPlayerSound(PlayerBlockAttackSound, false, isPlayer1);
				return;
			}

			if (KeyHeld(BlockKey) && blockAvailable > 0)
			{
				moveShieldOnce = false;
				SoundManager.PlayPlayerSound(PlayerBlockAttackStart, false, isPlayer1);
				blockTimer = 50.0f;
				blockAvailable--;
				return;
			}
		}

		//The attack animations can`t be interrupted by another attack
		if ((!isDamaged) && (currentAnimSequence < Special_OraOraOra || currentAnimSequence > Ult_Num_1))
		{
			
				int i = msg.knockbackVel;
				f_attackMoveDisplacer(-i * 3);
			
				
				//We interpret dmg received into ultimate accelerator, which gives extra energy
			UltPointsAccumulator(msg.dmg);

			if (msg.dmg > playerStats.hp)
			{
				playerStats.hp = 0;
			}
			else
				playerStats.hp = playerStats.hp - msg.dmg;

			//Here we tailor the face shown on HP Bar to the current state of player HP
			if (isPlayer1)
			{
				if (playerStats.hp < 160 && playerStats.hp > 110)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceLeft", AnimationManager.GetPlayerUITexturePath(FaceAnim, 1, isPlayerJotaro, isPlayer1));
				}
				if (playerStats.hp < 110 && playerStats.hp > 50)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceLeft", AnimationManager.GetPlayerUITexturePath(FaceAnim, 2, isPlayerJotaro, isPlayer1));
				}
				if (playerStats.hp < 50)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceLeft", AnimationManager.GetPlayerUITexturePath(FaceAnim, 3, isPlayerJotaro, isPlayer1));
				}
			}
			else
			{
				if (playerStats.hp < 160 && playerStats.hp > 110)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceRight", AnimationManager.GetPlayerUITexturePath(FaceAnim, 1, isPlayerJotaro, isPlayer1));
				}
				if (playerStats.hp < 110 && playerStats.hp > 50)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceRight", AnimationManager.GetPlayerUITexturePath(FaceAnim, 2, isPlayerJotaro, isPlayer1));
				}
				if (playerStats.hp < 50)
				{
					InterfaceManager.ChangeUIAnimFrame("PlayerFaceRight", AnimationManager.GetPlayerUITexturePath(FaceAnim, 3, isPlayerJotaro, isPlayer1));
				}
			}
			if (playerStats.hp <= 0)
			{
				
				if((isPlayer1 && EntityManager.player1LifeLeft <=0) || (!isPlayer1 && EntityManager.player2LifeLeft <=0) )
				InterfaceManager.isPlayerDead = true;
				
				isDamaged = false;
				
				if (isPlayer1)
				{
					EntityManager.player1LifeLeft--;
					EntityManager.CountDownToStart = 10;
					f_playAnimSeq(Is_Killed);
				}
				else
				{
					EntityManager.player2LifeLeft--;		
					EntityManager.CountDownToStart = 10;
					f_playAnimSeq(Is_Killed);
				}
				playerStats.hp = playerStats.hp_max;
				if (EntityManager.isPlayer1Jotaro)
				{
					InterfaceManager.player1Hp = 190;
					currentHpToShowPlayer1 = 190;

				}
				else
				{
					InterfaceManager.player1Hp = 210;
					currentHpToShowPlayer1 = 210;
				}
				
				if (EntityManager.isPlayer2Jotaro)
				{
					InterfaceManager.player2Hp = 190;
					currentHpToShowPlayer2 = 190;
				}
				else
				{
					InterfaceManager.player2Hp = 210;
					currentHpToShowPlayer2 = 210;
				}
			
			
				return;
			}
			if (msg.knockUpVel > 1.0f)
			{
				isKnockedUp = true;
				m_UpdwardVel = msg.knockUpVel;
				isInAir = true;
			}
			if (isInAir)
			{
				if (!isGameMode1VS1)
					isDamaged = true;

				f_playAnimSeq(Is_Hit_Air);
				return;
			}
			if (msg.dmg < 20)
			{
				if (rand)
					f_playAnimSeq(Is_Hit_Light);
				else
					f_playAnimSeq(Is_Hit_Light_2);
			}
			else if (msg.dmg > 20 && msg.dmg < 40)
			{
				if (rand)
					f_playAnimSeq(Is_Hit_Medium);
				else
					f_playAnimSeq(Is_Hit_Medium_2);
			}
			else if (msg.dmg > 40)
			{
				if (rand)
					f_playAnimSeq(Is_Hit_Hard);
				else
					f_playAnimSeq(Is_Hit_Hard_2);
			}
			else
			{
				if (rand)
					f_playAnimSeq(Is_Hit_Light);
				else
					f_playAnimSeq(Is_Hit_Light_2);
			}
			if (!isGameMode1VS1)
				isDamaged = true;
		}
	}
	//Converting damage dealt or received into ultimate points
	void CPlayerEntity::UltPointsAccumulator(int dmg)
	{
		
	
		if (isPlayer1)
		{
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer1  >  1000)
			{
				return;
			}
			ultPointsAccumulator = dmg * 12;
			InterfaceManager.ultEnergyDrainMeterPlayer1 += ultPointsAccumulator;
			
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer1  >  1000)
			{
				InterfaceManager.ultEnergyDrainMeterPlayer1 == 1000;
				return;
			}
			if (InterfaceManager.ultEnergyDrainMeterPlayer1 > 1000)
			{
				float overstack = InterfaceManager.ultEnergyDrainMeterPlayer1 - 1000;
				if (overstack > 1000)
					overstack = 1000;
			
				InterfaceManager.ultEnergyDrainMeterPlayer1 = overstack;

				ultPointsAccumulator = 0.0f;
				if (ultPointsAvailable < ultPointsMax)
				{
					ultPointsAvailable++;
					
						InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
						/*InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));*/
				}
		    }
			
		}
		if(!isPlayer1)
		{
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer2  >  1000)
			{
				return;
			}
			ultPointsAccumulator = dmg * 6;
			
			InterfaceManager.ultEnergyDrainMeterPlayer2 += ultPointsAccumulator * 2;
			
			if (ultPointsAvailable == ultPointsMax && InterfaceManager.ultEnergyDrainMeterPlayer2  >  1000)
			{
				InterfaceManager.ultEnergyDrainMeterPlayer2 == 1000;
				return;
			}
			if (InterfaceManager.ultEnergyDrainMeterPlayer2 > 1000)
			{
				float overstack = InterfaceManager.ultEnergyDrainMeterPlayer2 - 1000;
				if (overstack > 1000)
					overstack = 1000;
				InterfaceManager.ultEnergyDrainMeterPlayer2 = overstack;

				ultPointsAccumulator = 0.0f;
				if (ultPointsAvailable < ultPointsMax)
				{
					ultPointsAvailable++;

					/*InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));*/
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				}
			}
		}
		
	}
	//When stand is summoned, it drains energy from the meter
	void CPlayerEntity::DrainEnergy(TFloat32 updateTime)
	{
		if (isPlayer1)
		{
			standoEnergyDrain = updateTime * AnimMultSlow;
			InterfaceManager.ultEnergyDrainMeterPlayer1 -= standoEnergyDrain;
			if (InterfaceManager.ultEnergyDrainMeterPlayer1 < 0 && ultPointsAvailable > 0)
			{
				InterfaceManager.ultEnergyDrainMeterPlayer1 = 1000.0f;
				ultPointsAvailable--;
				if (isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				else
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));

			}
			if (ultPointsAvailable == 0 && !isAttacking && InterfaceManager.ultEnergyDrainMeterPlayer1 < 0)
			{
				f_Unsummon_Stando();
			}
		}
		else
		{
			standoEnergyDrain = updateTime * AnimMultSlow;
			InterfaceManager.ultEnergyDrainMeterPlayer2 -= standoEnergyDrain;
			if (InterfaceManager.ultEnergyDrainMeterPlayer2 < 0 && ultPointsAvailable > 0)
			{
				InterfaceManager.ultEnergyDrainMeterPlayer2 = 1000.0f;
				ultPointsAvailable--;
				if (isPlayer1)
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterLeft", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));
				else
					InterfaceManager.ChangeUIAnimFrame("PlayerUltMeterRight", AnimationManager.GetPlayerUITexturePath(Ult_Meter_Anim, ultPointsAvailable, isPlayerJotaro, isPlayer1));

			}
			if (ultPointsAvailable == 0 && !isAttacking && InterfaceManager.ultEnergyDrainMeterPlayer2 < 0)
			{
				f_Unsummon_Stando();
			}
		}
		
	}

	//Damage accumulator is the player shield, which is shown for a short time until block timer expires
	void CPlayerEntity::DamageAccumulator(TFloat32 updateTime, bool isFacingRight)
	{
		blockTimer -= updateTime * AnimMultSlow;
		if(EntityManager.zaWarudoEnabled && isPlayerJotaro)
			blockTimer -= updateTime * AnimMultSplitSec;
		if (blockTimer > 0.0f)
		{
			int shieldPosModX;
			if (faceDirectionRight)
				shieldPosModX = 4;
			else
				shieldPosModX = 0;
			if(isPlayer1)
			EntityManager.GetEntity("PlayerShieldJotaro")->Matrix().SetPosition(CVector3(player->Matrix().GetX() + shieldPosModX, player->Matrix().GetY() + 2, player->Matrix().GetZ() - 10));
			else
				EntityManager.GetEntity("PlayerShieldDio")->Matrix().SetPosition(CVector3(player->Matrix().GetX() + shieldPosModX, player->Matrix().GetY() + 2, player->Matrix().GetZ() - 10));
			isBlockingTimer0 = false;
		
			if(isPlayer1)
			InterfaceManager.blockTimeRemainsPlayer1 = blockTimer;
			else
				InterfaceManager.blockTimeRemainsPlayer2 = blockTimer;
		}


		if (blockTimer <0.0f && blockTimer != 0)
		{
			if (!moveShieldOnce)
			{
				if(isPlayer1)
				EntityManager.GetEntity("PlayerShieldJotaro")->Matrix().SetPosition(CVector3(1000, 10000, 10000));
				else
					EntityManager.GetEntity("PlayerShieldDio")->Matrix().SetPosition(CVector3(1000, 10000, 10000));
				moveShieldOnce = true;
			}

			blockResetTimer += updateTime * AnimMultNormal;
			
			if (blockResetTimer >= 4000 && blockAvailable < 3)
			{
				blockAvailable++;
				blockResetTimer = 0;
			}
		}
		if (isDamaged)
		{
			damageTransparencyTimer += updateTime * 20;
		}
		if (damageTransparencyTimer > 1.0f)
		{
			isTransparent = !isTransparent;
			damagedTimer += damageTransparencyTimer;
			damageTransparencyTimer = 0.0f;
		}
		if (damagedTimer > 20.0f)
		{
			isDamaged = false;
			damagedTimer = 0.0f;
			damageTransparencyTimer = 0.0f;
		}
	}
	//Animation controller for Jotaro.
	//Every animation has effect on certain frames, while continuing or when in end
	//Also sets the playback speed for certain animations
	bool CPlayerEntity::SwitchAnimStateJotaro(TFloat32 updateTime, bool isFacingRight, int AnimationType, string fullFileName)
	{
		switch (AnimationType)
		{
		case Intro:
			if (InterfaceManager.GameStart && currentAnim == 2)
			{
				MainCamera->Matrix().SetX(player->Matrix().GetX());
				SoundManager.PlayPlayerSound(HellToYouSound, false, false);
			}
			if (!InterfaceManager.GameStart)
			{
				currentAnim = 1;
			}
		case Walking:
			if (currentAnim == 0 || currentAnim == 7)
			{
				SoundManager.PlayPlayerSound(PlayerFootstepSound, false, isPlayer1);
			}
			break;
		case Block:
			if (currentAnim == 2 && buttonPressed)
			{
				currentAnim--;
			}
			break;
		case Crouch_Block: case Block_Air:
			if (currentAnim == 1 && isBlocking)
			{
				currentAnim--;
			}
			if (currentAnimSequence == Block_Air)
			{
				if (!isInAir)
				{
					currentAnim = 0;
					animationLock = false;
					currentAnimSequence = Block;
				}
			}

			break;

		case Dash: 
			if (currentAnim > 1 && currentAnim < 5)
			{		
				f_attackMoveDisplacer(15.0f);
			}
			break;
		case Dash_Back:
			if (currentAnim > 1 && currentAnim < 5)
			{
				f_attackMoveDisplacer(-15.0f);
			}
			break;

		case Light_Leg_Att:
			if (currentAnim > 1 && currentAnim < 5)
			{
				f_attackMoveDisplacer(2.5f);
			}
			break;
		case Medium_Att:
			if (currentAnim > 1 && currentAnim < 6)
			{
				f_attackMoveDisplacer(3.0f);
			}
			break;
		case Medium_Walk_Att:
			if (currentAnim == 5)
			{
				f_attackMoveDisplacer(15.0f);
			}
			break;
		case Medium_Crouch_Att:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 14)
			{
				f_Rescale(false, player);
			}
			break;
		case Medium_Air_Att:
			if (currentAnim == 1)
			{
				f_attackMoveDisplacer(5.0f);
			}
			break;
		case Heavy_Att:
			if (currentAnim == 2)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 23)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Walk_Att:
			if (currentAnim == 4)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 5)
			{
				f_attackMoveDisplacer(10.0f);
			}
			else if (currentAnim == 17)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Crouch_Att:
			if (currentAnim == 3)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 12)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Crouch_Fr_Att:
			if (currentAnim == 4)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 10)
			{
				f_Rescale(false, player);
			}
			break;
		case Throw:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			else if (currentAnim == 3)
			{
				f_attackMoveDisplacer(5.0f);
			}
			else if (currentAnim == 26)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Air_Att:
			if (currentAnim == 3)
			{
				f_attackMoveDisplacer(10.0f);
			}
			break;
		case Special_OraOraOra:
			if (currentAnim == 8 && currentStandoAnimSequence == Stando_OraOraOra)
			{
				currentAnim = 3;
			}
			break;
		case Special_Num_3:
			if (currentAnim >= 9 && currentStandoAnimSequence == Stando_Sp_3)
			{
				currentAnim--;
			}
			break;
		case Ult_Num_1: case Special_Num_4:
			
			if (currentAnim == 4 && (currentStandoAnimSequence == Stando_Ult_1 || currentStandoAnimSequence == Stando_Sp_4))
			{
				currentAnim--;
			}
			if (currentAnim == 2 && currentStandoAnimSequence == Stando_Ult_1)
			{
				if (isPlayer1)
				{
					EntityManager.doNotUpdatePlayer2 = false;
				}
				else
					EntityManager.doNotUpdatePlayer1 = false;

				

				SoundManager.PlayStandoSound(OraOraUltimateSound, false);
				animCycleDelay = 10.5;
			}
			break;

		case Jump:
			if (isInAir)
			{
				f_physGravity(updateTime);
			}
			break;
		case Is_Hit_Air:
			if (isInAir)
			{
				if (currentAnim >= 4)
					currentAnim == 4;

				f_physGravity(updateTime);
			}
		case Is_Killed:
			f_attackMoveDisplacer(-1.0f);
			break;
		default:
			if (isInAir)
				f_physGravity(updateTime);
			break;
		}
		if (animChangeTimer >= animCycleDelay )
		{
			if (currentAnimSequence != Summon && currentAnimSequence != Summon_Air && currentStandoAnimSequence == Stando_Idle && currentAnimSequence != Summon_Air && !isAttacking)
			{
				if (player->Matrix().GetScaleX() > 2.0f)
				{
					f_Rescale(false, player);
					Enlarged = false;
				}
			}
			this->player->Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &this->player->Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}




			currentAnim++;
			if (((AnimationType == Jump && currentAnim >= 11)) && isInAir)
			{
				currentAnim--;
			}

			if (currentAnim >= animsRight[AnimationType].size())
			{
				currentAnim = 0;
				thisDamageFrame = 0;
				isUlting = false;
				switch (currentAnimSequence)
				{
				case Turning:
					animationLock = false;
					currentAnimSequence = Walking;
					break;
				case Crouch:
					animationLock = false;
					currentAnim = animsRight[AnimationType].size() - 1;
					break;
				case Medium_Air_Att: case Heavy_Air_Att: case Block_Air:

					if (isInAir)
					{
						currentAnimSequence = Jump;
						m_UpwardVelocity = 0.0f;
						currentAnim = animsRight[AnimationType].size() - 1;
					}
					else
					{
						animationLock = false;
						currentAnimSequence = Idle;
						currentAnim = 0;
					}
					break;
				case Crouch_Turn: case Light_Crouch_Att: case Medium_Crouch_Att: case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att:  case Crouch_Block: case Is_Hit_Crouch: case Is_Hit_Crouch_2:

					if (currentAnimSequence == Crouch_Turn)
					{
						faceDirectionRight = !faceDirectionRight;
					}
					if (currentAnimSequence == Heavy_Crouch_Att || currentAnimSequence == Heavy_Crouch_Fr_Att || currentAnimSequence == Medium_Crouch_Att)
					{
						f_Create_Stando();
					}
					if (currentAnimSequence == Crouch_Block)
					{
						isBlocking = false;
					}
					currentAnimSequence = Crouch;
					animationLock = false;
					isAttacking = false;
					currentAnim = animsRight[Crouch].size() - 1;
					break;
				case Heavy_Att: case Heavy_Walk_Att: case Throw:
					animationLock = false;
					currentAnimSequence = Idle;
					f_Create_Stando();
					isAttacking = false;
					if (EntityManager.DoubleUltCollisionEvent)
					{
						EntityManager.DoubleUltCollisionEventFinish = true;
						MainCamera->Matrix().SetPosition(CVector3(0, 50, -150));
					}
					break;
				case Is_Hit_Air:
					currentAnim = 0;
					currentAnimSequence = Stand_Up;
					break;
				case Victory:
					victoryPoseCounter++;
					currentAnim = 8;
					break;
				case Victory_2:
					victoryPoseCounter++;
					currentAnim =13;
					break;
				

				default:
					if (currentAnimSequence == Intro)
					{
						f_playAnimSeq(Summon);
						EntityManager.player1IntroFinished = true;
						
					}
					if (currentAnimSequence == Is_Killed )
					{
						if (isPlayer1 && EntityManager.player1LifeLeft < 0)
						{
							if (isPlayer1)
							{
								isDeadPlayer1 = true;
								SMessage msg;
								msg.type = Msg_Victory;
								msg.knockbackVel = 0.1;
								Messenger.SendMessage(Player2UID, msg);
							}
							
								
							
							currentAnim = 12;
							break;
						}
						else if (!isPlayer1 && EntityManager.player2LifeLeft <= 0)
						{
							isDeadPlayer2 = true;
							SMessage msg;
							msg.type = Msg_Victory;
							msg.knockbackVel = 0.1;
							Messenger.SendMessage(PlayerUID, msg);
							currentAnim = 12;
							break;
						}
					}
					if (isAttacking)
					{
						isAttacking = false;
					}
					if (currentAnimSequence == Summon_Air || currentAnimSequence == Summon)
					{
						f_Create_Stando();
						if (isInAir)
						{
							m_UpwardVelocity = 0.0f;
							currentAnimSequence = Jump;
							currentAnim = animsRight[AnimationType].size() - 1;
							return true;
						}

					}
					
					if (currentAnimSequence == Jump)
					{
						airAttackCount = 0;
						isInAir = false;
						isAttacking = false;
					}
					if (currentAnimSequence == Block)
					{
						isBlocking = false;
					}
					if (currentStandoAnimSequence == Stando_Sp_2)
					{
						currentAnim = 0;
						return false;
					}

					animationLock = false;
					currentAnimSequence = Idle;
					break;
				}
			}
			animChangeTimer = 0.0f;

		}
		switch (currentAnimSequence)
		{
		case Intro:
			animChangeTimer += updateTime * AnimMultSlow * 0.5;
			break;
		case Idle: case Crouch_Turn:  case Special_Num_2:   case Is_Killed: case Victory: case Ult_Num_1: 
			animChangeTimer += updateTime * AnimMultSlow;
			break;
		case Walking: case Crouch: case Dash: case Dash_Back: case Summon: case Light_Crouch_Att: case Heavy_Air_Att: case Medium_Air_Att: case Throw:  case Victory_2: case Medium_Crouch_Att: 
			animChangeTimer += updateTime * AnimMultNormal;
			break;
		case Turning: case Stand_Up: case Jump: case Summon_Air: case Light_Leg_Att: case Medium_Walk_Att:  case Special_Num_3: case Special_OraOraOra: case Block:
			animChangeTimer += updateTime * AnimMultFast;
			break;
		case Heavy_Att: case Heavy_Walk_Att: case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att:
			if(currentAnimSequence == Heavy_Att && EntityManager.DoubleUltCollisionEvent)
				{
				animChangeTimer += updateTime * AnimMultSlow * 0.1;
				}
			else
			animChangeTimer += updateTime * AnimMultSplitSec;
		default:
			animChangeTimer += updateTime * AnimMultNormal;
			break;
		}
    }
	//Same function type as above but now for Dio
	bool CPlayerEntity::SwitchAnimStateDio(TFloat32 updateTime, bool isFacingRight, int AnimationType, string fullFileName)
	{
		if (EntityManager.zaWarudoEnabled)
		{
			zaWarudoTimer -= updateTime * AnimMultSplitSec * 1.25;
			if (zaWarudoTimer <= 0)
			{
				EntityManager.zaWarudoEnabled = false;
				thisUnaffected = false;
				EntityManager.GetEntity("ZaWarudoSphere")->Matrix().SetScale(CVector3(0.05f, 0.05f, 0.05f));
				EntityManager.GetEntity("ZaWarudoSphere")->Matrix().SetPosition(CVector3(-10000, -10000, -10000));
			}
		}
		
		if (EntityManager.DoubleUltCollisionTimer > 5.0f && currentAnim > 1)
		{
			int i = 0;
		}
		
		switch (AnimationType)
		{
			case Intro_2:
				if (!EntityManager.player1IntroFinished)
				{
					currentAnim = 1;
				}
				else MainCamera->Matrix().SetX(player->Matrix().GetX());
				if (currentAnim == 2)
				{
					SoundManager.PlayPlayerSound(DioIntroSound, false, false);
				}
				break;
		case Walking:
			if (currentAnim == 0 || currentAnim == 7 )
			{
				SoundManager.PlayPlayerSound(PlayerFootstepSound, false, isPlayer1);
			}
			break;
		case Block:
			if (currentAnim == 11 && buttonPressed)
			{
				currentAnim--;
			}
			break;
		case Crouch_Block: case Block_Air:
			if (currentAnimSequence == Block_Air && currentAnim == 10 && isBlocking)
			{
				currentAnim--;		
			}
			if (currentAnim == 13 && isBlocking)
			{
				currentAnim--;
			}
	
			if (currentAnimSequence == Block_Air)
			{
				if (!isInAir)
				{
					currentAnim = 0;
					animationLock = false;
					currentAnimSequence = Block;
				}
			}

			break;

		case Dash:
			if (currentAnim > 1 && currentAnim < 5)
			{
				f_attackMoveDisplacer(10.0f);
			}
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			 if (currentAnim == 2)
			{
				f_Rescale(false, player);
			}
			break;
		case Dash_Back:

			
			
			if (currentAnim == 1)
			{
				
				f_Rescale(true, player);
			}
			 if (currentAnim >=3 && currentAnim <=4 )
			{
				f_attackMoveDisplacer(-12.5f);
			}
			else  if (currentAnim == 6)
			 {
				f_Rescale(false, player);
				
			 }
			
			break;
		
		case Summon_Air: case Summon: 
			if (currentAnim == 2)
			{
				f_Rescale(true, player);
			}
			break;
		case Light_Leg_Att:
			if (currentAnim > 1 && currentAnim < 3)
			{
				f_attackMoveDisplacer(1.5f);
			}
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			if (currentAnim == 3)
			{
				f_Rescale(false, player);
			}
			
			break;
		case Light_Crouch_Att:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			 
			break;
		case Medium_Att: case Medium_Walk_Att:
			
				if (currentAnim == 3 || currentAnim == 8)
				{
					f_attackMoveDisplacer(-5.5f);
					f_Rescale(true, player);
				}
				 if (currentAnim == 6 || currentAnim == 10)
				{
					f_Rescale(false, player);
				}
			break;
		case Medium_Crouch_Att:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			if (currentAnim == 5)
			{
				f_attackMoveDisplacer(7.5f);
			}
			break;
		case Medium_Air_Att:
			if (currentAnim == 2)
			{
				f_Rescalex2(true, player);

				f_attackMoveDisplacer(5.0f);
			}
			break;
		case Heavy_Att:
			if (currentAnim == 0)
			{
				f_Rescale(true, player);
			}
			
			 if (currentAnim == 7)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Walk_Att:
			if (currentAnim == 4)
			{
				f_Rescale(true, player);
			}
			if (currentAnim > 2 && currentAnim < 6)
			{
				f_attackMoveDisplacer(4.0f);
			}
			 if (currentAnim == 6)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Crouch_Att:
			if (currentAnim == 4)
			{
				f_attackMoveDisplacer(17.5f);
				f_Rescale(true, player);
			}
			 if (currentAnim == 14)
			{
				f_Rescale(false, player);
			}
			break;
		case Heavy_Crouch_Fr_Att:
			if (currentAnim == 4)
			{
				f_Rescale(true, player);
			}
			 if (currentAnim == 10)
			{
				f_Rescale(false, player);
			}
			break;
		case Throw:
			if (currentAnim == 5)
			{
				f_Rescale(true, player);
			}
			break;
		case Heavy_Air_Att:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			
			if (currentAnim == 3)
			{
				f_attackMoveDisplacer(10.0f);
			}
			break;
		case Special_OraOraOra:
			if (currentAnim == 7 && currentStandoAnimSequence == Stando_OraOraOra)
			{
				currentAnim = 3;
			}
			break;
		case Special_Num_2:
			if (currentAnim == 5)
			{
				f_Rescale(true, player);
			}
			if (currentAnim == 14)
			{
				SoundManager.PlayPlayerSound(KnivesFlySound, false,isPlayer1);
				EntityManager.knivesFlyRight = isFacingRight;
				EntityManager.knivesOwnerPlayer1 = isPlayer1;
				if (isFacingRight)
				{
					EntityManager.GetEntity("KnivesRight")->Matrix().SetPosition(CVector3(player->Matrix().GetX() + 10, player->Matrix().GetY() + 5, player->Matrix().GetZ()));
				}
				else
					EntityManager.GetEntity("KnivesLeft")->Matrix().SetPosition(CVector3(player->Matrix().GetX() - 10, player->Matrix().GetY() + 5, player->Matrix().GetZ()));
				EntityManager.knivesCreated = true;
			}
			
			 if (currentAnim == 19)
			{
				f_Rescale(false, player);
			}
			 break;
		case Special_Num_3:
			if (currentAnim == 0) {
				player->Matrix().SetScale(CVector3(1.85, 1.85, 1.85));
				player->Matrix().SetY(15.0f);
				
			}
			if (currentAnim == 15)
			{
				EntityManager.GetEntity("ZaWarudoSphere")->Matrix().SetPosition(CVector3(MainCamera->Matrix().GetX(), MainCamera->Matrix().GetY(), MainCamera->Matrix().GetZ() + 20));
				EntityManager.zaWarudoEnabled = true;
				thisUnaffected = true;
				zaWarudoTimer = 6000;
			}
			if (currentAnim > 15)
			{
				EntityManager.GetEntity("ZaWarudoSphere")->Matrix().ScaleX(1.05);
				EntityManager.GetEntity("ZaWarudoSphere")->Matrix().ScaleY(1.05);
			}
			if (currentAnim == 25)
			{
				player->Matrix().SetScale(CVector3(1.5, 1.5, 1.5));
				player->Matrix().SetY(m_GroundLevel);
			}
		
			
			break;
		case Special_Num_4:
			if (currentAnim == 2)
			{
				player->Matrix().SetScale(CVector3(1.75, 1.75, 1.75));
				player->Matrix().SetY(15.0f);
			}
			if (currentAnim >= 13 && currentAnim < 19)
			{
				f_attackMoveDisplacer(6.0f);
			}
			if (currentAnim == 19)
			{
				player->Matrix().SetScale(CVector3(1.5, 1.5, 1.5));
				player->Matrix().SetY(m_GroundLevel);
			}
			if (currentAnim == 27)
			{
				player->Matrix().SetScale(CVector3(1.75, 1.75, 1.75));
				player->Matrix().SetY(15.0f);
			}
			if (currentAnim == 43)
			{
				player->Matrix().SetScale(CVector3(1.5, 1.5, 1.5));
				player->Matrix().SetY(m_GroundLevel);
			}
			break;
		case Ult_Num_1:
			
			
				if (currentAnim >= 1 && player->Matrix().GetY() > m_GroundLevel + 15 && !EntityManager.DoubleUltCollisionEvent)
				{
					if (isGameMode1VS1)
					{
						if (isPlayer1)
						{
							player->Matrix().SetX(EntityManager.player2Pos.x);
						}
						else
							player->Matrix().SetX(EntityManager.player1Pos.x);
					}
					player->Matrix().MoveY(-10);
					currentAnim = 1;
					if (player->Matrix().GetY() < m_GroundLevel + 15)
					{
						isInAir = false;
						player->Matrix().SetY(15);
					}
				}
			
			
			else if(currentAnim >= 12 && !EntityManager.DoubleUltCollisionEvent)
			{
				f_attackMoveDisplacer(4.0f);
				if (player->Matrix().GetX() < -90 || player->Matrix().GetX() > 90)
				{
					faceDirectionRight = !faceDirectionRight;
				}
			}
			if (currentAnim > 30 && EntityManager.DoubleUltCollisionEvent)
			{
				currentAnim = 15;
			}
			break;
		case Jump:
			if (currentAnim == 0)
			{
				f_Rescale(true, player);
			}
			 if (currentAnim == 8)
			{
				f_Rescale(false, player);
			}
			 if (currentAnim == 9 && isInAir) {
				currentAnim = 8;
			}
			if (isInAir)
			{
				f_physGravity(updateTime);
			}
			break;
		case Is_Hit_Hard_2:
			if (currentAnim == 1)
			{
				f_Rescale(true, player);
			}
			 if (currentAnim == 6)
			{
				f_Rescale(false, player);
			}
			break;
		case Is_Hit_Air:
			if (isInAir)
			{
				if (currentAnim >= 6)
					currentAnim = 6;

				f_physGravity(updateTime);
			}
			if (currentAnim == 0 || currentAnim == 7 || currentAnim == 11)
			{
				f_Rescale(true, player);
			}
			 if (currentAnim == 4 || currentAnim == 8 || currentAnim == 17)
			{
				f_Rescale(false, player);
			}
			break;
		case Is_Killed:
			if (currentAnim == 0)
			{
				f_Rescale(true, player);
			}
			f_attackMoveDisplacer(-1.0f);
			break;
		default:
			break;
		}
		if (isInAir && isBlocking)
			f_physGravity(updateTime);
		if (animChangeTimer >= animCycleDelay )
		{
			if (currentAnimSequence != Summon && currentAnimSequence != Summon_Air  && currentAnimSequence != Summon_Air && !isAttacking && currentAnimSequence != Special_Num_3 && currentAnimSequence != Special_Num_4 && currentAnimSequence != Ult_Num_1)
			{
				if (player->Matrix().GetScaleX() > 2.0f)
				{
					f_Rescale(false, player);
					Enlarged = false;
				}
			}
			this->player->Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &this->player->Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}

			currentAnim++;
			if (currentAnim >= animsRight[AnimationType].size())
			{
				currentAnim = 0;
				thisDamageFrame = 0;
				
				switch (currentAnimSequence)
				{
				case Turning:
					animationLock = false;
					currentAnimSequence = Walking;
					break;
				case Crouch:
					animationLock = false;
					currentAnim = animsRight[AnimationType].size() - 1;
					break;
				case Medium_Air_Att: case Heavy_Air_Att: case Block_Air:

					if (isInAir)
					{
						if (currentAnimSequence == Medium_Air_Att)
							f_Rescale(false, player);

						currentAnimSequence = Jump;
						m_UpwardVelocity = 0.0f;
						currentAnim = animsRight[currentAnimSequence].size() - 1;
					}
					else
					{
						animationLock = false;
						currentAnimSequence = Idle;
						currentAnim = 0;
					}
					break;
				case Crouch_Turn: case Light_Crouch_Att: case Medium_Crouch_Att: case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att:  case Crouch_Block: case Is_Hit_Crouch: case Is_Hit_Crouch_2:

					if (currentAnimSequence == Crouch_Turn)
					{
						faceDirectionRight = !faceDirectionRight;
					}
					
					if (currentAnimSequence == Crouch_Block)
					{
						isBlocking = false;
					}
					currentAnimSequence = Crouch;
					animationLock = false;
					isAttacking = false;
					currentAnim = animsRight[Crouch].size() - 1;
					break;
				case Heavy_Att:  case Throw:
					if (currentAnimSequence == Throw)
					{
						f_Rescale(false, player);
					}
					animationLock = false;
					currentAnimSequence = Idle;
					f_Create_Stando();
					isAttacking = false;
					break;
				case Is_Hit_Air:
					currentAnim = 0;
					currentAnimSequence = Stand_Up;
					break;
		
				case Victory:
					currentAnim = 10;
					victoryPoseCounter++;
					break;
				case Victory_2:
					victoryPoseCounter++;
					currentAnim = 23;
					break;
				default:
					if (currentAnimSequence == Intro_2)
					{
						f_playAnimSeq(Summon);
						MainCamera->Matrix().SetX(0);
						EntityManager.player2IntroFinished = true;
						InterfaceManager.isCountDownToStart = true;
					}
					if (currentAnimSequence == Special_Num_3)
					{
						player->Matrix().SetScale(CVector3(1.5, 1.5, 1.5));
						player->Matrix().SetY(m_GroundLevel);
					}
					if (currentAnimSequence == Ult_Num_1)
					{
						player->Matrix().SetScale(CVector3(1.1, 1.1, 1.1));
						player->Matrix().SetY(m_GroundLevel);
						player->Matrix().MoveZ(-10);
						EntityManager.RoddaRolla = false;
					}
					if (isAttacking)
					{
						isAttacking = false;
					}
					if (currentAnimSequence == Summon_Air || currentAnimSequence == Summon)
					{
						f_Create_Stando();
						if (isInAir)
						{
							m_UpwardVelocity = 0.0f;
							currentAnimSequence = Jump;
							currentAnim = animsRight[currentAnimSequence].size() - 1;
							return true;
						}

					}
					if (currentAnimSequence == Is_Killed)
					{
						if (isPlayer1 && EntityManager.player1LifeLeft < 0)
						{
							if (isPlayer1)
							{
								isDeadPlayer1 = true;
								SMessage msg;
								msg.type = Msg_Victory;
								msg.knockbackVel = 0.1;
								Messenger.SendMessage(Player2UID, msg);
							}
							currentAnim = 6;
							break;
						}
						else if (!isPlayer1 && EntityManager.player2LifeLeft < 0)
						{
							currentAnim = 6;
							isDeadPlayer2 = true;
							SMessage msg;
							msg.type = Msg_Victory;
							msg.knockbackVel = 0.1;
							Messenger.SendMessage(PlayerUID, msg);
							currentAnim = 12;
							break;
							break;
						}
						
					}
					if (currentAnimSequence == Jump)
					{
						airAttackCount = 0;
						isInAir = false;
						isAttacking = false;
					}
					if (currentAnimSequence == Block)
					{
						isBlocking = false;
					}
					if (currentStandoAnimSequence == Stando_Sp_2)
					{
						currentAnim = 0;
						return false;
					}

					animationLock = false;
					currentAnimSequence = Idle;
					break;
				}
			}
			animChangeTimer = 0.0f;

		}
		switch (currentAnimSequence)
		{
		case Is_Killed: case Victory:   case Intro_2:
			animChangeTimer += updateTime * AnimMultSlow;
			break;
		case Special_Num_3:  case Light_Leg_Att:
			animChangeTimer += updateTime * AnimMultSlow * 1.25;
			break;
		case Medium_Att: case Medium_Walk_Att:  case Special_Num_2: case Dash_Back: case Crouch_Turn: case Heavy_Walk_Att:
			animChangeTimer += updateTime * AnimMultSlow * 2.5;
			break;
		 case Idle: case Walking: case Crouch: case Dash:   case Light_Crouch_Att: case Heavy_Air_Att: case Medium_Air_Att: case Throw:  case Victory_2: case Medium_Crouch_Att:  case Special_Num_4: 
			animChangeTimer += updateTime * AnimMultNormal;
			break;
		case Turning: case Stand_Up: case Jump:   case Special_OraOraOra: 
			animChangeTimer += updateTime * AnimMultFast;
			break;
		case Heavy_Att:  case Heavy_Crouch_Att: case Heavy_Crouch_Fr_Att: case Summon: case Block: case Crouch_Block:  case Summon_Air: case Block_Air:
			animChangeTimer += updateTime * AnimMultSplitSec;
			break;
		case Ult_Num_1:
			if(currentAnim < 10)
				animChangeTimer += updateTime * AnimMultSlow * 0.4;
			else
			animChangeTimer += updateTime * AnimMultSplitSec * 0.8 ;
			break;
		default:
			animChangeTimer += updateTime * AnimMultNormal;
			break;
		}
		return true;
	}
	//Animating Dio Stando
	//His stando has significantly less animations than one Jotaro has because most of them were included into Dio`s sprites
	bool CPlayerEntity::Animate_StandoDio(TFloat32 updateTime, bool isFacingRight, int AnimationType)
	{
		f_Create_Stando();
		string fullFileName;
		if (isFacingRight)
		{
			fullFileName = MediaFolder + animsRight[AnimationType][currentStandoAnim].second;
		}
		else if (!isFacingRight)
		{
			fullFileName = MediaFolder + animsLeft[AnimationType][currentStandoAnim].second;
		}
		stando->Mesh->m_Materials->textures[0]->Release();
		if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &stando->Mesh->m_Materials->textures[0], NULL)))
		{
			string errorMsg = "Error loading texture " + fullFileName;
			SystemMessageBox(errorMsg.c_str(), "Mesh Error");
			return false;
		}
		if (AnimationType == Stando_Idle)
		{
			isStandoIdle = true;
			stando->Matrix().MoveY(sin(m_StandoFloating));
			m_StandoFloating += 0.01;
			return true;
		}
		if (standoAnimChangeTimer >= animCycleDelay)
		{
			isAttacking = true;
			switch (AnimationType)
			{

			case Stando_OraOraOra:
				
				if (currentStandoAnim == 2)
				{
					stando->Matrix().SetScale(CVector3(2, 1.5, 1.5));
				}
				
				break;
			default:
				break;
			}

			currentStandoAnim++;

			if (currentStandoAnim >= animsRight[AnimationType].size())
			{

				currentStandoAnim = 0;
				currentStandoAnimSequence = Stando_Idle;
				m_Stando_Ult_Displacement = 0;
				if (stando->Matrix().GetScaleX() > 1.5) {
					stando->Matrix().SetScaleX(1.5f);
				}
				if (ultPointsAvailable == 0)
				{
					f_Unsummon_Stando();
				}
			}

			standoAnimChangeTimer = 0.0f;
		}
		switch (currentStandoAnimSequence)
		{


		case Stando_OraOraOra:
			standoAnimChangeTimer += updateTime * AnimMultFast;
			break;
		default:
			standoAnimChangeTimer += updateTime * AnimMultNormal;
			break;
		}

		for (int i = 0; i <= (AttackAnimsPlayerFinish - AttackAnimsPlayerStart); i++)
		{
			if (i == currentStandoAnimSequence - AttackAnimsPlayerStart)
			{
				for (int j = 0; j < hitFrames[i].second.size(); j++)
				{
					if (hitFrames[i].second[j] == currentStandoAnim)
					{
						HitMessageComposer();
						SoundManager.PlayPlayerSound(PlayerBasicAttackSound, false, isPlayer1);
					}
				}
			}
		}

		return true;
	}
	void CPlayerEntity::SetupPlayer(bool isPlayer1,bool isPlayerJotaro)
	{
		SoundManager.Player1 = isPlayer1;
		stando = NULL;
		if (isPlayer1)
		{
			InterfaceManager.player1Jotaro = isPlayerJotaro;
			playerStats.hp_max = 190;
			playerStats.hp = 190;
			setHpTo = 190;
			standoEnergyDrain = 0.0f;
			ultPointsAvailable = 2;
		}
		else
		{
			InterfaceManager.player2Jotaro = isPlayerJotaro;
			playerStats.hp_max = 210;
			playerStats.hp = 210;
			setHpTo = 210;
			standoEnergyDrain = 0.0f;
			ultPointsAvailable = 3;
		}
		
		for (int i = 0; i < PlayerAnimationTypes; i++)
		{
			this->animsRight[i] = AnimationManager.GetAnimSequence(i, isPlayerJotaro, true);
			this->animsLeft[i] = AnimationManager.GetAnimSequence(i, isPlayerJotaro, false);
		}
		
    }
	void CPlayerEntity::SetupControls(bool isPlayer1)
	{
		if (isPlayer1)
		{
			JumpKey = Key_W;
			CrouchKey = Key_S;
			GoLeftKey = Key_A;
			GoRightKey = Key_D;
			Dash_FrontKey = Key_2;
			Dash_BackKey = Key_1;
			SummonKey = Key_R;
			BlockKey = Key_F;
			LightAttackKey = Key_V;
			MediumAttackKey = Key_B;
			HeavyAttackKey = Key_N;
			Special1Key = Key_1;
			Special2Key = Key_2;
			Special3Key = Key_3;
			Special4Key = Key_4;
			
		}
		else
		{
			JumpKey = Key_Numpad8;
			CrouchKey = Key_Numpad2;
			GoLeftKey = Key_Numpad4;
			GoRightKey = Key_Numpad6;
			Dash_FrontKey = Key_Numpad9;
			Dash_BackKey = Key_Numpad7;
			SummonKey = Key_Numpad0;
			BlockKey = Key_Numpad5;
			LightAttackKey = Key_P;
			MediumAttackKey = Key_I;
			HeavyAttackKey = Key_O;
			Special1Key = Key_0;
			Special2Key = Key_9;
			Special3Key = Key_8;
			Special4Key = Key_7;
			Special5Key = Key_6;
		}
	}
} // namespace gen