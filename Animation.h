#pragma pack(push, 1)

typedef struct animation_s
{
	BYTE bType;
	WORD wIndex; //The index in the animation set.
	WORD wSequence; //The animation's sequence number.
	float fSpeed; //The speed this animation is to be played.

	double fStartTime; //..
	double fEndTime; //..

	DWORD dwStartFrame;
	DWORD dwEndFrame;
	DWORD dwFrameNum; //Current frame of this animation!

	DWORD dwTarget;

	DWORD dwAction; //For identifying actions (lifestone recall, etc.)
	DWORD dwActionData[10];
} animation_t;
#pragma pack(pop)

typedef std::list<animation_t> animation_list;
