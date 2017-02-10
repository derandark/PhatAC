
#define MAX_MOTION_QUEUE 6
#define MAX_EMOTE_QUEUE 6

void Animation_Init();
void Animation_Shutdown();
DWORD Animation_GetAnimationSet();

void Animation_Attack(DWORD dwTarget, WORD wAnim, float fSpeed, DWORD dwWeaponID);
void Animation_PlayPrimary(WORD wAnim, float fSpeed, float fDelay);

void Animation_Think();
BOOL Animation_IsActive();

virtual void Animation_Complete(animation_t *data);
void Animation_ClipMotions(int iMaxCount = MAX_MOTION_QUEUE);
void Animation_PlayAnimation(animation_t* data);
void Animation_PlayEmote(WORD wIndex, float fSpeed);
int Animation_EmoteQueueCount();

void Animation_PlaySimpleAnimation(WORD wIndex, float fSpeed = 1.0f, float fDelay = 1.0f, DWORD dwAction = 0, DWORD dwActionEx = 0);

void Animation_Jump(float fPower, float fDir1, float fDir2, float fHeight);
void Animation_SetCombatStance(WORD);
void Animation_SetCombatMode(WORD);

BinaryWriter* Animation_GetAnimationInfo();
void Animation_Update();

BOOL m_bAnimUpdate;

DWORD m_dwAnimationSet;

animation_list m_lAnimQueue;
WORD m_wAnimSequence;

WORD m_wStance;
WORD m_wForwardAnim, m_wStrafAnim, m_wTurnAnim;
float m_fForwardSpeed, m_fStrafSpeed, m_fTurnSpeed;
float m_fSpeedMod;

BOOL m_bForced;


