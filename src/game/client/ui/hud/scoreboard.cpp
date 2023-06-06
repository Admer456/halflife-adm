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
// Scoreboard.cpp
//
// implementation of CHudScoreboard class
//

#include "hud.h"

#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"

#include "ctf/CTFDefs.h"

struct icon_sprite_t
{
	char szSpriteName[24];
	HSPRITE spr;
	Rect rc;
	RGB24 color;
	unsigned char bFlags;
};


icon_sprite_t g_PlayerSpriteList[MAX_PLAYERS_HUD + 1][6];

bool CHudScoreboard::Init()
{
	gHUD.AddHudElem(this);

	// Hook messages & commands here
	// RegisterClientCommand("+showscores", &CHudScoreboard::UserCmd_ShowScores, this);
	// RegisterClientCommand("-showscores", &CHudScoreboard::UserCmd_HideScores, this);

	g_ClientUserMessages.RegisterHandler("ScoreInfo", &CHudScoreboard::MsgFunc_ScoreInfo, this);
	g_ClientUserMessages.RegisterHandler("TeamScore", &CHudScoreboard::MsgFunc_TeamScore, this);
	g_ClientUserMessages.RegisterHandler("TeamInfo", &CHudScoreboard::MsgFunc_TeamInfo, this);
	g_ClientUserMessages.RegisterHandler("PlayerIcon", &CHudScoreboard::MsgFunc_PlayerIcon, this);
	g_ClientUserMessages.RegisterHandler("CTFScore", &CHudScoreboard::MsgFunc_CTFScore, this);

	InitHUDData();

	cl_showpacketloss = CVAR_CREATE("cl_showpacketloss", "0", FCVAR_ARCHIVE);

	return true;
}


bool CHudScoreboard::VidInit()
{
	// Load sprites here

	return true;
}

void CHudScoreboard::InitHUDData()
{
	memset(g_PlayerExtraInfo, 0, sizeof g_PlayerExtraInfo);
	m_iLastKilledBy = 0;
	m_fLastKillTime = 0;
	m_iPlayerNum = 0;
	m_iNumTeams = 0;
	memset(g_TeamInfo, 0, sizeof g_TeamInfo);

	m_iFlags &= ~HUD_ACTIVE; // starts out inactive

	m_iFlags |= HUD_INTERMISSION; // is always drawn during an intermission

	memset(g_PlayerSpriteList, 0, sizeof(g_PlayerSpriteList));
}

/* The scoreboard
We have a minimum width of 1-320 - we could have the field widths scale with it?
*/

// X positions
// relative to the side of the scoreboard
#define NAME_RANGE_MIN 20
#define NAME_RANGE_MAX 145
#define KILLS_RANGE_MIN 130
#define KILLS_RANGE_MAX 170
#define DIVIDER_POS 180
#define DEATHS_RANGE_MIN 185
#define DEATHS_RANGE_MAX 210
#define PING_RANGE_MIN 245
#define PING_RANGE_MAX 295
#define PL_RANGE_MIN 295
#define PL_RANGE_MAX 375

#define PL_CTF_RANGE_MIN 345
#define PL_CTF_RANGE_MAX 390

int SCOREBOARD_WIDTH = 320;


// Y positions
#define ROW_GAP 13
#define ROW_RANGE_MIN 15
#define ROW_RANGE_MAX (ScreenHeight - 50)

