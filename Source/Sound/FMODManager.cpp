#include "FMODManager.h"
#include "EntityManager.h"
using namespace FMOD;
using namespace gen;


/*
SOUND BOARD OF THE PROJECT

Here I load sound file names into banks, not the actual files, if a sound file is missing, it will simply will not play
There are several banks:
General, mainly for long music sequences that play on background,
Player, for player sounds and quotes,
Stando, for player`s summon`s effects,
Monster, which is not used currently,
And Menu just to hold that one intro sequence
*/
extern CEntityManager EntityManager;
extern CCamera* MainCamera;

FMODManager::FMODManager()
{
	//initializing sound device below
	InitFMOD();
	//The AudioFolder is the folder I hold the music in, currently it is Media//Sound//
	SoundDb.push_back(AudioFolder + "SonochiIntro.mp3");
	SoundDb.push_back(AudioFolder + "Jotaro_Victory.mp3");
	SoundDb.push_back(AudioFolder + "tobecontinued.mp3");
	SoundDb.push_back(AudioFolder + "VocalPercussion.mp3");

	MenuSoundDb.push_back(AudioFolder + "MainMenuIntro.mp3");

	PlayerSoundDb.push_back(AudioFolder + "BasicAttack.wav");
	PlayerSoundDb.push_back(AudioFolder + "PlayerBlockStartSound.wav");
	PlayerSoundDb.push_back(AudioFolder + "PlayerBlockSound.wav");
	PlayerSoundDb.push_back(AudioFolder + "PlayerFootstep.wav");
	PlayerSoundDb.push_back(AudioFolder + "PlayerJump.wav");
	PlayerSoundDb.push_back(AudioFolder + "Special_OnUse.mp3");
	PlayerSoundDb.push_back(AudioFolder + "Ora_Special.mp3");
	PlayerSoundDb.push_back(AudioFolder + "Ora_Single_Hit.mp3");
	PlayerSoundDb.push_back(AudioFolder + "YareYare.mp3");
	PlayerSoundDb.push_back(AudioFolder + "MudaMudaMuda.mp3");
	PlayerSoundDb.push_back(AudioFolder + "Za Warudo.mp3");
	PlayerSoundDb.push_back(AudioFolder + "Wryyy.mp3");
	PlayerSoundDb.push_back(AudioFolder + "Swoosh.wav");
	PlayerSoundDb.push_back(AudioFolder + "BloodSplatter.wav");
	PlayerSoundDb.push_back(AudioFolder + "RodaRolla.mp3");
	PlayerSoundDb.push_back(AudioFolder + "DoubleUltCollisionEvent.mp3");
	PlayerSoundDb.push_back(AudioFolder + "KonoDioDa.mp3");
	PlayerSoundDb.push_back(AudioFolder + "HellToYou.mp3");
	PlayerSoundDb.push_back(AudioFolder + "DioIntro.mp3");

	StandoSoundDb.push_back(AudioFolder + "BasicAttack.wav");
	StandoSoundDb.push_back(AudioFolder + "StandoStarFinger.wav");
	StandoSoundDb.push_back(AudioFolder + "StandoStarBreaker.wav");
	StandoSoundDb.push_back(AudioFolder + "Ora_Ultimate.mp3");
	
	MonsterSoundDb.push_back(AudioFolder + "GenericPunch.wav");
	MonsterSoundDb.push_back(AudioFolder + "HardPunch.wav");
	MonsterSoundDb.push_back(AudioFolder + "ZombieDeath.wav");

	Player1 = false;
}



