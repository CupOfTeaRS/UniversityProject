/*******************************************
	Entity.cpp

	Entity class implementation
********************************************/

#include "Entity.h"
#include "AnimationManager.h"
#include "FMODManager.h"
#include "UIManager.h"
#include "Messenger.h"
#include "EntityManager.h"

namespace gen
{

	extern ID3D10Device* g_pd3dDevice;
	extern CAnimationManager AnimationManager;
	extern const string MediaFolder;
	extern CMessenger Messenger;
	extern FMODManager SoundManager;
	extern CCamera* MainCamera;
	extern UIManager InterfaceManager;
	extern CEntityManager EntityManager;

	/*-----------------------------------------------------------------------------------------
	-------------------------------------------------------------------------------------------
		Base Entity Class
	-------------------------------------------------------------------------------------------
	-----------------------------------------------------------------------------------------*/
	int nextOneIsFlipped = false;
	// Base entity constructor, needs pointer to common template data and UID, may also pass 
	// name, initial position, rotation and scaling. Set up positional matrices for the entity
	CEntity::CEntity
	(
		CEntityTemplate* entityTemplate,
		TEntityUID       UID,
		const string&    name /*=""*/,
		const CVector3&  position /*= CVector3::kOrigin*/,
		const CVector3&  rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
		const CVector3&  scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
	)
	{
		m_Template = entityTemplate;
		m_UID = UID;
		m_Name = name;
		monsterComponent = NULL;


		// Allocate space for matrices
		TUInt32 numNodes = m_Template->Mesh()->GetNumNodes();
		m_RelMatrices = new CMatrix4x4[numNodes];
		m_Matrices = new CMatrix4x4[numNodes];
		isTexFlippedHorizontal = false;
		// Set initial matrices from mesh defaults
		for (TUInt32 node = 0; node < numNodes; ++node)
		{
			m_RelMatrices[node] = m_Template->Mesh()->GetNode(node).positionMatrix;
		}

		// Override root matrix with constructor parameters
		m_RelMatrices[0] = CMatrix4x4(position, rotation, kZXY, scale);


		AssembleMonster();

		if (m_UID > 30)
		{
			int i = 0;
		}
	}

	void CEntity::PreRender()
	{
		Mesh = m_Template->Mesh();
		// Calculate absolute matrices from relative node matrices & node heirarchy
		m_Matrices[0] = m_RelMatrices[0];
		TUInt32 numNodes = Mesh->GetNumNodes();
		for (TUInt32 node = 1; node < numNodes; ++node)
		{
			m_Matrices[node] = m_RelMatrices[node] * m_Matrices[Mesh->GetNode(node).parent];
		}
		// Incorporate any bone<->mesh offsets (only relevant for skinning)
		// Don't need this step for this exercise

		// Render with absolute matrices
		Mesh->PreRender(m_Matrices);
		i_subMeshCount = Mesh->GetSubMeshCount();
		for (int i = 0; i < Mesh->GetSubMeshCount(); i++)
		{
			SMeshBucketRenderData& buf = Mesh->GetSubMesh(i);
			buf.matrix = m_Matrices;
			renderdata.push_back(buf);
			renderMethods.push_back(buf.material.renderMethod);
			submeshToRender.push_back(buf.material.renderMethod);
		}

		renderMethodSize = renderMethods.size();

	}
	// Render the model
	void CEntity::Render()
	{
		// Get pointer to mesh to simplify code
		CMesh* Mesh = m_Template->Mesh();

		// Calculate absolute matrices from relative node matrices & node heirarchy
		m_Matrices[0] = m_RelMatrices[0];
		TUInt32 numNodes = Mesh->GetNumNodes();
		for (TUInt32 node = 1; node < numNodes; ++node)
		{
			m_Matrices[node] = m_RelMatrices[node] * m_Matrices[Mesh->GetNode(node).parent];
		}
		// Incorporate any bone<->mesh offsets (only relevant for skinning)
		// Don't need this step for this exercise

		// Render with absolute matrices
		Mesh->Render(m_Matrices);
	}

	void CEntity::ShadowRender()
	{
		// Get pointer to mesh to simplify code
		CMesh* Mesh = m_Template->Mesh();

		// Calculate absolute matrices from relative node matrices & node heirarchy
		m_Matrices[0] = m_RelMatrices[0];
		TUInt32 numNodes = Mesh->GetNumNodes();
		for (TUInt32 node = 1; node < numNodes; ++node)
		{
			m_Matrices[node] = m_RelMatrices[node] * m_Matrices[Mesh->GetNode(node).parent];
		}
		// Incorporate any bone<->mesh offsets (only relevant for skinning)
		// Don't need this step for this exercise

		// Render with absolute matrices
		if (m_Name != "Floor" || m_Name != "Sun")
			Mesh->ShadowMapRender(m_Matrices);
	}