bool CHudScoreboard::Draw(float fTime)
{
	bool can_show_packetloss = false;
	int FAR_RIGHT;

	if (!m_iShowscoresHeld && gHUD.m_Health.m_iHealth > 0)
	{
		if (!gHUD.m_iIntermission || gHUD.m_Teamplay == 2)
		{
			return true;
		}
	}
	else
	{
		if (gHUD.m_iIntermission && gHUD.m_Teamplay == 2)
		{
			return true;
		}
	}

	GetAllPlayersInfo();

	//  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
	if (cl_showpacketloss && 0 != cl_showpacketloss->value && (ScreenWidth >= 400))
	{
		can_show_packetloss = true;
		SCOREBOARD_WIDTH = 400;
	}
	else
	{
		SCOREBOARD_WIDTH = 320;
	}

	// just sort the list on the fly
	// list is sorted first by frags, then by deaths
	float list_slot = 0;
	int xpos_rel = (ScreenWidth - SCOREBOARD_WIDTH) / 2;

	// print the heading line
	int ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);
	int xpos = NAME_RANGE_MIN + xpos_rel;

	const RGB24 color{255, 140, 0};

	if (0 == gHUD.m_Teamplay)
		gHUD.DrawHudString(xpos, ypos, NAME_RANGE_MAX + xpos_rel, "Player", color);
	else
		gHUD.DrawHudString(xpos, ypos, NAME_RANGE_MAX + xpos_rel, "Teams", color);

	gHUD.DrawHudStringReverse(KILLS_RANGE_MAX + xpos_rel, ypos, 0, "kills", color);
	gHUD.DrawHudString(DIVIDER_POS + xpos_rel, ypos, ScreenWidth, "/", color);

	if (gHUD.m_Teamplay == 2)
	{
		gHUD.DrawHudString(xpos_rel + 190, ypos, ScreenWidth, "deaths", color);
		gHUD.DrawHudString(xpos_rel + 240, ypos, ScreenWidth, "/", color);
		gHUD.DrawHudString(xpos_rel + 255, ypos, ScreenWidth, "scores", color);
		gHUD.DrawHudString(xpos_rel + 310, ypos, ScreenWidth, "latency", color);

		if (can_show_packetloss)
		{
			gHUD.DrawHudString(xpos_rel + 355, ypos, ScreenWidth, "pkt loss", color);
		}

		FAR_RIGHT = can_show_packetloss ? 390 : 345;
	}
	else
	{
		gHUD.DrawHudString(xpos_rel + 190, ypos, ScreenWidth, "deaths", color);
		gHUD.DrawHudString(xpos_rel + PING_RANGE_MAX - 35, ypos, ScreenWidth, "latency", color);

		if (can_show_packetloss)
		{
			gHUD.DrawHudString(xpos_rel + PL_RANGE_MAX - 35, ypos, ScreenWidth, "pkt loss", color);
		}

		FAR_RIGHT = can_show_packetloss ? PL_RANGE_MAX : PING_RANGE_MAX;
	}

	FAR_RIGHT += 5;

	list_slot += 1.2;
	ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);
	xpos = NAME_RANGE_MIN + xpos_rel;
	FillRGBA(xpos - 5, ypos, FAR_RIGHT, 1, color, 255); // draw the seperator line

	list_slot += 0.8;

	if (0 == gHUD.m_Teamplay)
	{
		// it's not teamplay,  so just draw a simple player list
		DrawPlayers(xpos_rel, list_slot);
		return true;
	}

	// clear out team scores
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		if (gHUD.m_Teamplay != 2)
		{
			if (!g_TeamInfo[i].scores_overriden)
				g_TeamInfo[i].frags = g_TeamInfo[i].deaths = 0;
		}

		g_TeamInfo[i].ping = g_TeamInfo[i].packetloss = 0;
	}

	// recalc the team scores, then draw them
	for (int i = 1; i < MAX_PLAYERS_HUD; i++)
	{
		if (g_PlayerInfoList[i].name == nullptr)
			continue; // empty player slot, skip

		if (g_PlayerExtraInfo[i].teamname[0] == 0)
			continue; // skip over players who are not in a team

		// find what team this player is in
		int j;
		for (j = 1; j <= m_iNumTeams; j++)
		{
			if (!stricmp(g_PlayerExtraInfo[i].teamname, g_TeamInfo[j].name))
				break;
		}
		if (j > m_iNumTeams) // player is not in a team, skip to the next guy
			continue;

		if (gHUD.m_Teamplay != 2 && !g_TeamInfo[j].scores_overriden)
		{
			g_TeamInfo[j].frags += g_PlayerExtraInfo[i].frags;
			g_TeamInfo[j].deaths += g_PlayerExtraInfo[i].deaths;
		}

		g_TeamInfo[j].ping += g_PlayerInfoList[i].ping;
		g_TeamInfo[j].packetloss += g_PlayerInfoList[i].packetloss;

		if (0 != g_PlayerInfoList[i].thisplayer)
			g_TeamInfo[j].ownteam = true;
		else
			g_TeamInfo[j].ownteam = false;
	}

	// find team ping/packetloss averages
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		g_TeamInfo[i].already_drawn = false;

		if (g_TeamInfo[i].players > 0)
		{
			g_TeamInfo[i].ping /= g_TeamInfo[i].players;	   // use the average ping of all the players in the team as the teams ping
			g_TeamInfo[i].packetloss /= g_TeamInfo[i].players; // use the average ping of all the players in the team as the teams ping
		}
	}

	// Draw the teams
	while (true)
	{
		int highest_frags = -99999;
		int lowest_deaths = 99999;
		int best_team = 0;

		for (int i = 1; i <= m_iNumTeams; i++)
		{
			if (g_TeamInfo[i].players <= 0)
				continue;

			if (!g_TeamInfo[i].already_drawn && g_TeamInfo[i].frags >= highest_frags)
			{
				if (g_TeamInfo[i].frags > highest_frags || g_TeamInfo[i].deaths < lowest_deaths)
				{
					best_team = i;
					lowest_deaths = g_TeamInfo[i].deaths;
					highest_frags = g_TeamInfo[i].frags;
				}
			}
		}

		// draw the best team on the scoreboard
		if (0 == best_team)
			break;

		// draw out the best team
		team_info_t* team_info = &g_TeamInfo[best_team];

		ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);

		// check we haven't drawn too far down
		if (ypos > ROW_RANGE_MAX) // don't draw to close to the lower border
			break;

		xpos = NAME_RANGE_MIN + xpos_rel;
		const RGB24 textColor{255, 225, 55}; // draw the stuff kinda yellowish

		if (team_info->ownteam) // if it is their team, draw the background different color
		{
			// overlay the background in blue,  then draw the score text over it
			FillRGBA(NAME_RANGE_MIN + xpos_rel - 5, ypos, FAR_RIGHT, ROW_GAP, {0, 0, 255}, 70);
		}

		static char buf[64];

		if (gHUD.m_Teamplay != 2)
		{

			// draw their name (left to right)
			gHUD.DrawHudString(xpos, ypos, NAME_RANGE_MAX + xpos_rel, team_info->name, textColor);

			// draw kills (right to left)
			xpos = KILLS_RANGE_MAX + xpos_rel;
			gHUD.DrawHudNumberString(xpos, ypos, KILLS_RANGE_MIN + xpos_rel, team_info->frags, textColor);

			// draw divider
			xpos = DIVIDER_POS + xpos_rel;
			gHUD.DrawHudString(xpos, ypos, xpos + 20, "/", textColor);

			// draw deaths
			xpos = DEATHS_RANGE_MAX + xpos_rel;
			gHUD.DrawHudNumberString(xpos, ypos, DEATHS_RANGE_MIN + xpos_rel, team_info->deaths, textColor);

			xpos = ((PING_RANGE_MAX - PING_RANGE_MIN) / 2) + PING_RANGE_MIN + xpos_rel + 25;
		}
		else
		{
			sprintf(buf, "%s Team Score:", team_info->name);
			gHUD.DrawHudString(xpos, ypos, xpos_rel + 205, buf, textColor);
			gHUD.DrawHudNumberString(xpos_rel + 275, ypos, xpos_rel + 250, team_info->frags, textColor);

			xpos = ((PING_RANGE_MAX - PING_RANGE_MIN) / 2) + PING_RANGE_MIN + xpos_rel + 25 + 50;
		}

		// draw ping
		// draw ping & packetloss
		sprintf(buf, "%d", team_info->ping);

		gHUD.DrawHudStringReverse(xpos, ypos, xpos - 50, buf, gHUD.m_HudColor);

		//  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
		if (can_show_packetloss)
		{
			xpos = ((PL_RANGE_MAX - PL_RANGE_MIN) / 2) + PL_RANGE_MIN + xpos_rel + 25 + 10;

			sprintf(buf, "  %d", team_info->packetloss);
			gHUD.DrawHudString(xpos, ypos, xpos + 50, buf, gHUD.m_HudColor);
		}

		team_info->already_drawn = true; // set the already_drawn to be true, so this team won't get drawn again
		list_slot++;

		// draw all the players that belong to this team, indented slightly
		list_slot = DrawPlayers(xpos_rel, list_slot, 10, team_info->name);
	}

	// draw all the players who are not in a team
	list_slot += 0.5;
	DrawPlayers(xpos_rel, list_slot, 0, "");

	return true;
}

