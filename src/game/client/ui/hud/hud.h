/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//

#pragma once

#include "common_types.h"
#include "cl_dll.h"
#include "ammo.h"
#include "palette.h"
#include "cl_util.h"
#include "parsemsg.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS 2
#define DHN_3DIGITS 4
#define MIN_ALPHA 100

#define HUDELEM_ACTIVE 1

typedef struct
{
	int x, y;
} POSITION;

#include "global_consts.h"

typedef struct
{
	unsigned char r, g, b, a;
} RGBA;

typedef struct cvar_s cvar_t;


#define HUD_ACTIVE 1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH 32

#define MAX_MOTD_LENGTH 1536

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	POSITION m_pos;
	int m_type;
	int m_iFlags; // active, moving,
	virtual ~CHudBase() {}
	virtual bool Init() { return false; }
	virtual bool VidInit() { return false; }
	virtual bool Draw(float flTime) { return false; }
	virtual void Think() {}
	virtual void Reset() {}
	virtual void InitHUDData() {} // called every time a server is connected to
};

struct HUDLIST
{
	CHudBase* p;
	HUDLIST* pNext;
};



//
//-----------------------------------------------------
//
#include "voice_status.h" // base voice handling class
#include "hud_spectator.h"


//
//-----------------------------------------------------
//
class CHudAmmo : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	void Think() override;
	void Reset() override;
	bool DrawWList(float flTime);
	bool MsgFunc_CurWeapon(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_WeaponList(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_AmmoX(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_AmmoPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_WeapPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ItemPickup(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_HideWeapon(const char* pszName, int iSize, void* pbuf);

	void SlotInput(int iSlot);
	void UserCmd_Slot1();
	void UserCmd_Slot2();
	void UserCmd_Slot3();
	void UserCmd_Slot4();
	void UserCmd_Slot5();
	void UserCmd_Slot6();
	void UserCmd_Slot7();
	void UserCmd_Slot8();
	void UserCmd_Slot9();
	void UserCmd_Slot10();
	void UserCmd_Close();
	void UserCmd_NextWeapon();
	void UserCmd_PrevWeapon();

	bool IsCrosshairDrawn() const { return m_DrawCrosshair; }

	void SetDrawCrosshair(bool state)
	{
		m_DrawCrosshair = state;
	}

	void SetCrosshair(HSPRITE sprite, Rect rect);
	void SetAutoaimCrosshair(HSPRITE sprite, Rect rect);

private:
	void DrawCrosshair(int x, int y);

private:
	struct Crosshair
	{
		HSPRITE sprite = 0;
		Rect rect{};
	};

	float m_fFade;
	RGBA m_rgba;
	WEAPON* m_pWeapon;
	int m_HUD_bucket0;
	int m_HUD_selection;

	bool m_DrawCrosshair = true;

	Crosshair m_Crosshair;
	Crosshair m_AutoaimCrosshair;

	cvar_t* m_pCvarCrosshairScale = nullptr;
};


#include "health.h"


#define FADE_TIME 100


//
//-----------------------------------------------------
//
class CHudGeiger : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Geiger(const char* pszName, int iSize, void* pbuf);

private:
	int m_iGeigerRange;
};

//
//-----------------------------------------------------
//
class CHudTrain : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Train(const char* pszName, int iSize, void* pbuf);

private:
	HSPRITE m_hSprite;
	int m_iPos;
};

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	void Reset() override;
	void ParseStatusString(int line_num);

	bool MsgFunc_StatusText(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_StatusValue(const char* pszName, int iSize, void* pbuf);

protected:
	enum
	{
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 3,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH]; // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	 // the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];						 // an array of values for use in the status bar

	bool m_bReparseString; // set to true whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float* m_pflNameColors[MAX_STATUSBAR_LINES];
};

struct extra_player_info_t
{
	short frags;
	short deaths;
	short playerclass;
	short health; // UNUSED currently, spectator UI would like this
	bool dead;	  // UNUSED currently, spectator UI would like this
	short teamnumber;
	char teamname[MAX_TEAM_NAME];
	short teamid;
	short flagcaptures;
};

struct team_info_t
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	bool ownteam;
	short players;
	bool already_drawn;
	bool scores_overriden;
	int teamnumber;
};