	void CEntity::BucketRender(ERenderMethod method)
	{
		// Get pointer to mesh to simplify code

		// Calculate absolute matrices from relative node matrices & node heirarchy
		m_Matrices[0] = m_RelMatrices[0];
		TUInt32 numNodes = Mesh->GetNumNodes();

		for (TUInt32 node = 1; node < numNodes; ++node)
		{
			m_Matrices[node] = m_RelMatrices[node] * m_Matrices[Mesh->GetNode(node).parent];
		}



		// Render with material buckets
		//As we have pre-rendered all the entities into buckets, the pipeline is less encumbered by switching techniques and gives 5-10% more FPS than usual. May seem clumsy, because it is.
		for (TUInt32 i = 0; i < Mesh->GetSubMeshCount(); i++)
		{

			if (Mesh->HasMaterialChosen(method, i))
				Mesh->BucketRender(m_Matrices, method, i);
		}

	}
	//Animate function for the monsters, which is not used now
	bool CEntity::Animate(TFloat32 updateTime)
	{

		string fullFileName;
		if (isFacingRight)
		{
			fullFileName = MediaFolder + MonsterAnimsRight[thisMonsterType][currentAnim].second;
		}
		else if (!isFacingRight)
		{
			fullFileName = MediaFolder + MonsterAnimsLeft[thisMonsterType][currentAnim].second;

		}
		
		if (animChangeTimer >= animCycleDelay)
		{
			animChangeTimer = 0.0f;
			Mesh->m_Materials->textures[0]->Release();
			if (FAILED(D3DX10CreateShaderResourceViewFromFile(g_pd3dDevice, fullFileName.c_str(), NULL, NULL, &Mesh->m_Materials->textures[0], NULL)))
			{
				string errorMsg = "Error loading texture " + fullFileName;
				SystemMessageBox(errorMsg.c_str(), "Mesh Error");
				return false;
			}
			currentAnim++;
		}
		if (currentAnim >= MonsterAnimsRight[Zombie].size())
		{
			currentAnim = 0;
		}
		dmgImmunityTimer -= updateTime * AnimMultSlow;
		animChangeTimer += updateTime * AnimMultNormal;
		return true;


	}
	//Monster assembler, which gives it stats depending on type, is not used.
	void CEntity::AssembleMonster()
	{
		for (int i = 0; i < EntityManager.LevelMonsters.size(); i++)
		{
			if (EntityManager.LevelMonsters[i] == m_Name)
			{
				isAMonster = true;
				if (m_Name.find("Zombie") != std::string::npos)
				{
					for (int i = 0; i < MonsterAnimTypeCount; i++)
					{
						MonsterAnimsRight[i] = AnimationManager.GetMonsterAnimSequence(Zombie, i, true);
						MonsterAnimsLeft[i] = AnimationManager.GetMonsterAnimSequence(Zombie, i, false);
						thisMonsterType = Zombie;
						distFromCenter = 25.0f;
						monsterStats.hp_max = 50;
						monsterStats.hp = monsterStats.hp_max;
						monsterStats.hp_toShow = monsterStats.hp_max;
						monsterStats.movespeed = 10;
					}
				}
			}
		}
	}
	bool CEntity::Update(TFloat32 updateTime)
	{
		if (isDeleted)
		{
			return true;
		}
		//Everything that is not a player or a tree, is a house, since we care only for the name string
		//Clutter falling on the background is moved here
		if (m_Name.find("House") != std::string::npos)
		{
			
				FloatingCounter += updateTime * 100;
			//We do not move clutter when Dio stops time
				if(!EntityManager.zaWarudoEnabled)
			this->Matrix().MoveY(sin(FloatingCounter));
			if (this->Matrix().GetY() < -500)
			{
				this->Matrix().SetY(300 + Random(0, 1000));
			}
		}
		return true;
	}
	//It renders hp bar above any enemy, not used
	bool CEntity::RenderEntityUI(TFloat32 updateTime)
	{
		int x, y;
		stringstream outText;
		if (monsterStats.hp_toShow > monsterStats.hp)
		{
			monsterStats.hp_toShow -= updateTime * AnimMultNormal;
		}
		for (int i = 0; i < (int)monsterStats.hp_toShow; i++)
		{
			outText << "||||";
		}
		float r = (float)(monsterStats.hp_max - monsterStats.hp_toShow) / monsterStats.hp_max;
		float g = 1.0f - r;
	    MainCamera->PixelFromWorldPt(Matrix().Position(),ViewportWidth,ViewportHeight,&x,&y);
		InterfaceManager.RenderText(outText.str(), x,y - 150 ,r ,g, 0.3f,OSDFontMedium,true);

		return true;
    }  
	
	
}// namespace gen