FMODManager::~FMODManager()
{
}
void FMODManager::InitFMOD()
{

	 pSystem = NULL;

	result = FMOD::System_Create(&pSystem);      // Create the main system object.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}

	result = pSystem->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (result != FMOD_OK)
	{
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}

	

}
//Global sounds, nothing special
void FMODManager::PlayGlobal(int VectorPosition, bool fadeIn)
{
	pSound->release();
	const string* str_ptr = &SoundDb[VectorPosition];
	const char *cstr = str_ptr->c_str();
	int rate;
	unsigned long long dspclock;

	
	pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pSound);
	pSystem->playSound(pSound,NULL,false,&pChannel);

	pSystem->getSoftwareFormat(&rate, 0, 0);

	if (VectorPosition == VocalPercussionSound)
	{
		pChannel->setVolume(0.15);
		pChannel->setLoopCount(5);
	}
	else
		pChannel->setVolume(1.0f);
}
void FMODManager::PlayMenuSound(int VectorPosition)
{
	pMenuSound->release();
	const string* str_ptr = &MenuSoundDb[VectorPosition];
	const char *cstr = str_ptr->c_str();
	pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pMenuSound);
	pSystem->playSound(pMenuSound, NULL, false, &pMenuChannel);
	SwitchMenuSounds(VectorPosition);
}
void FMODManager::PlayPlayerSound(int VectorPosition, bool fadeIn,bool isPlayer1)
{
	//Vector position is the,literally, vector position which is enumerated by names found in FMODManager.h, calling wrong Play... function with wrong vector position will result in vector error. We don`t do that.

	
	const string* str_ptr = &PlayerSoundDb[VectorPosition];
	const char *cstr = str_ptr->c_str();
	//Here we deside while player actually is the source of the music
	//Don`t forget to release the files to reduce number of memory leaks!
	if (VectorPosition >= PlayerBlockAttackStart && VectorPosition <= PlayerBlockAttackSound)
	{
	
		if (isPlayer1)
		{
			pPlayerDefenseSound->release();
			pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pPlayerDefenseSound);
			pSystem->playSound(pPlayerDefenseSound, NULL, false, &pPlayerDefenseChannel);
		}
		else
		{
			pPlayer2DefenseSound->release();
			pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pPlayer2DefenseSound);
			pSystem->playSound(pPlayer2DefenseSound, NULL, false, &pPlayer2DefenseChannel);
		}
	
	}
	else
	{
		if (isPlayer1)
		{
			pPlayerAttackSound->release();
			pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pPlayerAttackSound);
			pSystem->playSound(pPlayerAttackSound, NULL, false, &pPlayerChannel);
		}
		else
		{
			pPlayer2AttackSound->release();
			pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pPlayer2AttackSound);
			pSystem->playSound(pPlayer2AttackSound, NULL, false, &pPlayer2Channel);
		}
	}
	//Switch functions regulate loudness and fade of certain tracks that are either too loud or too long
	if(isPlayer1)
	SwitchPlayerPlaySound(VectorPosition, fadeIn,pPlayerChannel,pPlayerDefenseChannel);
	else
	SwitchPlayerPlaySound(VectorPosition, fadeIn, pPlayer2Channel, pPlayer2DefenseChannel);
	
}
void FMODManager::PlayStandoSound(int VectorPosition, bool fadeOut)
{
	pStandoAttackSound->release();
	const string* str_ptr = &StandoSoundDb[VectorPosition];
	const char *cstr = str_ptr->c_str();
	pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pStandoAttackSound);
	pSystem->playSound(pStandoAttackSound, NULL, false, &pStandoChannel);

	SwitchStandoAttackSound(VectorPosition, false);
}
void FMODManager::PlayMonsterSound(int VectorPosition, bool fadeOut)
{
	pMonsterAttackSound->release();
	const string* str_ptr = &MonsterSoundDb[VectorPosition];
	const char *cstr = str_ptr->c_str();
	pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pMonsterAttackSound);
	pSystem->playSound(pMonsterAttackSound, NULL, false, &pMonsterChannel);

	SwitchMonsterSounds(VectorPosition, false);
}
void FMODManager::SwitchMenuSounds(int vectorposition)
{
	int rate;
	unsigned long long dspclock;

	pSystem->getSoftwareFormat(&rate, 0, 0);

	switch (vectorposition)
	{
	case SonochiSound:
		pMenuChannel->setVolume(0.5f);
		break;
	default:
		break;
	}
}
void FMODManager::SwitchPlayerPlaySound(int vectorposition, bool fadein, Channel* pPlayerChannel,Channel* pPlayerDefenseChannel)
{
	//here is the example of such switch function
	//We track the time passed since we started playing the track, then count in the rate at which the track plays and use this data to add or remove fade points.
	int rate;
	unsigned long long dspclock;
	const string* str_ptr = &PlayerSoundDb[DoubleUltCollisionEventSound];
	const char *cstr = str_ptr->c_str();
	pSystem->getSoftwareFormat(&rate, 0, 0);
	switch (vectorposition)
	{
	case PlayerBasicAttackSound:
		break;
	case PlayerBlockAttackSound: case PlayerBlockAttackStart:
		pPlayerDefenseChannel->setVolume(0.25);
	case Special_SummonSound:
		pPlayerChannel->setVolume(0.25);
		break;
	case OraOraSpecialSound:
		pPlayerChannel->getDSPClock(0, &dspclock);
		pPlayerChannel->addFadePoint(dspclock + (rate * 1), 1.0f);
		pPlayerChannel->addFadePoint(dspclock + (rate * 1.1), 0.0f);
		break;
	case OraOraSingleHitSound: case KnivesFlySound:
		pPlayerChannel->setVolume(2);
		break;
	case ZaWarudoSound:   
		pPlayerChannel->setVolume(0.75f);
		break;
	case MudaMudaMudaSound:
		pPlayerChannel->setVolume(0.0f);
		pPlayerChannel->getDSPClock(0, &dspclock);
		pPlayerChannel->addFadePoint(dspclock + (rate * 1), 1.0f);
		pPlayerChannel->addFadePoint(dspclock + (rate * 1.5), 0.0f);
		break;
	case WryyySound:
		pPlayerChannel->getDSPClock(0, &dspclock);
		pPlayerChannel->addFadePoint(dspclock + (rate * 1), 1.0f);
		pPlayerChannel->addFadePoint(dspclock + (rate * 2), 0.0f);
		break;
	case DoubleUltCollisionEventSound:
		pStandoAttackSound->release();
		pPlayerAttackSound->release();
		pPlayer2AttackSound->release();
		
		break;
	case YareYareTauntSound:
		pPlayerChannel->setVolume(2.0f);
		break;
	case DioIntroSound:
		pPlayerChannel->setVolume(0.25f);
		break;
	default:
		break;
	}
}

