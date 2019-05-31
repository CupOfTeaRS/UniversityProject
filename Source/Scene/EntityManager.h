/*******************************************
	EntityManager.h

	Responsible for entity creation and
	destruction
********************************************/

#pragma once

#include <map>
using namespace std;

#include "CHashTable.h"
#include "Entity.h"
#include "PlayerEntity.h"
#include "MeshData.h"
#include "RenderMethod.h"

namespace gen
{

// The entity manager is responsible for creation, update, rendering and deletion of
// entities. It also manages UIDs for entities using a hash table
class CEntityManager
{
/////////////////////////////////////
//	Constructors/Destructors
public:

	// Constructor
	CEntityManager();

	// Destructor
	~CEntityManager();

private:
	// Prevent use of copy constructor and assignment operator (private and not defined)
	CEntityManager( const CEntityManager& );
	CEntityManager& operator=( const CEntityManager& );


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	// Template creation / destruction

	// Create a base entity template with the given type, name and mesh. Returns the new entity
	// template pointer
	CEntityTemplate* CEntityManager::CreateTemplate
	(
		const string& type,
		const string& name,
		const string& mesh
	);

	// Note: Planets use the base template class, don't need a custom function

	// Destroy the given template (name) - returns true if the template existed and was destroyed
	bool DestroyTemplate( const string& name );

	// Destroy all templates held by the manager
	void DestroyAllTemplates();


	/////////////////////////////////////
	// Entity creation / destruction

	// Create a base class entity - requires a template name, may supply entity name and position
	// Returns the UID of the new entity
	TEntityUID CreateEntity
	(
		const string&    templateName,
		const string&    name = "",
		const CVector3&  position = CVector3::kOrigin, 
		const CVector3&  rotation = CVector3( 0.0f, 0.0f, 0.0f ),
		const CVector3&  scale = CVector3( 1.0f, 1.0f, 1.0f )
	);

	// Create a planet, requires a planet template name, may supply entity name and position
	// Returns the UID of the new entity
	TEntityUID CreatePlanet
	(
		const string&   templateName,
		const string&   name = "",
		TFloat32        spinSpeed = kfPi,
		const CVector3& position = CVector3::kOrigin, 
		const CVector3& rotation = CVector3( 0.0f, 0.0f, 0.0f ),
		const CVector3& scale = CVector3( 1.0f, 1.0f, 1.0f )
	);
	TEntityUID CreatePlayer
	(
		const string&   templateName,
		const string&   name = "",
		const CVector3& position = CVector3::kOrigin,
		const CVector3& rotation = CVector3(0.0f, 0.0f, 0.0f),
		const CVector3& scale = CVector3(1.0f, 1.0f, 1.0f)
	);
	//submesh struct
	struct SSubMeshDX
	{
		TUInt32                  node;     // Node controlling this sub-mesh 
		TUInt32                  material; // Index of material used by this sub-mesh

										   // Vertex data for the sub-mesh stored in a vertex buffer and the number of vertices in the buffer
		ID3D10Buffer*            vertexBuffer;
		TUInt32                  numVertices;

		// Description of the elements in a single vertex (position, normal, UVs etc.)
		static const int         MAX_VERTEX_ELTS = 64;
		D3D10_INPUT_ELEMENT_DESC vertexElts[MAX_VERTEX_ELTS];
		ID3D10InputLayout*       vertexLayout; // Layout of a vertex (derived from above array)
		unsigned int             vertexSize;   // Size of vertex calculated from contained elements

											   // Index data for the sub-mesh stored in a index buffer and the number of indices in the buffer
		ID3D10Buffer*            indexBuffer;
		TUInt32                  numIndices;
	};
	struct SSubmesh
	{
		SMeshBucketRenderData bucketRenderData;
		TEntityUID UID;
	};

	// Destroy the given entity - returns true if the entity existed and was destroyed
	bool DestroyEntity( TEntityUID UID );

	// Destroy all entities held by the manager
	void DestroyAllEntities();


	/////////////////////////////////////
	// Template / Entity access

	// Return the template with the given name
	CEntityTemplate* GetTemplate( const string& name )
	{
		// Find the template name in the template map
		TTemplateIter entityTemplate = m_Templates.find( name );
		if (entityTemplate == m_Templates.end())
		{
			// Template name not found
			return 0;
		}
		return (*entityTemplate).second;
	}


	// Return the number of entities
	TUInt32 NumEntities() 
	{
		return static_cast<TUInt32>(m_Entities.size());
	}

	
	// Return the entities at the given array index
	CEntity* GetEntityAtIndex( TUInt32 index )
	{
		return m_Entities[index];
	}

	// Return the entity with the given UID
	CEntity* GetEntity( TEntityUID UID )
	{
		// Find the entity UID in the entity hash map
		TUInt32 entityIndex;
		if (!m_EntityUIDMap->LookUpKey( UID, &entityIndex ))
		{
			return 0;
		}
		return m_Entities[entityIndex];
	}

	// Return the entity with the given name & optionally the given template name & type
	CEntity* GetEntity( const string& name, const string& templateName = "",
	                    const string& templateType = "" )
	{
		TEntityIter entity = m_Entities.begin();
		while (entity != m_Entities.end())
		{
			if ((*entity)->GetName() == name && 
				(templateName.length() == 0 || (*entity)->Template()->GetName() == templateName) &&
				(templateType.length() == 0 || (*entity)->Template()->GetType() == templateType))
			{
				return (*entity);
			}
			++entity;
		}
		return 0;
	}


