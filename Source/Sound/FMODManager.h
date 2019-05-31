

#include <iostream>
#include "fmod.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include <string>
#include <vector>

using namespace std;
using namespace FMOD;

static const string AudioFolder = "Media\\Sound\\";

enum SoundDbTags
{
	IntroSound,
	VictorySound,
    ToBeContinuedSound,
	VocalPercussionSound,
	LastS,
};
enum PlayerAttackSoundDbTags
{
	PlayerBasicAttackSound,
	PlayerBlockAttackStart,
	PlayerBlockAttackSound,
	PlayerFootstepSound,
	PlayerJumpSound,
	Special_SummonSound,
	OraOraSpecialSound,
	OraOraSingleHitSound,
	YareYareTauntSound,
	MudaMudaMudaSound,
	ZaWarudoSound,
	WryyySound,
	KnivesFlySound,
	BloodSplatterSound,
	RodaRollaSound,
	DoubleUltCollisionEventSound,
	KonoDioDaSound,
	HellToYouSound,
	DioIntroSound,
	LastPlayerJtSounds,
};

enum StandoJtAttackSoundDbTags
{
	StandoBasicAttackSound,
	Stando_StarFingerSound,
	Stando_StarBreakerSound,
	OraOraUltimateSound,
	LastStandoJtSounds,
};
enum MonsterSoundDbTags
{
	MonsterBasicHitSound,
	MonsterHardHitSound,
	ZombieDeathSound,
	LastMosterSounds,
};
enum MainMenuSounds
{
	SonochiSound,
	LastMenuSounds,
};
class FMODManager
{
public:

	bool Player1;
	bool Player2;

	FMODManager();
	~FMODManager();

	void InitFMOD();
	void ExitFMOD();

	void FadeThink();

	bool IsSoundPlaying(const char* pathToFileFromSoundsFolder);

	void PlayGlobal(int position, bool fadeOut);
	void PlayPlayerSound(int position, bool fadeOut, bool isPlayer1);
	void PlayMenuSound(int position);
	void PlayDoubleUltEvent();
	void ClearAllSounds();
	void PlayStandoSound(int position, bool fadeout);
	void PlayMonsterSound(int position, bool fadeout);
	void StopAmbientSound(bool fadeOut);
	void TransitionAmbientSounds(const char* pathToFileFromSoundsFolder);
	


private:

	System * pSystem;
	Sound			*pSound;
	Sound           *pMenuSound;
	Sound           *pPlayerAttackSound;
	Sound           *pPlayer2AttackSound;
	Sound           *pPlayerDefenseSound;
	Sound           *pPlayer2DefenseSound;
	Sound           *pStandoAttackSound;
	Sound           *pStando2AttackSound;
	Sound           *pMonsterAttackSound;
	SoundGroup		*pSoundGroup;
	Channel			*pChannel;
	Channel         *pMenuChannel;
	Channel         *pPlayerChannel;
	Channel         *pPlayer2Channel;
	Channel         *pPlayerDefenseChannel;
	Channel         *pPlayer2DefenseChannel;
	Channel         *pStandoChannel;
	Channel         *pStando2Channel;
	Channel         *pMonsterChannel;
	ChannelGroup	*pChannelGroup;
	FMOD_RESULT		result;
	const char* GetFullPathToSound(const char* pathToFileFromModFolder);
	const char* GetCurrentSoundName(void);
	void SwitchPlayerPlaySound(int vectorposition, bool fadein, Channel* attackChannel, Channel* defenseChannel);
	void SwitchStandoAttackSound(int vectorposition, bool fadeOut);
	void SwitchMonsterSounds(int vectorposition, bool fadeOut);
	void SwitchMenuSounds(int vectorposition);
	const char* currentSound;
	const char* TransitionTo;
	bool m_bShouldTransition;
	bool m_bFadeIn;
	bool m_bFadeOut;
	float m_fFadeDelay;

	vector<string> SoundDb;
	vector<string> MenuSoundDb;
	vector<string> PlayerSoundDb;
	vector<string> Player2SoundDb;
	vector<string> StandoSoundDb;
	vector<string> MonsterSoundDb;
};





