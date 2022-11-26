/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#pragma once

enum pipewrench_e
{
	PIPEWRENCH_IDLE1 = 0,
	PIPEWRENCH_IDLE2,
	PIPEWRENCH_IDLE3,
	PIPEWRENCH_DRAW,
	PIPEWRENCH_HOLSTER,
	PIPEWRENCH_ATTACK1HIT,
	PIPEWRENCH_ATTACK1MISS,
	PIPEWRENCH_ATTACK2HIT,
	PIPEWRENCH_ATTACK2MISS,
	PIPEWRENCH_ATTACK3HIT,
	PIPEWRENCH_ATTACK3MISS,
	PIPEWRENCH_BIG_SWING_START,
	PIPEWRENCH_BIG_SWING_HIT,
	PIPEWRENCH_BIG_SWING_MISS,
	PIPEWRENCH_BIG_SWING_IDLE
};

class CPipewrench : public CBasePlayerWeapon
{
private:
	enum SwingMode
	{
		SWING_NONE = 0,
		SWING_START_BIG,
		SWING_DOING_BIG,
	};

public:
	using BaseClass = CBasePlayerWeapon;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void OnCreate() override;
	void Spawn() override;
	void Precache() override;
	void EXPORT SwingAgain();
	void EXPORT Smack();

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Swing(const bool bFirst);
	void EXPORT BigSwing();
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;

	void GetWeaponData(weapon_data_t& data) override;

	void SetWeaponData(const weapon_data_t& data) override;

	int iItemSlot() override;

	bool GetItemInfo(ItemInfo* p) override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	float m_flBigSwingStart;
	int m_iSwingMode;
	int m_iSwing;
	TraceResult m_trHit;

private:
	unsigned short m_usPipewrench;
};
