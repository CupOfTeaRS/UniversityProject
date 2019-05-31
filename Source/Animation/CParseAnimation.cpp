///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////

#include "BaseMath.h"
#include "Entity.h"
#include "CParseAnimation.h"

namespace gen
{

	/*---------------------------------------------------------------------------------------------
	Constructors / Destructors
	---------------------------------------------------------------------------------------------*/

	// Constructor initialises state variables
	CParseAnimation::CParseAnimation(CAnimationManager* animationManager)
	{
		// Take copy of entity manager for creation
		m_AnimationManager = animationManager;

		// File state
		m_CurrentSection = None;

		// Template state
	

	
	}


	/*---------------------------------------------------------------------------------------------
	Callback Functions
	---------------------------------------------------------------------------------------------*/

	// Callback function called when the parser meets the start of a new element (the opening tag).
	// The element name is passed as a string. The attributes are passed as a list of (C-style)
	// string pairs: attribute name, attribute value. The last attribute is marked with a null name
	void CParseAnimation::StartElt(const string& eltName, SAttribute* attrs)
	{
		
		CheckPlayerElt(eltName, attrs);
		CheckMonsterElt(eltName, attrs);
		CheckUIElt(eltName, attrs);
		CheckMenuElt(eltName, attrs);
		StartInfoElt(eltName, attrs);

		
	}

	// Callback function called when the parser meets the end of an element (the closing tag). The
	// element name is passed as a string
	


	/*---------------------------------------------------------------------------------------------
	Section Parsing
	---------------------------------------------------------------------------------------------*/



	// Called when the parser meets the start of an element (opening tag) in the entities section
	void CParseAnimation::StartInfoElt(const string& eltName, SAttribute* attrs)
	{
		//place for more types

	

		if (eltName == "Frame")
		{
			
			AnimPair temp;
			temp.first = GetAttributeInt(attrs, "ID");
			
			temp.second = GetAttribute(attrs, "Path");
			m_TempVector.push_back(temp);
			bool isLastReached = GetAttributeInt(attrs, "Last");
			if (isLastReached )
			{
				CreateAnimation();
				m_TempVector.clear();
				
			}
		}

	}

	// Called when the parser meets the end of an element (closing tag) in the entities section
	void CParseAnimation::PlayerEndElt(const string& eltName)
	{/*
		if (eltName == "RIGHT")
		{
			CreateAnimation();
			m_TempVector.clear();
		}*/
	}


	/*---------------------------------------------------------------------------------------------
	Entity Template and Instance Creation
	---------------------------------------------------------------------------------------------*/

