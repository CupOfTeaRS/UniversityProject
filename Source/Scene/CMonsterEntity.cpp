#include "CMonsterEntity.h"
#include "Entity.h"
using namespace std;


namespace gen {
	extern ID3D10Device* g_pd3dDevice;
	extern CAnimationManager AnimationManager;
	extern const string MediaFolder;

 CMonsterEntity::CMonsterEntity()
	{


		for (int i = 0; i < MonsterAnimTypeCount; i++)
		{
			MonsterAnimsRight[i].swap(AnimationManager.GetMonsterAnimSequence(Zombie,i,true));
			MonsterAnimsLeft[i].swap(AnimationManager.GetMonsterAnimSequence(Zombie, i, false));
		}
		

	}

	
	bool CMonsterEntity::Update()
	{
		if (KeyHit(Key_L))
		{
			isFacingRight = !isFacingRight;
		}
		RenderAnim();
		return true;
	}
	bool CMonsterEntity::RenderAnim()
	{
		string fullFileName;
		if (isFacingRight)
		{
			fullFileName = MediaFolder + MonsterAnimsRight[Walk][10].second;
		}
		else if (!isFacingRight)
		{
			fullFileName = MediaFolder + MonsterAnimsLeft[Walk][10].second;
		}
		if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &MeshAnimFrame, NULL)))
		{
			string errorMsg = "Error loading texture " + fullFileName;
			SystemMessageBox(errorMsg.c_str(), "Mesh Error");
			return false;
		}
		return true;
	}
}