// returns the ypos where it finishes drawing
int CHudScoreboard::DrawPlayers(int xpos_rel, float list_slot, int nameoffset, const char* team)
{
	bool can_show_packetloss = false;
	int FAR_RIGHT;

	//  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
	if (cl_showpacketloss && 0 != cl_showpacketloss->value && (ScreenWidth >= 400))
	{
		can_show_packetloss = true;
		SCOREBOARD_WIDTH = 400;
	}
	else
	{
		SCOREBOARD_WIDTH = 320;
	}

	if (gHUD.m_Teamplay == 2)
	{
		FAR_RIGHT = can_show_packetloss ? PL_CTF_RANGE_MAX : PL_CTF_RANGE_MIN;
	}
	else
	{
		FAR_RIGHT = can_show_packetloss ? PL_RANGE_MAX : PING_RANGE_MAX;
	}

	FAR_RIGHT += 5;

	// draw the players, in order,  and restricted to team if set
	while (true)
	{
		// Find the top ranking player
		int highest_frags = -99999;
		int lowest_deaths = 99999;
		int best_player = 0;

		for (int i = 1; i < MAX_PLAYERS_HUD; i++)
		{
			if (g_PlayerInfoList[i].name && (g_PlayerExtraInfo[i].frags + g_PlayerExtraInfo[i].flagcaptures) >= highest_frags)
			{
				if (!(team && stricmp(g_PlayerExtraInfo[i].teamname, team))) // make sure it is the specified team
				{
					extra_player_info_t* pl_info = &g_PlayerExtraInfo[i];
					if ((pl_info->frags + pl_info->flagcaptures) > highest_frags || pl_info->deaths < lowest_deaths)
					{
						best_player = i;
						lowest_deaths = pl_info->deaths;
						highest_frags = (pl_info->frags + pl_info->flagcaptures);
					}
				}
			}
		}

		if (0 == best_player)
			break;

		// draw out the best player
		hud_player_info_t* pl_info = &g_PlayerInfoList[best_player];

		int ypos = ROW_RANGE_MIN + (list_slot * ROW_GAP);

		// check we haven't drawn too far down
		if (ypos > ROW_RANGE_MAX) // don't draw to close to the lower border
			break;

		int xpos_icon = xpos_rel;

		for (int icon = 0; icon < 6; ++icon)
		{
			const auto& sprite = g_PlayerSpriteList[best_player][icon];

			if (0 != sprite.spr)
			{
				SPR_Set(sprite.spr, sprite.color);
				gEngfuncs.pfnSPR_DrawAdditive(0, xpos_icon, ypos, &sprite.rc);
				xpos_icon += sprite.rc.left - sprite.rc.right - 5;
			}
		}

		int xpos = NAME_RANGE_MIN + xpos_rel;
		RGB24 textColor{255, 255, 255};
		if (best_player == m_iLastKilledBy && 0 != m_fLastKillTime && m_fLastKillTime > gHUD.m_flTime)
		{
			if (0 != pl_info->thisplayer)
			{ // green is the suicide color? i wish this could do grey...
				FillRGBA(NAME_RANGE_MIN + xpos_rel - 5, ypos, FAR_RIGHT, ROW_GAP, {80, 155, 0}, 70);
			}
			else
			{ // Highlight the killers name - overlay the background in red,  then draw the score text over it
				FillRGBA(NAME_RANGE_MIN + xpos_rel - 5, ypos, FAR_RIGHT, ROW_GAP, {255, 0, 0}, ((float)15 * (float)(m_fLastKillTime - gHUD.m_flTime)));
			}
		}
		else if (0 != pl_info->thisplayer) // if it is their name, draw it a different color
		{
			// overlay the background in blue,  then draw the score text over it
			FillRGBA(NAME_RANGE_MIN + xpos_rel - 5, ypos, FAR_RIGHT, ROW_GAP, {0, 0, 255}, 70);
		}

		// draw their name (left to right)
		gHUD.DrawHudString(xpos + nameoffset, ypos, NAME_RANGE_MAX + xpos_rel, pl_info->name, textColor);

		// draw kills (right to left)
		xpos = KILLS_RANGE_MAX + xpos_rel;
		gHUD.DrawHudNumberString(xpos, ypos, KILLS_RANGE_MIN + xpos_rel, g_PlayerExtraInfo[best_player].frags, textColor);

		// draw divider
		xpos = DIVIDER_POS + xpos_rel;
		gHUD.DrawHudString(xpos, ypos, xpos + 20, "/", textColor);

		// draw deaths

		if (gHUD.m_Teamplay == 2)
		{
			gHUD.DrawHudNumberString(xpos_rel + 230, ypos, xpos_rel + DEATHS_RANGE_MIN, g_PlayerExtraInfo[best_player].deaths, textColor);
			gHUD.DrawHudString(xpos_rel + 240, ypos, xpos_rel + 260, "/", textColor);
			gHUD.DrawHudNumberString(xpos_rel + 275, ypos, xpos_rel + 250, g_PlayerExtraInfo[best_player].flagcaptures, textColor);
		}
		else
		{
			gHUD.DrawHudNumberString(xpos_rel + DEATHS_RANGE_MAX, ypos, xpos_rel + DEATHS_RANGE_MIN, g_PlayerExtraInfo[best_player].deaths, textColor);
		}

		// draw ping & packetloss
		static char buf[64];
		sprintf(buf, "%d", g_PlayerInfoList[best_player].ping);
		xpos = ((PING_RANGE_MAX - PING_RANGE_MIN) / 2) + PING_RANGE_MIN + xpos_rel + 25;

		if (gHUD.m_Teamplay == 2)
		{
			xpos = xpos_rel + 345;
		}

		gHUD.DrawHudStringReverse(xpos, ypos, xpos - 50, buf, textColor);

		//  Packetloss removed on Kelly 'shipping nazi' Bailey's orders
		if (can_show_packetloss)
		{
			if (g_PlayerInfoList[best_player].packetloss >= 63)
			{
				textColor = RGB_REDISH;
				sprintf(buf, " !!!!");
			}
			else
			{
				sprintf(buf, "  %d", g_PlayerInfoList[best_player].packetloss);
			}

			xpos = ((PL_RANGE_MAX - PL_RANGE_MIN) / 2) + PL_RANGE_MIN + xpos_rel + 25 + 10;

			if (gHUD.m_Teamplay == 2)
			{
				xpos = xpos_rel + 385;
			}

			gHUD.DrawHudString(xpos, ypos, xpos + 50, buf, textColor);
		}

		pl_info->name = nullptr; // set the name to be nullptr, so this client won't get drawn again
		list_slot++;
	}

	return list_slot;
}


