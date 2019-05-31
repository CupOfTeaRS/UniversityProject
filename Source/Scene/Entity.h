/*******************************************
	Entity.h

	Base entity template and entity classes
********************************************/

#pragma once

#include <string>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "CMatrix4x4.h"
#include "Camera.h"
#include "Mesh.h"
#include "CMonsterEntity.h"
namespace gen
{

/////////////////////////////////////
//	Public types

// An entity UID is just a 32 bit value
typedef TUInt32 TEntityUID;
const TEntityUID SystemUID = 0xffffffff;
const TEntityUID PlayerUID = 0x091a4f3;
const TEntityUID Player2UID = 0x001a4f5;
/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Entity Template Base Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// Base entity template only contains a mesh, i.e. the only common feature of all entities
// is that they have some geometry. In fact, if we had cameras or lights as entities, we couldn't
// even make this assumption. However, this is just a simple example of an entity system
class CEntityTemplate
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Base entity template constructor needs template type (e.g. "Car"), name (e.g. "Fiat Panda")
	// and the associated mesh (e.g. "panda.x")
	CEntityTemplate( const string& type, const string& name, const string& meshFilename )
	{
		m_Type = type;
		m_Name = name;

		// Load mesh
		m_Mesh = new CMesh();
		if (!m_Mesh->Load( meshFilename ))
		{
			string errorMsg = "Error loading mesh " + meshFilename;
			SystemMessageBox( errorMsg.c_str(), "Mesh Error" );
			throw; // failure in constructor can only be signalled with exception 
		}
	}

	// Destructor - base class destructors should always be virtual
	virtual ~CEntityTemplate()
	{
		delete m_Mesh;
	}
	
private:
	// Prevent use of copy constructor and assignment operator (private and not defined)
	CEntityTemplate( const CEntityTemplate& );
	CEntityTemplate& operator=( const CEntityTemplate& );


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	//	Getters

	const string& GetType()
	{
		return m_Type;
	}

	const string& GetName()
	{
		return m_Name;
	}

	CMesh* const Mesh()
	{
		return m_Mesh;
	}


/////////////////////////////////////
//	Private interface
private:

	// Type and name of the template
	string m_Type;
	string m_Name;

	// The mesh representing this entity
	CMesh* m_Mesh;
	
	
};



/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Base Entity Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

// Base entity holds a pointer to its template data and the current position as a set of
// matrices. The entity can be rendered but its update function does nothing - base class
// entities are assumed to be static scene elements
class CEntity
{
/////////////////////////////////////
//	Constructors/Destructors
public:
	// Base entity constructor, needs pointer to common template data and UID, may also pass 
	// name, initial position, rotation and scaling. Set up positional matrices for the entity
	CEntity();
	CEntity
	(
		CEntityTemplate* entityTemplate,
		TEntityUID       UID,
		const string&    name = "",
		const CVector3&  position = CVector3::kOrigin, 
		const CVector3&  rotation = CVector3( 0.0f, 0.0f, 0.0f ),
		const CVector3&  scale = CVector3( 1.0f, 1.0f, 1.0f )
	);

	// Destructor - base class destructors should always be virtual
	virtual ~CEntity()
	{
		delete[] m_Matrices;
		delete[] m_RelMatrices;
	}

private:
	// Prevent use of copy constructor and assignment operator (private and not defined)
	CEntity( const CEntity& );
	CEntity& operator=( const CEntity& );


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	// Getters

	 TEntityUID const GetUID()
	{
		return m_UID;
	}

	CEntityTemplate* const Template()
	{
		return m_Template;
	}

	const string& GetName()
	{
		return m_Name;
	}

	SMeshBucketRenderData getSubmeshData(int num)
	{
		return renderdata[num];
	}
	int getSubmeshCount()
	{
		return renderdata.size();

	}
	
	/////////////////////////////////////
	// Matrix access

	// Direct access to position and matrix
	CVector3& Position( TUInt32 node = 0 )
	{
		return m_RelMatrices[node].Position();
	}
	virtual CMatrix4x4& Matrix( TUInt32 node = 0 )
	{
		return m_RelMatrices[node];
	}

	int GetSubMeshCount()
	{
		return i_subMeshCount;
	}

	/////////////////////////////////////
	// Update / Render

	// Perform whatever update is required for this entity, pass time since last update
	// Return false if the entity is to be destroyed
	// Virtual function, base version does nothing
	virtual bool Update(TFloat32 updateTime);

	virtual bool Animate(TFloat32 updateTime, bool isFacingRight, int AnimationType) { return true; }
	 virtual bool RenderEntityUI(TFloat32 updateTime);

	// Render the entity
	void PreRender();
	void Render();
	void BucketRender(ERenderMethod method);
	void ShadowRender();
	vector<ERenderMethod> renderMethods;
	vector<int> submeshToRender;

	int renderMethodSize;
	bool isTexFlippedHorizontal;
	bool doNotTouch = false;
	bool isDepthSorted = false;
	float depthFromCamera;
	CMesh* Mesh;
	float animChangeTimer = 0.0f;
	float animCycleDelay = 10.5;
	float dmgImmunityTimer = 0.0f;
	TInt32 AnimMultNormal = 250;
	TInt32 AnimMultSlow = 75;
	TInt32 AnimMultFast = 350;
	TInt32 AnimMultSplitSec = 500;
	float FloatingCounter;
	TFloat32 m_GroundLevel = 10.0f;
	TFloat32 m_Gravity = 2.5f;
	TFloat32 m_UpdwardVel = 0.0f;
	bool isInAir = false;
	TFloat32 m_HorizontalVel = 0.0f;
	bool isInKnockback = false;
	bool isKnockbackRight = true;

	TFloat32 deathTimer = 0.0f;
	bool isDeleted = false;
	int currentAnim = 0;
	bool isAMonster = false;
	struct MonsterStats
	{
		int hp;
		int hp_max;
		float hp_toShow;
		float movespeed;
	};
	MonsterStats monsterStats;
	int thisMonsterType;
	bool isCollidingWithPlayer = false;
	bool isCollidingWithPlayerToDamage = false;
	bool isCollidingWithPlayerStando = false;
	TFloat32 distFromCenter;
/////////////////////////////////////
//	Private interface
private:

	CMonsterEntity * monsterComponent;
	// The template used by this entity - the common data for all entities of this type
	CEntityTemplate* m_Template;

	// Unique identifier and name for the entity
	TEntityUID  m_UID;
	string      m_Name;

	// Relative and absolute world matrices for each node in the template's mesh
	CMatrix4x4* m_RelMatrices; // Dynamically allocated arrays
	CMatrix4x4* m_Matrices;

	int i_subMeshCount;
	meshRenderData renderdata;

	bool Animate(TFloat32 updateTime);

	AnimationSequence MonsterAnimsRight[LastMonsterAnims];
	AnimationSequence MonsterAnimsLeft[LastMonsterAnims];
	// Relative and absolute world matrices for each node in the template's mesh
	private:
		bool isFacingRight = false;
		void AssembleMonster();
protected:
	struct EntityStats
	{
		TUInt32 hp;
		TUInt32 hp_max;
		
	};
	
	
	
};


} // namespace gen
