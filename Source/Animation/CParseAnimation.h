///////////////////////////////////////////////////////////
//  CParseLevel.cpp
//  A class to parse and setup a level (entity templates
//  and instances) from an XML file
//  Created on:      30-Jul-2005 14:40:00
//  Original author: LN
///////////////////////////////////////////////////////////

#ifndef GEN_C_PARSE_ANIM_H_INCLUDED
#define GEN_C_PARSE_ANIM_H_INCLUDED

#include <string>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "AnimationManager.h"
#include "CParseXML.h"

namespace gen
{

	/*---------------------------------------------------------------------------------------------
	CParseLevel class
	---------------------------------------------------------------------------------------------*/
	// A XML parser to read and setup a level - made up of entity templates and entity instances
	// Derived from the general CParseXML class, which performs the basic syntax parsing. The base
	// class calls functions (overridden) in this class when it encounters the start and end of
	// elements in the XML (opening and closing tags). These functions then perform appropriate
	// setup. This is an event driven system, requiring this class to store state - the entity /

	// template / member variables it is currently building
	class CParseAnimation : public CParseXML
	{

		/*---------------------------------------------------------------------------------------------
		Constructors / Destructors
		---------------------------------------------------------------------------------------------*/
	public:
		// Constructor gets a pointer to the entity manager and initialises state variables
		CParseAnimation(CAnimationManager* animationManager);


		/*-----------------------------------------------------------------------------------------
		Private interface
		-----------------------------------------------------------------------------------------*/
	private:

		/*---------------------------------------------------------------------------------------------
		Types
		---------------------------------------------------------------------------------------------*/

		// File section currently being parsed
		enum EFileSection
		{
			None,
			Player,
			Monster,
			Menu,
			UI,
		};
		enum EFilePlayerSection
		{
			Jotaro,
			Dio,
			
		};

		/*---------------------------------------------------------------------------------------------
		Callback functions
		---------------------------------------------------------------------------------------------*/

		// Callback function called when the parser meets the start of a new element (the opening tag).
		// The element name is passed as a string. The attributes are passed as a list of (C-style)
		// string pairs: attribute name, attribute value. The last attribute is marked with a null name
		void StartElt(const string& eltName, SAttribute* attrs);

		// Callback function called when the parser meets the end of an element (the closing tag). The
		// element name is passed as a string


		void  StartInfoElt(const string& eltName, SAttribute* attrs);
		void NPCStartElt(const string& eltName, SAttribute* attrs);

		void PlayerEndElt(const string& eltName);

		void CheckPlayerElt(const string& eltName, SAttribute* attrs);
		void CheckMonsterElt(const string& eltName, SAttribute* attrs);
		void CheckUIElt(const string& eltName, SAttribute* attrs);
		void CheckMenuElt(const string& eltName, SAttribute* attrs);
		void NPCEndElt(const string& eltName);
		/*---------------------------------------------------------------------------------------------
		Section Parsing
		---------------------------------------------------------------------------------------------*/

		// 




		/*---------------------------------------------------------------------------------------------
		Entity Template and Instance Creation
		---------------------------------------------------------------------------------------------*/

		// Create an entity template using data collected from parsed XML elements
		

		// Create an entity using data collected from parsed XML elements
		void CreateAnimation();


		/*---------------------------------------------------------------------------------------------
		Data
		---------------------------------------------------------------------------------------------*/

		// Constructer is passed a pointer to an entity manager used to create templates and
		// entities as they are parsed
		CAnimationManager* m_AnimationManager;

		// File state
		EFileSection m_CurrentSection;

		// Current template state (i.e. latest values read during parsing)
		
		string   m_animPath;
	    
		bool b_currentPlayerJotaro = true;
		
		PlayerAnimationType m_AnimationType;
		MonsterTypes m_MonsterType;
		MonsterAnimationTypes m_MonsterAnimType;
		PlayerUIAnimationTypes m_PlayerUIAnimType;
		MenuUIAnimationTypes m_MenuUIAnimType;
		AnimationSequence m_TempVector;
		bool b_rightDir = true;
		
		// Current entity state (i.e. latest values read during parsing)
	

	};


} // namespace gen

#endif // GEN_C_PARSE_LEVEL_H_INCLUDED