/*******************************************
PlayerEntity.h

Player entity class	definition
********************************************/

#pragma once

#include <string>
using namespace std;

#include "Defines.h"
#include "Entity.h"
#include "AnimationManager.h"
#include "Messenger.h"
namespace gen
{

	/////////////////////////////////////////////////////////////////////////
	// No need for a specialised Player template class, only requires base
	// functionality (mesh management)
	/////////////////////////////////////////////////////////////////////////


	/*-----------------------------------------------------------------------------------------
	-------------------------------------------------------------------------------------------
	Player Entity Class
	-------------------------------------------------------------------------------------------
	-----------------------------------------------------------------------------------------*/

	// A Player inherits the ID/positioning/rendering support of the base class and adds
	// instance status (spin speed only). It also provides an update function to perform spin
	class CPlayerEntity : public CEntity
	{
		/////////////////////////////////////
		//	Constructors/Destructors
	public:
		// Player constructor intialises car-specific data and passes parameters to base class constructor
		CPlayerEntity
		(
			CEntityTemplate* PlayerTemplate,
			TEntityUID       UID,
			const string&    name = "PlayerSUB",
			const CVector3&  position = CVector3::kOrigin,
			const CVector3&  rotation = CVector3(0.0f, 0.0f, 0.0f),
			const CVector3&  scale = CVector3(1.0f, 1.0f, 1.0f)
		);

		// No destructor needed

	private:
		// Disallow use of copy constructor and assignment operator (private and not defined)
		CPlayerEntity(const CPlayerEntity&);
		CPlayerEntity& operator=(const CPlayerEntity&);

		virtual CMatrix4x4& Matrix(TUInt32 node = 0)
		{
			return m_RelMatrices[node];
		}

		/////////////////////////////////////
		//	Public interface
	public:

		/////////////////////////////////////
		// Getters / Setters

		
	
		

		/////////////////////////////////////
		// Update

		// Update the car - performs car message processing and behaviour
		// Return false if the entity is to be destroyed
		// Keep as a virtual function in case of further derivation
		virtual bool Update(TFloat32 updateTime);
		virtual bool Animate(TFloat32 updateTime, bool isFacingRight, int AnimationType);
		bool Animate_Stando(TFloat32 updateTime, bool isFacingRight, int AnimationType);
		bool Animate_StandoDio(TFloat32 updateTime, bool isFacingRight, int AnimationType);
		 bool RenderEntityUI(TFloat32 updateTime);
		/////////////////////////////////////
		//	Private interface
	private:

		//Controls

		EKeyCode JumpKey;
		EKeyCode CrouchKey;
		EKeyCode GoLeftKey;
		EKeyCode GoRightKey;
		EKeyCode Dash_FrontKey;
		EKeyCode Dash_BackKey;
		EKeyCode SummonKey;
		EKeyCode BlockKey;
		EKeyCode LightAttackKey;
		EKeyCode MediumAttackKey;
		EKeyCode HeavyAttackKey;
		EKeyCode Special1Key;
		EKeyCode Special2Key;
		EKeyCode Special3Key;
		EKeyCode Special4Key;
		EKeyCode Special5Key;
		/////////////////////////////////////
		// Data
		CEntityTemplate* m_Template;

		// Unique identifier and name for the entity
		TEntityUID  m_UID;
		string      m_Name;
		EntityStats playerStats;
		bool isPlayer1;
		TFloat32 m_UpwardVelocity = 0.0f;
		TFloat32 m_HorizontalMoveSpeed = 0.5f;
		int currentAnimSequence = 0;
		int prevAnimSequence = 0;
		AnimationSequence animsRight[PlayerAnimationTypes];
		AnimationSequence animsLeft[PlayerAnimationTypes];
		HitDetectorInfo hitFrames;
		TUInt32 playerMaxHp = 200;
		bool isAnimating = true;
		bool animationLock = false;
		bool buttonPressed = false;
		bool isInAir = false;
		bool isAttacking = false;
		bool isUlting = false;
		bool isBlocking = true;
		TUInt32 blockAvailable = 3;
		TFloat32 blockTimer = 0.0f;
		TFloat32 blockResetTimer = 0.0f;
		TUInt32 ultPointsAvailable;
		TUInt32 ultPointsMax = 4;
		TUInt32 ultPointsAccumulator = 0;
		TFloat32 standoEnergyDrain ;
		TFloat32 zaWarudoTimer = 0.0f;
		bool thisUnaffected = false;
		int thisDamageFrame;
		int thisDamageFrameStando;
		bool isBlockingTimer0 = false;
		bool moveShieldOnce = false;
		bool Enlarged = false;
		bool isDamaged = false;
		TFloat32 playerAttackAnimsMax;
		TFloat32 damagedTimer = 0.0f;
		TFloat32 damageTransparencyTimer = 0.0f;
		TFloat32 setHpTo = 0.0f;
		bool isTransparent = false;
		bool isDeadPlayer1 = false;
		bool isDeadPlayer2 = false;
		bool isVictorious = false;
		int victoryPoseCounter = 0;
		bool isStandoSummoned = false;
		bool isStandoIdle = true;
		int currentStandoAnimSequence = Stando_Idle;
		TUInt32  buttonPressCount = 0;
		TFloat32 buttonPressTimer = 0.0f;
		TUInt32 airAttackLimit = 1;
		TUInt32 airAttackCount = 0;
		bool DoubleUltPlayOnce = false;
		bool isKnockedUp = false;
		void SetupPlayer(bool isPlayer1,bool isPlayerJotaro);
		void setHitFrames(bool isPlayerJotaro);
		void f_playAnimSeq(int animType);
		void f_playAnimSeqDio(int animType);
		void f_Idle();
		void f_Turn(bool faceDirRight);
		void f_Block();
		void f_Crouch();
		void f_Crouch_Turn();
		void f_Dash(bool faceDirRight);
		void f_Dash_BW(bool faceDirRight);
		void f_Stand_Up();
		void f_Create_Stando();
		void f_Unsummon_Stando();
		void f_Rescale(bool Enlarge, CEntity* entity);
		void f_Rescalex2(bool Enlarge, CEntity* entity);
		void f_RescaleY(bool Enlarge, CEntity* entity, TFloat32 amount);
		void f_attackMoveDisplacer( float displacementX);
		void f_Stando_Displacement();
		void f_physGravity(TFloat32 updateTime);
		void SwitchPlayerHpState(SMessage msg);
		bool SwitchAnimStateJotaro(TFloat32 updateTime, bool isFacingRight, int animSequence, string fullFileName);
		bool SwitchAnimStateDio(TFloat32 updateTime, bool isFacingRight, int animSequence, string fullFileName);
		void SetupControls(bool isPlayer1);
		void UltPointsAccumulator(int dmg);
		// Relative and absolute world matrices for each node in the template's mesh
		CMatrix4x4* m_RelMatrices; // Dynamically allocated arrays
		CMatrix4x4* m_Matrices;
		// Entity status
		CEntity* player ;  // 
		CEntity* stando;
		bool isPlayerJotaro;
		
		float standoAnimChangeTimer = 0.0f;
		int currentAnim = 0;
		int currentStandoAnim = 0;
		
		float m_StandoFloating = 0.0f;
		float m_Stando_Ult_Displacement = 0.0f;
		bool faceDirectionRight = true;
		void Controls(TFloat32 updateTime);
		void MessageComponent();
		void HitMessageComposer();
		void DrainEnergy(TFloat32 updateTime);
		void DamageAccumulator(TFloat32 updateTime,bool isFacingRight);
	};
	

} // namespace gen