	// Create an entity template using data collected from parsed XML elements
	void CParseAnimation::CheckMenuElt(const string& eltName, SAttribute* attrs)
	{
		if (eltName == "Menu")
		{
			m_CurrentSection = Menu;
		}
		if (eltName == "INTRO")
		{
			m_MenuUIAnimType = IntroAnim;
		}
		if (eltName == "MODE_SELECT")
		{
			m_MenuUIAnimType = ModeSelectAnim;
		}
		if (eltName == "JOTARO_SELECT")
		{
			m_MenuUIAnimType = JotaroSelectAnim;
		}
		if (eltName == "DIO_SELECT")
		{
			m_MenuUIAnimType = DioSelectAnim;
		}
	}
	void CParseAnimation::CheckPlayerElt(const string& eltName, SAttribute* attrs)
	{
		if (eltName == "Player")
		{
			m_CurrentSection = Player;
		}
		 if (eltName == "MONSTER")
		{
			m_CurrentSection = Monster;
		}

		if (eltName == "Jotaro")
		{
			b_currentPlayerJotaro = true;
		}
		if (eltName == "Dio")
		{
			m_CurrentSection = Player;
			b_currentPlayerJotaro = false;
		}
		if (eltName == "IDLE")
		{
			m_AnimationType = Idle;
		}
		if (eltName == "WALK")
		{
			m_AnimationType = Walking;
		}
		if (eltName == "JUMP")
		{
			m_AnimationType = Jump;
		}
		if (eltName == "BLOCK")
		{
			m_AnimationType = Block;
		}
		if (eltName == "CROUCH")
		{
			m_AnimationType = Crouch;
		}
		if (eltName == "CROUCH_TURN")
		{
			m_AnimationType = Crouch_Turn;
		}
		if (eltName == "CROUCH_BLOCK")
		{
			m_AnimationType = Crouch_Block;
		}
		if (eltName == "BLOCK_AIR")
		{
			m_AnimationType = Block_Air;
		}
		if (eltName == "TURNAROUND")
		{
			m_AnimationType = Turning;
		}
		if (eltName == "STANDUP")
		{
			m_AnimationType = Stand_Up;
		}
		if (eltName == "DASH_FW")
		{
			m_AnimationType = Dash;
		}
		if (eltName == "DASH_BW")
		{
			m_AnimationType = Dash_Back;
		}
		if (eltName == "SUMMON_STANDO")
		{
			m_AnimationType = Summon;
		}
		if (eltName == "SUMMON_STANDO_AIR")
		{
			m_AnimationType = Summon_Air;
		}
		if (eltName == "LIGHT_LEG_ATTACK")
		{
			m_AnimationType = Light_Leg_Att;
		}
		if (eltName == "LIGHT_CROUCH_ATTACK")
		{
			m_AnimationType = Light_Crouch_Att;
		}
		if (eltName == "MEDIUM_ATTACK")
		{
			m_AnimationType = Medium_Att;
		}
		if (eltName == "MEDIUM_WALK_ATTACK")
		{
			m_AnimationType = Medium_Walk_Att;
		}
		if (eltName == "MEDIUM_CROUCH_ATTACK")
		{
			m_AnimationType = Medium_Crouch_Att;
		}
		if (eltName == "MEDIUM_AIR_ATTACK")
		{
			m_AnimationType = Medium_Air_Att;
		}
		if (eltName == "HEAVY_ATTACK")
		{
			m_AnimationType = Heavy_Att;
		}
		if (eltName == "HEAVY_WALK_ATTACK")
		{
			m_AnimationType = Heavy_Walk_Att;
		}
		if (eltName == "HEAVY_CROUCH_ATTACK")
		{
			m_AnimationType = Heavy_Crouch_Att;
		}
		if (eltName == "HEAVY_CROUCH_FR_ATTACK")
		{
			m_AnimationType = Heavy_Crouch_Fr_Att;
		}
		if (eltName == "HEAVY_AIR_ATTACK")
		{
			m_AnimationType = Heavy_Air_Att;
		}
		if (eltName == "THROW")
		{
			m_AnimationType = Throw;
		}
		if (eltName == "STANDO_ORAORAORA")
		{
			m_AnimationType = Stando_OraOraOra;
		}
		if (eltName == "STANDO_SP_2")
		{
			m_AnimationType = Stando_Sp_2;
		}
		if (eltName == "STANDO_SP_3")
		{
			m_AnimationType = Stando_Sp_3;
		}
		if (eltName == "STANDO_SP_4")
		{
			m_AnimationType = Stando_Sp_4;
		}
		if (eltName == "STANDO_ULT_1")
		{
			m_AnimationType = Stando_Ult_1;
		}
		if (eltName == "STANDO_IDLE")
		{
			m_AnimationType = Stando_Idle;
		}
		if (eltName == "SPECIAL_ORAORAORA")
		{
			m_AnimationType = Special_OraOraOra;
		}
		if (eltName == "SPECIAL_NUMBER_2")
		{
			m_AnimationType = Special_Num_2;
		}
		if (eltName == "SPECIAL_NUMBER_3")
		{
			m_AnimationType = Special_Num_3;
		}
		if (eltName == "SPECIAL_NUMBER_4")
		{
			m_AnimationType = Special_Num_4;
		}
		if (eltName == "ULTIMATE_NUMBER_1")
		{
			m_AnimationType = Ult_Num_1;
		}
		if (eltName == "IS_HIT_LIGHT")
		{
			m_AnimationType = Is_Hit_Light;
		}
		if (eltName == "IS_HIT_LIGHT_2")
		{
			m_AnimationType = Is_Hit_Light_2;
		}
		if (eltName == "IS_HIT_MEDIUM")
		{
			m_AnimationType = Is_Hit_Medium;
		}
		if (eltName == "IS_HIT_MEDIUM_2")
		{
			m_AnimationType = Is_Hit_Medium_2;
		}
		if (eltName == "IS_HIT_HARD")
		{
			m_AnimationType = Is_Hit_Hard;
		}
		if (eltName == "IS_HIT_HARD_2")
		{
			m_AnimationType = Is_Hit_Hard_2;
		}
		if (eltName == "IS_HIT_CROUCH")
		{
			m_AnimationType = Is_Hit_Crouch;
		}
		if (eltName == "IS_HIT_CROUCH_2")
		{
			m_AnimationType = Is_Hit_Crouch_2;
		}
		if (eltName == "IS_HIT_AIR")
		{
			m_AnimationType = Is_Hit_Air;
		}
		if (eltName == "IS_KILLED")
		{
			m_AnimationType = Is_Killed;
		}
		if (eltName == "VICTORY")
		{
			m_AnimationType = Victory;
		}
		if (eltName == "VICTORY_2")
		{
			m_AnimationType = Victory_2;
		}
		if (eltName == "INTRO")
		{
			m_AnimationType = Intro;
		}
		if (eltName == "INTRO_2")
		{
			m_AnimationType = Intro_2;
		}
		if (eltName == "RIGHT")
		{
			b_rightDir = true;
		}
		else if (eltName == "LEFT")
		{
			b_rightDir = false;
		}
	}
	// Create an entity using data collected from parsed XML elements
	void CParseAnimation::CreateAnimation()
	{
		if (m_CurrentSection == Player)
			m_AnimationManager->PushAnimation(m_TempVector, b_currentPlayerJotaro, b_rightDir, m_AnimationType);
		else if (m_CurrentSection == Monster)
			m_AnimationManager->PushMonsterAnimation(m_TempVector, m_MonsterType, m_MonsterAnimType, b_rightDir);
		else if (m_CurrentSection == UI)
			m_AnimationManager->PushUIAnimation(m_TempVector, b_currentPlayerJotaro, b_rightDir, m_PlayerUIAnimType);
		else if (m_CurrentSection == Menu)
			m_AnimationManager->PushMenuUIAnimation(m_TempVector, m_MenuUIAnimType);

	}
	void CParseAnimation::CheckMonsterElt(const string& eltName, SAttribute* attrs)
	{
		if (eltName == "MONSTER")
		{
			m_CurrentSection = Monster;
		}
		if (m_CurrentSection == Monster)
		{
			if (eltName == "ZOMBIE")
			{
				m_MonsterType = Zombie;
			}

			if (eltName == "WALK")
			{
				m_MonsterAnimType = Walk;
			}

			if (eltName == "RIGHT")
			{
				b_rightDir = true;
			}
			else if (eltName == "LEFT")
			{
				b_rightDir = false;
			}
		}
		
	}
	void CParseAnimation::CheckUIElt(const string& eltName, SAttribute* attrs)
	{
		if (eltName == "CHARACTER_UI")
		{
			m_CurrentSection = UI;
		}
		if (m_CurrentSection == UI)
		{
			if (eltName == "FACE")
			{
				m_PlayerUIAnimType = FaceAnim;
			}
			if (eltName == "HP_BAR")
			{
				m_PlayerUIAnimType = HealthBarAnim;
			}
			if (eltName == "ULT_BAR")
			{
				m_PlayerUIAnimType = Ult_Meter_Anim;
			}
			if (eltName == "ULT_BAR_READY")
			{
				m_PlayerUIAnimType = Ult_Meter_Ready_Anim;
			}
			if (eltName == "RIGHT")
			{
				b_rightDir = false;
			}
			if (eltName == "LEFT")
			{
				b_rightDir = true;
			}
		}
	}

} // namespace gen