/*******************************************
	EntityManager.cpp

	Responsible for entity creation and
	destruction
********************************************/

#include "EntityManager.h"
#include "RenderMethod.h"
#include "Mesh.h"
#include "MathDX.h"
#include <algorithm>
#include "FMODManager.h"
#include "UIManager.h"
namespace gen
{
	extern ID3D10Device* g_pd3dDevice;
	extern bool isGameMode1VS1;
	extern CCamera* MainCamera;
	extern CMessenger Messenger;
	extern FMODManager SoundManager;
	extern UIManager InterfaceManager;
/////////////////////////////////////
// Constructors/Destructors

// Constructor reserves space for entities and UID hash map, also sets first UID
CEntityManager::CEntityManager()
{
	// Initialise list of entities and UID hash map
	m_Entities.reserve( 1024 );
	m_EntityUIDMap = new CHashTable<TEntityUID, TUInt32>( 2048, JOneAtATimeHash ); 

	// Set first entity UID that will be used
	m_NextUID = 0;


	m_IsEnumerating = false;
	MonsterTypeStrings[0] = "Zombie";
}

// Destructor removes all entities
CEntityManager::~CEntityManager()
{
	DestroyAllEntities();
}


/////////////////////////////////////
// Template creation / destruction

// Create a base entity template with the given type, name and mesh. Returns the new entity
// template pointer
CEntityTemplate* CEntityManager::CreateTemplate
(
	const string& type,
	const string& name,
	const string& mesh
)
{
	// Create new entity template
	CEntityTemplate* newTemplate = new CEntityTemplate( type, name, mesh );

	// Add the template name / template pointer pair to the map
    m_Templates[name] = newTemplate;

	return newTemplate;
}

// Destroy the given template (name) - returns true if the template existed and was destroyed
bool CEntityManager::DestroyTemplate( const string& name )
{
	// Find the template name in the template map
	TTemplateIter entityTemplate = m_Templates.find( name );
	if (entityTemplate == m_Templates.end())
	{
		// Not found
		return false;
	}

	// Delete the template and remove the map entry
	delete entityTemplate->second;
	m_Templates.erase( entityTemplate );
	return true;
}

// Destroy all templates held by the manager
void CEntityManager::DestroyAllTemplates()
{
	while (m_Templates.size())
	{
		TTemplateIter entityTemplate = m_Templates.begin();
		while (entityTemplate != m_Templates.end())
		{
			delete entityTemplate->second;
			++entityTemplate;
		};
		m_Templates.clear();
	}
}


/////////////////////////////////////
// Entity creation / destruction

// Create a base class entity - requires a template name, may supply entity name and position
// Returns the UID of the new entity
TEntityUID CEntityManager::CreateEntity
(
	const string&    templateName,
	const string&    name /*= ""*/,
	const CVector3&  position /*= CVector3::kOrigin*/, 
	const CVector3&  rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
	const CVector3&  scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
)
{
	// Get template associated with the template name
	CEntityTemplate* entityTemplate = GetTemplate( templateName );
	if (templateName == "Zombie")
	{
		LevelMonsters.push_back(name);
	}
	// Create new entity with next UID
	CEntity* newEntity = new CEntity( entityTemplate, m_NextUID, name, position, rotation, scale );

	// Get vector index for new entity and add it to vector
	TUInt32 entityIndex = static_cast<TUInt32>(m_Entities.size());
	m_Entities.push_back( newEntity );

	// Add mapping from UID to entity index into hash map
	m_EntityUIDMap->SetKeyValue( m_NextUID, entityIndex );
	
	m_IsEnumerating = false; // Cancel any entity enumeration (entity list has changed)

	// Return UID of new entity then increase it ready for next entity
	return m_NextUID++;
}

// Create a planet, requires a planet template name, may supply entity name and position
// Returns the UID of the new entity

TEntityUID CEntityManager::CreatePlayer
(
	const string&   templateName,
	const string&   name /*= ""*/,
	const CVector3& position /*= CVector3::kOrigin*/,
	const CVector3& rotation /*= CVector3( 0.0f, 0.0f, 0.0f )*/,
	const CVector3& scale /*= CVector3( 1.0f, 1.0f, 1.0f )*/
)
{
	// Get planet template associated with the template name
	CEntityTemplate* playerTemplate = GetTemplate(templateName);

	// Create new planet entity with next UID
	CPlayerEntity* newEntity =
		new CPlayerEntity(playerTemplate, m_NextUID, name, position, rotation, scale);

	// Get vector index for new entity and add it to vector
	TUInt32 entityIndex = static_cast<int>(m_Entities.size());
	m_Entities.push_back(newEntity);

	// Add mapping from UID to entity index into hash map
	m_EntityUIDMap->SetKeyValue(999, entityIndex);

	m_IsEnumerating = false; // Cancel any entity enumeration (entity list has changed)

							 // Return UID of new entity then increase it ready for next entity
	return m_NextUID++;
}
// Destroy the given entity - returns true if the entity existed and was destroyed
bool CEntityManager::DestroyEntity( TEntityUID UID )
{
	// Find the vector index of the given UID
	TUInt32 entityIndex;
	if (!m_EntityUIDMap->LookUpKey( UID, &entityIndex ))
	{
		// Quit if not found
		return false;
	}

	// Delete the given entity and remove from UID map
	delete m_Entities[entityIndex];
	m_EntityUIDMap->RemoveKey( UID );

	// If not removing last entity...
	if (entityIndex != m_Entities.size() - 1)
	{
		// ...put the last entity into the empty entity slot and update UID map
		m_Entities[entityIndex] = m_Entities.back();
		m_EntityUIDMap->SetKeyValue( m_Entities.back()->GetUID(), entityIndex );
	}
	m_Entities.pop_back(); // Remove last entity

	m_IsEnumerating = false; // Cancel any entity enumeration (entity list has changed)
	return true;
}


// Destroy all entities held by the manager
void CEntityManager::DestroyAllEntities()
{
	m_EntityUIDMap->RemoveAllKeys();
	while (m_Entities.size())
	{
		delete m_Entities.back();
		m_Entities.pop_back();
	}

	m_IsEnumerating = false; // Cancel any entity enumeration (entity list has changed)
}


/////////////////////////////////////
// Update / Rendering

// Call all entity update functions. Pass the time since last update
void CEntityManager::UpdateAllEntities(float updateTime)
{
	TUInt32 entity = 0;
	for (int i = 0; i < m_Entities.size(); i++)
	{
		// Update entity, if it returns false, then destroy it
		if (!m_Entities[entity]->Update(updateTime))
		{
			DestroyEntity(m_Entities[entity]->GetUID());
		}
		else
		{
			++entity;
		}
	}
	if (!DoubleUltCollisionSoundPlayed && DoubleUltCollisionEvent)
	{
		SoundManager.ClearAllSounds();
		SoundManager.PlayDoubleUltEvent();
		DoubleUltCollisionSoundPlayed = true;
	}
	if (DoubleUltCollisionEvent || DoubleUltCollisionTimer > 1.0f)
	{
		f_DoubleUltCollisionEvent(updateTime);
	}
	
	UpdateParticles(updateTime);
	
}
// Pre render all entities

void CEntityManager::PreRenderAllEntities()
{
	i_Vectorsize = m_Entities.size();
	i_numRenderMethod = GetNumRenderMethods();

	bool hasMethod[NumRenderMethods];

	for (int i = 0; i < NumRenderMethods; i++)
	{
		hasMethod[i] = false;
	}

	for (TUInt32 i = 0; i < i_Vectorsize; ++i)
	{
		m_Entities[i]->PreRender();
		

		for (int j = 0; j < m_Entities[i]->getSubmeshCount(); j++)
		{
			SMeshBucketRenderData renderdata = m_Entities[i]->getSubmeshData(j);
			SSubmesh submesh;

			submesh.bucketRenderData = renderdata;
			submesh.UID = m_Entities[i]->GetUID();
		    
			hasMethod[submesh.bucketRenderData.material.renderMethod] = true;
				}

		for (int j = 0; j < NumRenderMethods; j++)
		{
			if (hasMethod[j] == true)
			{
				m_EntityBuckets[j].push_back(m_Entities[i]);
			}
		}
		// for shadow stuff

		if (m_Entities[i]->GetName() == "ShadowDropPoint1")
		{
			m_Entities[i]->Matrix().FaceTarget(CVector3(0, 0, 0));
		}
		else if (m_Entities[i]->GetUID() == 1 || m_Entities[i]->GetUID() == 4)
		{
			m_Entities[i]->doNotTouch = true;
		}
	}
	


	
}
// Render all entities
void CEntityManager::RenderAllEntities()
{
	/*TEntityIter entity = m_Entities.begin();
	while (entity != m_Entities.end())
	{
		(*entity)->Render();
		++entity;
	}*/
	for (int i = 0; i < m_Entities.size(); i++)
	{
		m_Entities[i]->Render();
	}
}

void CEntityManager::ShadowRenderAllEntities()
{
	
	bool render = false;
	for (int i = 0; i < m_Entities.size(); i++)
	{
		for (int j = 0; j < m_Entities[i]->renderMethods.size(); j++)
		{
			if (m_Entities[i]->renderMethods[j] != AlphaBlend)
			{
				render = true;
			}
		}
		if (render)
		{
			m_Entities[i]->ShadowRender();
		}
		render = false;
	}

	
}
void CEntityManager::BucketRenderAllEntities()
{
	D3DXVECTOR3 cameraFacing = MainCamera->GetFacing();
	
	/*for (int i = 0; i < materialCount; i++)
	{
		BucketRenderByMaterial(i);
	}*/

	for (int i = 0; i < NumRenderMethods; i++)
	{
		
		ERenderMethod method;
		switch (i)
		{
		case PlainColour:
			method = PlainColour;
			
			break;
		case PlainTexture:
			method = PlainTexture;
			break;
		case PixelLit:
			method = PixelLit;
			break;
		case PixelLitTex:
			method = PixelLitTex;
			break;
		case NormalMap:
			method = NormalMap;
			break;
		case ParallaxMap:
			method = ParallaxMap;
			break;
		case AlphaBlend:
			method = AlphaBlend;
			for (int j = 0; j  < m_EntityBuckets[i].size(); j++)
			{
				//Do not Touch is created because of a PlayerEntity not having Mesh and Render info of it`s own, which causes errors
				//So we skip them
				if (m_EntityBuckets[i][j]->doNotTouch)
				{
					continue;
			    }
					CVector3 distToCamera = m_EntityBuckets[i][j]->Matrix().GetPosition() - MainCamera->Matrix().GetPosition();
					m_EntityBuckets[i][j]->depthFromCamera = distToCamera.Length();
			}
			break;
		case Atmosphere:
			method = Atmosphere;
			break;
		case GodRays:
			method = GodRays;
			break;
		case Fog:
			method = Fog;
			break;
		default:
			method = PixelLit;
			break;
		}

		
		currentTechnique = GetRenderMethodTechnique(method);
		if (currentTechnique != nullptr)
		{
			currentTechnique->GetDesc(&currentTechDesc);
		}
		QuicksortEntitiesByDepth(AlphaBlend);
		for (int j = 0; j < m_EntityBuckets[i].size(); j++)
		{
			m_EntityBuckets[i][j]->BucketRender(method);
		}

		
	}
	
}
void CEntityManager::BucketRenderByMaterial(int material)
{
	ERenderMethod method;
	switch (material)
	{
	case PlainColour: 
		method = PlainColour;
		break;
	case PlainTexture:
		method = PlainTexture;
		break;
	case PixelLit:
		method = PixelLit;
		break;
	case PixelLitTex:
		method = PixelLitTex;
		break;
	case NormalMap:
		method = NormalMap;
		break;
	case ParallaxMap:
		method = ParallaxMap;
		break;
	case AlphaBlend:
		method = AlphaBlend;

		break;
	case DepthOnly:
		method = DepthOnly;
		break;
	default:
		method = PixelLitTex;
		break;
	}

	for (int i = 0; i < m_Entities.size(); i++)
	{
		
		m_Entities[i]->BucketRender(method);
	}
	
	
}

//The quicksort function, that determines which sprites render last and which first
void CEntityManager::QuicksortEntitiesByDepth(int method)
{
	int counter = 0;
	int which = 0;
	float max = 0.0;
	do {
		for (int i = 0; i < m_EntityBuckets[method].size() - counter ; i++)
		{
			if (m_EntityBuckets[method][i]->depthFromCamera > max)
			{
				max = m_EntityBuckets[method][i]->depthFromCamera;
				which = i;
			}
		}
		m_EntityBuckets[method].push_back(m_EntityBuckets[method][which]);
		m_EntityBuckets[method].erase(m_EntityBuckets[method].begin() + which);
		counter++;
		max = 0;
	} while (counter != m_EntityBuckets[method].size());
}
void CEntityManager::CollisionCalculator()
{
	//Collision calculator for player vs monsters, currently not used
	for (int i = 0; i < m_Entities.size(); i++)
	{
		if (m_Entities[i]->isAMonster)
		{
			m_Entities[i]->isCollidingWithPlayer = false;
			m_Entities[i]->isCollidingWithPlayerToDamage = false;
			CVector3 playerpos = GetEntity("Player")->Matrix().Position();
			CVector3 monsterpos = m_Entities[i]->Matrix().Position();
			float adjustor = 0;
			if (RoddaRolla)
			{
				adjustor = 20.0f;
			}
			if (Distance(playerpos,monsterpos ) < m_Entities[i]->distFromCenter + adjustor)
			{
				m_Entities[i]->isCollidingWithPlayer = true;
			}
			if (Distance(playerpos, monsterpos) < m_Entities[i]->distFromCenter / 4)
			{
				m_Entities[i]->isCollidingWithPlayerToDamage = true;
			}
				CVector3 standopos = GetEntity("Stando")->Matrix().Position();
				if (Distance(standopos, monsterpos) < m_Entities[i]->distFromCenter)
				{
					m_Entities[i]->isCollidingWithPlayer = true;
				}
		}
	}
}
void CEntityManager::UpdateParticles(TFloat32 updatetime)
{
	if (isGameMode1VS1)
	{
		if (abs(Distance(player1Pos, player2Pos)) < 30.0f || abs(Distance(standoPlayer1Pos, player2Pos)) < 50.0f)
		{
			player1CanHitplayer2 = true;
		}
		else
		{
			player1CanHitplayer2 = false;
		}

		if (abs(Distance(player2Pos, player1Pos)) < 30.0f || abs(Distance(standoPlayer2Pos, player1Pos)) < 50.0f)
		{
			player2CanHitplayer1 = true;
		}
		else
		{
			player2CanHitplayer1 = false;
		}

		if (KeyHeld(Key_Q))
		{
			int i = 0;
		}
		if (knivesCreated && !zaWarudoEnabled)
		{
			if (knivesFlyRight)
			{
				GetEntity("KnivesRight")->Matrix().MoveX(5.0f);
			}
			else
				GetEntity("KnivesLeft")->Matrix().MoveX(-5.0f);

			if (knivesOwnerPlayer1)
			{
				if (abs(Distance(GetEntity("KnivesLeft")->Matrix().Position(), player2Pos)) < 30.0f || abs(Distance(GetEntity("KnivesRight")->Matrix().Position(), player2Pos)) < 30.0f)
				{
					SMessage msg;
					msg.dmg = 65;
					msg.from = SystemUID;
					msg.type = Msg_Dmg;
					msg.knockbackVel = 0.1;
					msg.knockUpVel  = 0.1;
					Messenger.SendMessage(Player2UID, msg);
					GetEntity("KnivesLeft")->Matrix().SetPosition(CVector3(-10000, -1000, -1000));
					GetEntity("KnivesRight")->Matrix().SetPosition(CVector3(-10000, -1000, -1000));
					knivesCreated = false;
					SoundManager.PlayPlayerSound(BloodSplatterSound, false, false);
				}
			}
			else
			{
				if (abs(Distance(GetEntity("KnivesLeft")->Matrix().Position(), player1Pos)) < 30.0f || abs(Distance(GetEntity("KnivesRight")->Matrix().Position(), player1Pos)) < 30.0f)
				{
					SMessage msg;
					msg.dmg = 65;
					msg.from = SystemUID;
					msg.type = Msg_Dmg;
					msg.knockbackVel = 0.1;
					msg.knockUpVel = 0.1;
					Messenger.SendMessage(PlayerUID, msg);
					GetEntity("KnivesLeft")->Matrix().SetPosition(CVector3(-10000, -1000, -1000));
					GetEntity("KnivesRight")->Matrix().SetPosition(CVector3(-10000, -1000, -1000));
					knivesCreated = false;
					SoundManager.PlayPlayerSound(BloodSplatterSound, false, true);
				}
			}


		}


	}
}
void CEntityManager::f_DoubleUltCollisionEvent(float updateTime)
{
	DoubleUltCollisionTimer += updateTime;
	if (KeyHit(Key_B))
	{
		player1ButtonPressCounter++;
	}
	if (KeyHit(Key_I))
	{
		player2ButtonPressCounter++;
	}


	if (DoubleUltCollisionTimer > 0.1f && DoubleUltCollisionTimer < 1.2f)
	{
		if (!isPlayer2Jotaro)
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x, player2Pos.y + 5, player2Pos.z - 55));
		else
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x, player1Pos.y + 5, player2Pos.z - 55));
	}
	else if (DoubleUltCollisionTimer > 1.6f && DoubleUltCollisionTimer < 2.5f)
	{
		if (isPlayer2Jotaro)
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x, player2Pos.y + 5, player2Pos.z - 55));
		else
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x, player1Pos.y + 5, player2Pos.z - 55));
	}
	else if (DoubleUltCollisionTimer > 3.3f && DoubleUltCollisionTimer < 7.0f)
	{
		if (!isPlayer2Jotaro)
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x - 10, player2Pos.y + 5, player2Pos.z - 85));
		else
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x - 10, player1Pos.y + 5, player2Pos.z - 85));
	}
	else if (DoubleUltCollisionTimer > 9.5f && DoubleUltCollisionTimer < 17.0f)
	{
		if (isPlayer2Jotaro)
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x, player2Pos.y + 5, player2Pos.z - 55));
		else
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x, player1Pos.y + 5, player2Pos.z - 55));
	}
	if (DoubleUltCollisionTimer > 9.50f)
	{
		DoubleUltCollisionEventFinish = true;
	}
	if (DoubleUltCollisionEventFinish)
	{
		SMessage msg;
		SMessage msgLoss;
		msg.type = Msg_Victory;
		msgLoss.type = Msg_Dmg;
		msgLoss.dmg = 999;
		if (player1ButtonPressCounter > player2ButtonPressCounter)
		{
			Messenger.SendMessage(PlayerUID, msg);
			Messenger.SendMessage(Player2UID, msgLoss);
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x, 50, -150));
		}
		else
		{
			Messenger.SendMessage(Player2UID, msg);
			Messenger.SendMessage(PlayerUID, msgLoss);
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x, 50, -150));
		}
		DoubleUltCollisionEvent = false;
		
	}
	/*
	else if (DoubleUltCollisionTimer > 3.0f && DoubleUltCollisionTimer < 4.0f)
	{
		if (isPlayer2Jotaro)
			MainCamera->Matrix().SetPosition(CVector3(player2Pos.x, player2Pos.y + 5, player2Pos.z - 35));
		else
			MainCamera->Matrix().SetPosition(CVector3(player1Pos.x, player1Pos.y + 5, player2Pos.z - 35));
	}*/
}
} // namespace gen