#include "player_info.h"

//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_DeathMsg(const char* pszName, int iSize, void* pbuf);

private:
	int m_HUD_d_skull; // sprite index of skull icon
};

//
//-----------------------------------------------------
//
class CHudMenu : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	void Reset() override;
	bool Draw(float flTime) override;
	bool MsgFunc_ShowMenu(const char* pszName, int iSize, void* pbuf);

	void SelectMenuItem(int menu_item);

	bool m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	bool m_fWaitingForMore;
};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	bool Init() override;
	void InitHUDData() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_SayText(const char* pszName, int iSize, void* pbuf);
	void SayTextPrint(const char* pszBuf, int iBufSize, int clientIndex = -1);
	void EnsureTextFitsInOneLineAndWrapIfHaveTo(int line);
	friend class CHudSpectator;

private:
	struct cvar_s* m_HUD_saytext;
	struct cvar_s* m_HUD_saytext_time;
	struct cvar_s* m_con_color;
};

//
//-----------------------------------------------------
//
class CHudBattery : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_Battery(const char* pszName, int iSize, void* pbuf);

private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	Rect* m_prc1;
	Rect* m_prc2;
	int m_iBat;
	int m_iBatMax;
	float m_fFade;
	int m_iHeight; // width of the battery innards
};


//
//-----------------------------------------------------
//
class CHudFlashlight : public CHudBase
{
private:
	struct LightData
	{
		HSPRITE m_hSprite1 = 0;
		HSPRITE m_hSprite2 = 0;
		HSPRITE m_hBeam = 0;
		Rect* m_prc1 = nullptr;
		Rect* m_prc2 = nullptr;
		Rect* m_prcBeam = nullptr;
		int m_iWidth = 0; // width of the battery innards
	};

public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	void Reset() override;
	bool MsgFunc_Flashlight(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_FlashBat(const char* pszName, int iSize, void* pbuf);

private:
	LightData* GetLightData()
	{
		switch (m_SuitLightType)
		{
		default:
		case SuitLightType::Flashlight:
			return &m_Flashlight;
		case SuitLightType::Nightvision:
			return &m_Nightvision;
		}
	}

	void DrawNightVision();

private:
	LightData m_Flashlight;
	LightData m_Nightvision;

	HSPRITE m_nvSprite;

	SuitLightType m_SuitLightType = SuitLightType::Flashlight;
	float m_flBat;
	int m_iBat;
	bool m_fOn;
	float m_fFade;
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t* pMessage;
	float time;
	int x, y;
	int totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

//
//-----------------------------------------------------
//

class CHudTextMessage : public CHudBase
{
public:
	bool Init() override;
	static char* LocaliseTextString(const char* msg, char* dst_buffer, int buffer_size);
	static char* BufferedLocaliseTextString(const char* msg);
	const char* LookupString(const char* msg_name, int* msg_dest = nullptr);
	bool MsgFunc_TextMsg(const char* pszName, int iSize, void* pbuf);
};

//
//-----------------------------------------------------
//

class CHudMessage : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw(float flTime) override;
	bool MsgFunc_HudText(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_HudTextPro(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_GameTitle(const char* pszName, int iSize, void* pbuf);

	float FadeBlend(float fadein, float fadeout, float hold, float localTime);
	int XPosition(float x, int width, int lineWidth);
	int YPosition(float y, int height);

	void MessageAdd(const char* pName, float time);
	void MessageAdd(client_textmessage_t* newMessage);
	void MessageDrawScan(client_textmessage_t* pMessage, float time);
	void MessageScanStart();
	void MessageScanNextChar();
	void Reset() override;

private:
	client_textmessage_t* m_pMessages[maxHUDMessages];
	float m_startTime[maxHUDMessages];
	message_parms_t m_parms;
	float m_gameTitleTime;
	client_textmessage_t* m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH 24

class CHudStatusIcons : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	void Reset() override;
	bool Draw(float flTime) override;
	bool MsgFunc_StatusIcon(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_CustomIcon(const char* pszName, int iSize, void* pbuf);

	enum
	{
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 5,
		MAX_CUSTOMSPRITES = 6,
	};


	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	void EnableIcon(const char* pszIconName, const RGB24& color);
	void DisableIcon(const char* pszIconName);

	void EnableCustomIcon(int nIndex, char* pszIconName, const RGB24& color, const Rect& aRect);
	void DisableCustomIcon(int nIndex);

private:
	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		Rect rc;
		RGB24 color;
		int teamnumber; //Not actually used
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];
	icon_sprite_t m_CustomList[MAX_CUSTOMSPRITES];
};

//
//-----------------------------------------------------
//

class CHudFlagIcons : public CHudBase
{
public:
	bool Init();
	bool VidInit();
	void InitHUDData();
	bool Draw(float flTime);

	void EnableFlag(const char* pszFlagName, unsigned char team_idx, unsigned char red, unsigned char green, unsigned char blue, unsigned char score);
	void DisableFlag(const char* pszFlagName, unsigned char team_idx);

	bool MsgFunc_FlagIcon(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_FlagTimer(const char* pszName, int iSize, void* pbuf);

private:
	enum
	{
		MAX_FLAGSPRITENAME_LENGTH = 24,
		MAX_FLAGSPRITES = 4,
	};

	struct flag_sprite_t
	{
		char szSpriteName[MAX_FLAGSPRITENAME_LENGTH];
		HSPRITE spr;
		Rect rc;
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char score;
	};


	flag_sprite_t m_FlagList[MAX_FLAGSPRITES];
	bool m_bIsTimer;
	bool m_bTimerReset;
	float m_flTimeStart;
	float m_flTimeLimit;
};


class CHudPlayerBrowse : public CHudBase
{
public:
	bool Init();
	bool VidInit();
	void InitHUDData();
	bool Draw(float flTime);

	bool MsgFunc_PlyrBrowse(const char* pszName, int iSize, void* pbuf);

private:
	enum
	{
		MAX_POWERUPSPRITENAME_LENGTH = 15,
	};

	struct powerup_sprite_t
	{
		char szSpriteName[MAX_POWERUPSPRITENAME_LENGTH];
		HSPRITE spr;
		Rect rc;
		RGB24 color;
	};

	float m_flDelayFade;
	float m_flDelayFadeSprite;

	powerup_sprite_t m_PowerupSprite;

	char m_szLineBuffer[256];
	char m_szNewLineBuffer[256];

	int m_iTeamNum;
	int m_iNewTeamNum;
	int m_iHealth;
	int m_iArmor;
	bool m_fFriendly;
};

//
//-----------------------------------------------------
//
class CHudScoreboard : public CHudBase
{
public:
	bool Init();
	void InitHUDData();
	bool VidInit();
	bool Draw(float flTime);
	int DrawPlayers(int xoffset, float listslot, int nameoffset = 0, const char* team = nullptr); // returns the ypos where it finishes drawing
	void UserCmd_ShowScores();
	void UserCmd_HideScores();
	bool MsgFunc_ScoreInfo(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_TeamInfo(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_TeamScore(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_PlayerIcon(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_CTFScore(const char* pszName, int iSize, void* pbuf);
	void DeathMsg(int killer, int victim);



	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	bool m_iShowscoresHeld;

	struct cvar_s* cl_showpacketloss;

	void GetAllPlayersInfo();
};

class CHud
{
private:
	HUDLIST* m_pHudList;
	HSPRITE m_hsprLogo;
	int m_iLogo;
	client_sprite_t* m_pSpriteList;
	int m_iSpriteCount;
	int m_iSpriteCountAllRes;
	float m_flMouseSensitivity;
	int m_iConcussionEffect;

	bool m_NightVisionState = false;

public:
	HSPRITE m_hsprCursor;
	float m_flTime;		  // the current client time
	float m_fOldTime;	  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector m_vecOrigin;
	Vector m_vecAngles;
	int m_iKeyBits;
	int m_iHideHUDDisplay;
	int m_iFOV;
	int m_Teamplay;
	int m_iRes;
	cvar_t* m_pCvarStealMouse;
	cvar_t* m_pCvarDraw;
	cvar_t* m_pCvarCrosshair;

	RGB24 m_HudColor = RGB_HUD_COLOR;

	/**
	*	@brief The color to use for Hud items
	*/
	RGB24 m_HudItemColor = RGB_HUD_COLOR;

	int m_iFontHeight;
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, const RGB24& color);
	int DrawHudString(int x, int y, int iMaxX, const char* szString, const RGB24& color);
	int DrawHudStringReverse(int xpos, int ypos, int iMinX, const char* szString, const RGB24& color);
	int DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, const RGB24& color);
	int GetNumWidth(int iNumber, int iFlags);

	int GetHudNumberWidth(int number, int width, int flags);
	int DrawHudNumberReverse(int x, int y, int number, int flags, const RGB24& color);

	bool HasWeapon(int id) const
	{
		return (m_iWeaponBits & (1ULL << id)) != 0;
	}

	bool HasSuit() const
	{
		return HasWeapon(WEAPON_SUIT);
	}

	bool HasAnyWeapons() const
	{
		return (m_iWeaponBits & ~static_cast<std::uint64_t>(WEAPON_SUIT)) != 0;
	}

	const Vector& GetViewOrigin() const;
	const Vector& GetViewAngles() const;

private:
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE* m_rghSprites; /*[HUD_SPRITE_COUNT]*/ // the sprites loaded from hud.txt
	Rect* m_rgrcRects;							  /*[HUD_SPRITE_COUNT]*/
	char* m_rgszSpriteNames;					  /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

	struct cvar_s* default_fov;

public:
	HSPRITE GetSprite(int index)
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}

	Rect& GetSpriteRect(int index)
	{
		return m_rgrcRects[index];
	}


	int GetSpriteIndex(const char* SpriteName); // gets a sprite index, for use in the m_rghSprites[] array

	CHudAmmo m_Ammo;
	CHudHealth m_Health;
	CHudSpectator m_Spectator;
	CHudGeiger m_Geiger;
	CHudBattery m_Battery;
	CHudTrain m_Train;
	CHudFlashlight m_Flash;
	CHudMessage m_Message;
	CHudScoreboard m_Scoreboard;
	CHudStatusBar m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudSayText m_SayText;
	CHudMenu m_Menu;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;

	CHudFlagIcons m_FlagIcons;
	CHudPlayerBrowse m_PlayerBrowse;

	void Init();
	void VidInit();
	void Think();
	bool Redraw(float flTime, bool intermission);
	bool UpdateClientData(client_data_t* cdata, float time);

	CHud() : m_iSpriteCount(0), m_pHudList(nullptr) {}
	~CHud(); // destructor, frees allocated memory

	// user messages
	bool MsgFunc_Damage(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_GameMode(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_HudColor(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_Logo(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_ResetHUD(const char* pszName, int iSize, void* pbuf);
	void MsgFunc_InitHUD(const char* pszName, int iSize, void* pbuf);
	void MsgFunc_ViewMode(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_SetFOV(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_Concuss(const char* pszName, int iSize, void* pbuf);
	bool MsgFunc_Weapons(const char* pszName, int iSize, void* pbuf);

	// Screen information
	SCREENINFO m_scrinfo;

	std::uint64_t m_iWeaponBits;
	bool m_fPlayerDead;
	bool m_iIntermission;

	// sprite indexes
	int m_HUD_number_0;


	void AddHudElem(CHudBase* p);

	float GetSensitivity();

	bool IsNightVisionOn() const { return m_NightVisionState; }

	void SetNightVisionState(bool state);

	RGB24 GetHudItemColor(RGB24 color) const
	{
		if (IsNightVisionOn())
		{
			return m_HudItemColor;
		}
		else
		{
			return color;
		}
	}
};

extern CHud gHUD;

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