void FMODManager::SwitchStandoAttackSound(int vectorposition, bool fadeout)
{
	int rate;
	unsigned long long dspclock;

	pSystem->getSoftwareFormat(&rate, 0, 0);

	switch (vectorposition)
	{
	case OraOraSpecialSound:
		pStandoChannel->setVolume(0.05f);
		break;
	case Stando_StarFingerSound:
		break;
	case Stando_StarBreakerSound:
		pStandoChannel->setVolume(0.2);
		pStandoChannel->getDSPClock(&dspclock, 0);
		pStandoChannel->addFadePoint(dspclock + (rate), 0.0f);
		pStandoChannel->addFadePoint(dspclock + (rate * 1), 1.0f);
		pStandoChannel->set3DDistanceFilter(true, 10, 10);
		break;
	case OraOraUltimateSound:
		pStandoChannel->setVolume(0.5f);
		pStandoChannel->getDSPClock(0, &dspclock);
		pStandoChannel->addFadePoint(dspclock + (rate * 3), 1.0f);
		pStandoChannel->addFadePoint(dspclock + (rate * 4), 0.0f);
		break;
	default:
		break;
	}
}
void FMODManager::SwitchMonsterSounds(int vectorposition, bool fadeout)
{
	int rate;
	unsigned long long dspclock;

	pSystem->getSoftwareFormat(&rate, 0, 0);
	switch (vectorposition)
	{
	case MonsterBasicHitSound:
		pMonsterChannel->setVolume(0.25f);
		break;
	}
}
void FMODManager::ClearAllSounds()
{
	//Clear function, if we need to silence everything at once
	pSound->release();
	pPlayerAttackSound->release();
	pPlayer2AttackSound->release();
    pPlayerDefenseSound->release();
	pPlayer2DefenseSound->release();
	pStandoAttackSound->release();
	pStando2AttackSound->release();
	pMonsterAttackSound->release();
	pMenuSound->release();
}
void FMODManager::PlayDoubleUltEvent()
{
	//Let`s hope you will get here
	const string* str_ptr = &PlayerSoundDb[DoubleUltCollisionEventSound];
	const char *cstr = str_ptr->c_str();
	pSystem->createSound(cstr, FMOD_DEFAULT, NULL, &pSound);
	pSystem->playSound(pSound, NULL, false, NULL);
}
	
