/*******************************************
AnimationManager.cpp

Responsible for animation creation and
destruction

This animation manager has sole purpose - load the enormous amounts of strings from Parser into specific vector structures.
Player strings into PlayerEntity, MenuItems into Interface, etc.
********************************************/

#include "AnimationManager.h"
#include "RenderMethod.h"
#include "Mesh.h"
#include "MathDX.h"
#include <algorithm>


namespace gen
{
	extern ID3D10Device* g_pd3dDevice;
	extern const  string MediaFolder;
	/////////////////////////////////////
	// Constructors/Destructors

	// Constructor reserves space for entities and UID hash map, also sets first UID
	CAnimationManager::CAnimationManager()
	{
		JotaroAnims_Right->reserve(1024);
		JotaroAnims_Left->reserve(1024);

		DioAnimsLeft->reserve(512);
		DioAnimsRight->reserve(512);

		BlankTextureName = MediaFolder + "blank.png";
		for (int i = 0; i < LastMonsterTypes; i++)
		{
			MonsterAnimationsRight[i]->reserve(128);
			MonsterAnimationsLeft[i]->reserve(128);
		}

	}

	CAnimationManager::~CAnimationManager()
	{

	}
	//Setter function(Parser->Manager)
	//Manager serves as a transition operator, which takes and loads, nothing else. Also capable of returning a string of choice.
	void CAnimationManager::PushAnimation(AnimationSequence sequence, bool isPlayerJotaro, bool isFacingRight, PlayerAnimationType type)
	{
		if (isPlayerJotaro && isFacingRight)
		{
			JotaroAnims_Right[type] = sequence;
			return;
		}
		else if (isPlayerJotaro && !isFacingRight)
		{
			JotaroAnims_Left[type] = sequence;
			return;
		}
		else if (!isPlayerJotaro && isFacingRight)
		{
			DioAnimsRight[type] = sequence;
			return;
		}
		else if (!isPlayerJotaro && !isFacingRight)
		{
			DioAnimsLeft[type] = sequence;
			return;
		}
	}

	void CAnimationManager::PushUIAnimation(AnimationSequence sequence, bool isPlayerJotaro, bool isFacingRight, PlayerUIAnimationTypes type)
	{
		if (isPlayerJotaro && isFacingRight)
		{
			JotaroUIAnims_Right[type] = sequence;
			return;
		}
		else if (isPlayerJotaro && !isFacingRight)
		{
			JotaroUIAnims_Left[type] = sequence;
			return;
		}
		else if (!isPlayerJotaro && isFacingRight)
		{
			DioUIAnims_Right[type] = sequence;
			return;
		}
		else if (!isPlayerJotaro && !isFacingRight)
		{
			DioUIAnims_Left[type] = sequence;
			return;
		}
	}
	//The animations have type, player affinity, orientation and the vector itself. When called it loads the chosen vector from parser to chosen vector in PlayerEntity.
	//Getter function (Manager->Entity)
	AnimationSequence CAnimationManager::GetAnimSequence(int type, bool isPlayerJotaro, bool isFacingRight)
	{
		if (isPlayerJotaro && isFacingRight)
		{
			return JotaroAnims_Right[type];
		}
		else if (isPlayerJotaro && !isFacingRight)
		{
			return JotaroAnims_Left[type];
		}
		else if (!isPlayerJotaro && isFacingRight)
		{
			return DioAnimsRight[type];
		}
		else if (!isPlayerJotaro && !isFacingRight)
		{
			return DioAnimsLeft[type];
		}
	}
	AnimationSequence CAnimationManager::GetMonsterAnimSequence(int type, int animtype, bool isFacingRight)
	{
		if (isFacingRight)
			return MonsterAnimationsRight[type][animtype];
		else
			return MonsterAnimationsLeft[type][animtype];
	}
	string CAnimationManager::GetMenuUIAnimFrame(int frameNum, int animtype)
	{
		return MenuAnims[animtype][frameNum].second;
	}
	void CAnimationManager::PushMonsterAnimation(AnimationSequence sequence, int monsterType, int monsterAnim, bool isFacingRight)
	 {
		
		if (isFacingRight)
		{
			MonsterAnimationsRight[monsterType][monsterAnim] = sequence;
			return;
		}
		else
			MonsterAnimationsLeft[monsterType][monsterAnim] = sequence;
		return;
     }
	void CAnimationManager::PushMenuUIAnimation(AnimationSequence sequence, MenuUIAnimationTypes type)
	{
		MenuAnims[type] = sequence;
	}
	string CAnimationManager::GetPlayerUITexturePath(int type, int position, bool isPlayerJotaro, bool isFacingRight)
	{
		if (isPlayerJotaro && isFacingRight)
		{
			return JotaroUIAnims_Right[type][position].second;
			
		}
		else if (isPlayerJotaro && !isFacingRight)
		{
			return JotaroUIAnims_Left[type][position].second;
		}
		else if (!isPlayerJotaro && isFacingRight)
		{
			return DioUIAnims_Right[type][position].second;
		}
		else if (!isPlayerJotaro && !isFacingRight)
		{
			return DioUIAnims_Left[type][position].second;
		}
	}
} // namespace gen