void CHudScoreboard::GetAllPlayersInfo()
{
	for (int i = 1; i < MAX_PLAYERS_HUD; i++)
	{
		gEngfuncs.pfnGetPlayerInfo(i, &g_PlayerInfoList[i]);

		if (0 != g_PlayerInfoList[i].thisplayer)
			m_iPlayerNum = i; // !!!HACK: this should be initialized elsewhere... maybe gotten from the engine
	}
}

void CHudScoreboard::MsgFunc_ScoreInfo(const char* pszName, BufferReader& reader)
{
	m_iFlags |= HUD_ACTIVE;

	short cl = reader.ReadByte();
	short frags = reader.ReadShort();
	short deaths = reader.ReadShort();
	short playerclass = reader.ReadShort();
	short teamnumber = reader.ReadShort();

	if (cl > 0 && cl <= MAX_PLAYERS_HUD)
	{
		g_PlayerExtraInfo[cl].frags = frags;
		g_PlayerExtraInfo[cl].deaths = deaths;
		g_PlayerExtraInfo[cl].playerclass = playerclass;
		g_PlayerExtraInfo[cl].teamnumber = teamnumber;

		gViewPort->UpdateOnPlayerInfo();
	}
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
void CHudScoreboard::MsgFunc_TeamInfo(const char* pszName, BufferReader& reader)
{
	short cl = reader.ReadByte();

	if (cl > 0 && cl <= MAX_PLAYERS_HUD)
	{ // set the players team
		strncpy(g_PlayerExtraInfo[cl].teamname, reader.ReadString(), MAX_TEAM_NAME);

		if (gHUD.m_Teamplay == 2)
		{
			g_PlayerExtraInfo[cl].teamid = reader.ReadByte();
		}
	}

	if (gViewPort && gViewPort->m_pScoreBoard)
	{
		m_iNumTeams = gViewPort->m_pScoreBoard->RebuildTeams();
		return;
	}

	// rebuild the list of teams

	// clear out player counts from teams
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		g_TeamInfo[i].players = 0;
	}

	// rebuild the team list
	GetAllPlayersInfo();
	m_iNumTeams = 0;
	for (int i = 1; i < MAX_PLAYERS_HUD; i++)
	{
		if (g_PlayerInfoList[i].name == nullptr)
			continue;

		if (g_PlayerExtraInfo[i].teamname[0] == 0)
			continue; // skip over players who are not in a team

		// is this player in an existing team?
		int j;
		for (j = 1; j <= m_iNumTeams; j++)
		{
			if (g_TeamInfo[j].name[0] == '\0')
				break;

			if (!stricmp(g_PlayerExtraInfo[i].teamname, g_TeamInfo[j].name))
				break;
		}

		if (j > m_iNumTeams)
		{ // they aren't in a listed team, so make a new one
			// search through for an empty team slot
			for (j = 1; j <= m_iNumTeams; j++)
			{
				if (g_TeamInfo[j].name[0] == '\0')
					break;
			}
			m_iNumTeams = std::max(j, m_iNumTeams);

			strncpy(g_TeamInfo[j].name, g_PlayerExtraInfo[i].teamname, MAX_TEAM_NAME);
			g_TeamInfo[j].players = 0;
		}

		g_TeamInfo[j].players++;
	}

	// clear out any empty teams
	for (int i = 1; i <= m_iNumTeams; i++)
	{
		if (g_TeamInfo[i].players < 1)
			memset(&g_TeamInfo[i], 0, sizeof(team_info_t));
	}
}

// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths
// if this message is never received, then scores will simply be the combined totals of the players.
void CHudScoreboard::MsgFunc_TeamScore(const char* pszName, BufferReader& reader)
{
	char* TeamName = reader.ReadString();

	// find the team matching the name
	int i;
	for (i = 1; i <= m_iNumTeams; i++)
	{
		if (!stricmp(TeamName, g_TeamInfo[i].name))
			break;
	}
	if (i > m_iNumTeams)
		return;

	// use this new score data instead of combined player scores
	g_TeamInfo[i].scores_overriden = true;
	g_TeamInfo[i].frags = reader.ReadShort();
	g_TeamInfo[i].deaths = reader.ReadShort();
}

void CHudScoreboard::MsgFunc_PlayerIcon(const char* pszName, BufferReader& reader)
{
	const short playerIndex = reader.ReadByte();
	const bool isActive = 0 != reader.ReadByte();
	const int iconIndex = reader.ReadByte();
	const unsigned char itemId = reader.ReadByte();

	if (playerIndex > MAX_PLAYERS_HUD)
		return;

	if (!isActive)
	{
		for (int i = 0; i < 6; ++i)
		{
			if ((itemId & g_PlayerSpriteList[playerIndex][i].bFlags) != 0)
			{
				memset(&g_PlayerSpriteList[playerIndex][i], 0, sizeof(g_PlayerSpriteList[playerIndex][i]));
			}
		}
		return;
	}

	if (0 == itemId)
	{
		memset(&g_PlayerSpriteList[playerIndex][iconIndex], 0, sizeof(g_PlayerSpriteList[playerIndex][iconIndex]));
		return;
	}

	for (int i = 0, id = CTFItem::BlackMesaFlag; i < 2; ++i, id <<= 1)
	{
		if ((itemId & id) == 0)
		{
			continue;
		}

		const int spriteIndex = gHUD.GetSpriteIndex("score_flag");

		auto& sprite = g_PlayerSpriteList[playerIndex][0];

		sprite.spr = gHUD.GetSprite(spriteIndex);
		sprite.rc = gHUD.GetSpriteRect(spriteIndex);
		sprite.bFlags = itemId;
		sprite.color = id == CTFItem::BlackMesaFlag ? RGB_YELLOWISH : RGB_GREENISH;

		strcpy(sprite.szSpriteName, "score_flag");

		return;
	}

	for (int i = 1; i < 6; ++i)
	{
		if ((itemId & g_PlayerSpriteList[playerIndex][i].bFlags) != 0)
		{
			return;
		}
	}

	// Find an empty sprite slot
	for (int i = 1; i < 6; ++i)
	{
		auto& sprite = g_PlayerSpriteList[playerIndex][i];

		if (0 == sprite.spr)
		{
			if ((itemId & CTFItem::LongJump) != 0)
			{
				const int spriteIndex = gHUD.GetSpriteIndex("score_ctfljump");

				sprite.spr = gHUD.GetSprite(spriteIndex);
				sprite.rc = gHUD.GetSpriteRect(spriteIndex);
				sprite.bFlags = itemId;
				sprite.color = {255, 160, 0};

				strcpy(sprite.szSpriteName, "score_ctfljump");
			}
			else if ((itemId & CTFItem::PortableHEV) != 0)
			{
				const int spriteIndex = gHUD.GetSpriteIndex("score_ctfphev");

				sprite.spr = gHUD.GetSprite(spriteIndex);
				sprite.rc = gHUD.GetSpriteRect(spriteIndex);
				sprite.bFlags = itemId;
				sprite.color = {128, 160, 255};

				strcpy(sprite.szSpriteName, "score_ctfphev");
			}
			else if ((itemId & CTFItem::Backpack) != 0)
			{
				const int spriteIndex = gHUD.GetSpriteIndex("score_ctfbpack");

				sprite.spr = gHUD.GetSprite(spriteIndex);
				sprite.rc = gHUD.GetSpriteRect(spriteIndex);
				sprite.bFlags = itemId;
				sprite.color = {255, 255, 0};

				strcpy(sprite.szSpriteName, "score_ctfbpack");
			}
			else if ((itemId & CTFItem::Acceleration) != 0)
			{
				const int spriteIndex = gHUD.GetSpriteIndex("score_ctfaccel");

				sprite.spr = gHUD.GetSprite(spriteIndex);
				sprite.rc = gHUD.GetSpriteRect(spriteIndex);
				sprite.bFlags = itemId;
				sprite.color = {255, 0, 0};

				strcpy(sprite.szSpriteName, "score_ctfaccel");
			}
			else if ((itemId & CTFItem::Regeneration) != 0)
			{
				const int spriteIndex = gHUD.GetSpriteIndex("score_ctfregen");

				sprite.spr = gHUD.GetSprite(spriteIndex);
				sprite.rc = gHUD.GetSpriteRect(spriteIndex);
				sprite.bFlags = itemId;
				sprite.color = {0, 255, 0};

				strcpy(sprite.szSpriteName, "score_ctfregen");
			}
			break;
		}
	}

	m_iFlags |= HUD_ACTIVE;
}

void CHudScoreboard::MsgFunc_CTFScore(const char* pszName, BufferReader& reader)
{
	const int playerIndex = reader.ReadByte();
	const int score = reader.ReadByte();

	if (playerIndex >= 1 && playerIndex <= MAX_PLAYERS_HUD)
	{
		g_PlayerExtraInfo[playerIndex].flagcaptures = score;
	}
}

void CHudScoreboard::DeathMsg(int killer, int victim)
{
	// if we were the one killed,  or the world killed us, set the scoreboard to indicate suicide
	if (victim == m_iPlayerNum || killer == 0)
	{
		m_iLastKilledBy = 0 != killer ? killer : m_iPlayerNum;
		m_fLastKillTime = gHUD.m_flTime + 10; // display who we were killed by for 10 seconds

		if (killer == m_iPlayerNum)
			m_iLastKilledBy = m_iPlayerNum;
	}
}



void CHudScoreboard::UserCmd_ShowScores()
{
	m_iShowscoresHeld = true;
}

void CHudScoreboard::UserCmd_HideScores()
{
	m_iShowscoresHeld = false;
}