	// Begin an enumeration of entities matching given name, template name and type
	// An empty string indicates to match anything in this field (would be nice to support
	// wildcards, e.g. match name of "Ship*")
	void BeginEnumEntities( const string& name, const string& templateName,
	                        const string& templateType = "" )
	{
		m_IsEnumerating = true;
		m_EnumEntity = m_Entities.begin();
		m_EnumName = name;
		m_EnumTemplateName = templateName;
		m_EnumTemplateType = templateType;
	}

	// Finish enumerating entities (see above)
	void EndEnumEntities()
	{
		m_IsEnumerating = false;
	}

	// Return next entity matching parameters passed to a previous call to BeginEnumEntities
	// Returns 0 if BeginEnumEntities not called or no more matching entities
	CEntity* EnumEntity()
	{
		if (!m_IsEnumerating)
		{
			return 0;
		}

		while (m_EnumEntity != m_Entities.end())
		{
			if ((m_EnumName.length() == 0 || (*m_EnumEntity)->GetName() == m_EnumName) && 
				(m_EnumTemplateName.length() == 0 ||
				 (*m_EnumEntity)->Template()->GetName() == m_EnumTemplateName) &&
				(m_EnumTemplateType.length() == 0 ||
				 (*m_EnumEntity)->Template()->GetType() == m_EnumTemplateType))
			{
				CEntity* foundEntity = *m_EnumEntity;
				++m_EnumEntity;
				return foundEntity;
			}
			++m_EnumEntity;
		}
		
		m_IsEnumerating = false;
		return 0;
	}

	//static bool compareSubmeshes( const CSubmesh* a,  const  CSubmesh* b);
	/////////////////////////////////////
	// Update / Rendering
	vector<string> LevelMonsters;
	string MonsterTypeStrings[MonsterTypeCount];
	// Call all entity update functions - not the ideal method, OK for this example
	// Pass the time since last update
	void UpdateAllEntities( float updateTime );
	void UpdateParticles(float updateTime);
	// Render all entities - not the ideal method, OK for this example
	void PreRenderAllEntities();
	void RenderAllEntities();
	void BucketRenderAllEntities();
	void ShadowRenderAllEntities();
	void BucketRenderByMaterial(int material);
	void f_DoubleUltCollisionEvent(float updateTime);
	bool RoddaRolla = false;
	bool player1IntroFinished = false;
	bool player2IntroFinished = false;
	bool isPlayer1Taken = false;
	bool isPlayer1Jotaro = true;
	bool isPlayer2Jotaro = false;
	bool isPlayer1Ulting = false;
	bool isPlayer2Ulting = false;
	bool doNotUpdatePlayer1 = false;
	bool doNotUpdatePlayer2 = false;
	bool DoubleUltCollisionEvent = false;
	bool DoubleUltCollisionEventFinish = false;
	bool DoubleUltCollisionSoundPlayed = false;
	bool isPaused = false;
	TFloat32 CountDownToStart = 0.0f;
	TFloat32 DoubleUltCollisionTimer = 0.0f;
	int player1ButtonPressCounter = 0;
	int player2ButtonPressCounter = 0;
	CVector3 player1Pos;
	CVector3 player2Pos;
	CVector3 standoPlayer1Pos;
	CVector3 standoPlayer2Pos;
	bool player1CanHitplayer2;
	bool player2CanHitplayer1;
	int player1LifeLeft = 3;
	int player2LifeLeft = 3;
	bool zaWarudoEnabled = false;
	bool knivesCreated = false;
	bool knivesFlyRight = true;
	bool knivesOwnerPlayer1 = false;

	ID3D10EffectTechnique* currentTechnique;
	D3D10_TECHNIQUE_DESC currentTechDesc;
/////////////////////////////////////
//	Private interface
private:

	/////////////////////////////////////
	// Types

	// Entity templates are held in a map, define some types for convenience
	typedef map<string, CEntityTemplate*> TTemplates;
	typedef TTemplates::iterator TTemplateIter;

	// Entity instances are held in a vector, define some types for convenience
	typedef vector<CEntity*> TEntities;

	//typedef vector<CSubmesh*> TSubmeshes;
	typedef TEntities::iterator TEntityIter;


	static const int materialCount = NumRenderMethods;
	/////////////////////////////////////
	// Template Data

	// The map of template names / templates
	TTemplates m_Templates;
	CMesh* Mesh;

	/////////////////////////////////////
	// Entity Data

	// The main list of entities. This vector is kept packed - i.e. with no gaps. If an
	// entity is removed from the middle of the list, the last entity is moved down to
	// fill its space
	public:
	TEntities m_Entities;
	private:
	TEntities m_EntityBuckets[NumRenderMethods];
	
	public:
		void CollisionCalculator();
	/*TSubmeshes m_Submeshes[NumRenderMethods];*/
	private:
	bool hasmethods[materialCount] = {false,false,false,false,false,false,false};
	void QuicksortEntitiesByDepth( int method);

	// A mapping from UIDs to indexes into the above array
	CHashTable<TEntityUID, TUInt32>* m_EntityUIDMap;

	// Entity IDs are provided using a single increasing integer
	TEntityUID m_NextUID;


	/////////////////////////////////////
	// Data for Entity Enumeration

	bool        m_IsEnumerating;
	TEntityIter m_EnumEntity;
	string      m_EnumName;
	string      m_EnumTemplateName;
	string      m_EnumTemplateType;
	
	TUInt32 i_Vectorsize;
	TUInt32 i_numRenderMethod;
};

} // namespace gen
