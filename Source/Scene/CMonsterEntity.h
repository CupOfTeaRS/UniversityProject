#pragma once

#include "AnimationManager.h"
#include "RenderMethod.h"
namespace gen {


	class CMonsterEntity 
	{
		CMonsterEntity();
		// Destructor - base class destructors should always be virtual
	~CMonsterEntity();
		

	
	bool Update();
	bool RenderAnim();
		 ID3D10ShaderResourceView* MeshAnimFrame = NULL;
		 CMatrix4x4 MeshMatrix ; 
		 AnimationSequence MonsterAnimsRight[LastMonsterAnims];
		 AnimationSequence MonsterAnimsLeft[LastMonsterAnims];
		 // Relative and absolute world matrices for each node in the template's mesh
	private: 
		 bool isFacingRight = false;
	};
}


