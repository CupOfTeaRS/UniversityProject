/*******************************************
AnimationManager.h

Responsible for animations
********************************************/

#pragma once

#include <map>
#include <string>
#include <vector>

using namespace std;

#include "CHashTable.h"
//new types to help organize the data
typedef pair<int, string> AnimPair;
typedef vector<AnimPair> AnimationSequence;
typedef pair<int, vector<int>> HitFrames;
typedef vector<HitFrames> HitDetectorInfo;
namespace gen
{
	

	//Whole selection of player animation types, each one has around 50 different sequences

	enum PlayerAnimationType {
		  Idle,
		  Walking,
		  Turning,
		  Jump,
		  Block,
		  Crouch,
		  Crouch_Turn,
		  Crouch_Block,
		  Block_Air,
		  Stand_Up,
		  Dash,
		  Dash_Back,
		  Summon,
		  Summon_Air,
		  Light_Leg_Att,
		  Light_Crouch_Att,
		  Medium_Att,
		  Medium_Walk_Att,
		  Medium_Crouch_Att,
		  Medium_Air_Att,
		  Heavy_Att,
		  Heavy_Walk_Att,
		  Heavy_Crouch_Att,
		  Heavy_Crouch_Fr_Att,
		  Heavy_Air_Att,
		  Throw,
		  Stando_OraOraOra,
		  Stando_Sp_2,
		  Stando_Sp_3,
		  Stando_Sp_4,
		  Stando_Ult_1,
		  Stando_Idle,
		  Special_OraOraOra,
		  Special_Num_2,
		  Special_Num_3,
		  Special_Num_4,
		  Ult_Num_1,	  
		  Is_Hit_Light,
		  Is_Hit_Light_2,
		  Is_Hit_Medium,
		  Is_Hit_Medium_2,
		  Is_Hit_Hard,
		  Is_Hit_Hard_2,
		  Is_Hit_Crouch,
		  Is_Hit_Crouch_2,
		  Is_Hit_Air,
		  Is_Killed,
		  Victory,
		  Victory_2,
		  Intro,
		  Intro_2,
		  PlayerAnimLast,

	};
	enum MonsterAnimationTypes
	{
		Walk,
		LastMonsterAnims,
	};
	enum MonsterTypes
	{
		Zombie,
		LastMonsterTypes,
	};
	enum PlayerUIAnimationTypes
	{
		FaceAnim,
		HealthBarAnim,
		Ult_Meter_Anim,
		Ult_Meter_Ready_Anim,
		LastPlayerUIAnims,
	};
	enum MenuUIAnimationTypes
	{
		IntroAnim,
		ModeSelectAnim,
		JotaroSelectAnim,
		DioSelectAnim,
		LastMenuUIAnims,
	};
			static const TInt32 PlayerAnimationTypes = PlayerAnimLast;
			static const TInt32 MenuUIAnimationTypeCount = LastMenuUIAnims;
			static const TInt32 MonsterTypeCount = LastMonsterTypes;
			static const TInt32 MonsterAnimTypeCount = LastMonsterAnims;
			static const TInt32 AttackAnimsPlayerStart = Light_Leg_Att;
			static const TInt32 AttackAnimsPlayerFinish = Stando_Ult_1;
			static const TInt32 PlayerUIAnimsCount = LastPlayerUIAnims;

	class CAnimationManager
	{
		/////////////////////////////////////
		//	Constructors/Destructors
	



	public:

		// Constructor
		CAnimationManager();

		// Destructor
		~CAnimationManager();

		//Parser function
		void PushAnimation(AnimationSequence sequence, bool isPlayerJotaro, bool isFacingRight, PlayerAnimationType type);
		void PushMonsterAnimation(AnimationSequence sequence, int monsterType, int monsterAnim, bool isFacingRight);
		void PushUIAnimation(AnimationSequence sequence, bool isPlayerJotaro, bool isFacingRight, PlayerUIAnimationTypes type);
		void PushMenuUIAnimation(AnimationSequence sequence, MenuUIAnimationTypes type);
		AnimationSequence GetAnimSequence(int type, bool isPlayerJotaro, bool isFacingRight);
		AnimationSequence GetMonsterAnimSequence(int monstertype, int animtype, bool isFacingRight);
		string GetMenuUIAnimFrame(int frameNum, int animType);
	private:


		// Prevent use of copy constructor and assignment operator (private and not defined)
		CAnimationManager(const CAnimationManager&);
		CAnimationManager& operator=(const CAnimationManager&);

		string BlankTextureName;

		AnimationSequence JotaroAnims_Right[PlayerAnimationTypes];
		AnimationSequence JotaroAnims_Left[PlayerAnimationTypes];

		AnimationSequence DioAnimsRight[PlayerAnimationTypes];
		AnimationSequence DioAnimsLeft[PlayerAnimationTypes];

		AnimationSequence MonsterAnimationsRight[MonsterTypeCount][MonsterAnimTypeCount];
		AnimationSequence MonsterAnimationsLeft[MonsterTypeCount][MonsterAnimTypeCount];

		AnimationSequence JotaroUIAnims_Right[PlayerUIAnimsCount];
		AnimationSequence JotaroUIAnims_Left[PlayerUIAnimsCount];

		AnimationSequence DioUIAnims_Right[PlayerUIAnimsCount];
		AnimationSequence DioUIAnims_Left[PlayerUIAnimsCount];

		AnimationSequence MenuAnims[MenuUIAnimationTypeCount];
		/////////////////////////////////////
		//	Public interface
	public:

		/////////////////////////////////////
		// Template creation / destruction

		string GetBlankTexturePath()
		{
			return BlankTextureName;
		}
		string GetPlayerUITexturePath(int type, int position, bool isPlayerJotaro, bool isFacingRight);
		





	
	
	};

} // namespace gen