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

#include <algorithm>

#include "cbase.h"
#include "WeaponDataSystem.h"

bool WeaponDataSystem::Initialize()
{
	g_NetworkData.RegisterHandler("Weapons", this);
	return true;
}

void WeaponDataSystem::HandleNetworkDataBlock(NetworkDataBlock& block)
{
	if (block.Mode == NetworkDataMode::Serialize)
	{
		block.Data = json::array();

		// Send all valid weapon info.
		for (const auto& info : m_WeaponInfos)
		{
			if (info.Id == WEAPON_NONE)
			{
				continue;
			}

			json data = json::object();

			data.emplace("Id", info.Id);
			data.emplace("Name", info.Name.c_str());

			data.emplace("Slot", info.Slot);
			data.emplace("Position", info.Position);
			data.emplace("Weight", info.Weight);
			data.emplace("Flags", info.Flags);

			data.emplace("Ammo1", info.AmmoType1.c_str());
			data.emplace("MagazineSize1", info.MagazineSize1);

			data.emplace("Ammo2", info.AmmoType2.c_str());

			block.Data.push_back(std::move(data));
		}
	}
	else
	{
		Clear();

		for (const auto& data : block.Data)
		{
			WeaponInfo info;

			info.Id = data.value("Id", WEAPON_NONE);
			info.Name = data.value("Name", "").c_str();

			info.Slot = data.value("Slot", 0);
			info.Position = data.value("Position", 0);
			info.Weight = data.value("Weight", 0);
			info.Flags = data.value("Flags", 0);

			info.AmmoType1 = data.value("Ammo1", "").c_str();
			info.MagazineSize1 = data.value("MagazineSize1", 0);

			info.AmmoType2 = data.value("Ammo2", "").c_str();

			if (Register(std::move(info)) == -1)
			{
				block.ErrorMessage = "Invalid weapon info received from server";
				return;
			}
		}
	}
}

int WeaponDataSystem::IndexOf(std::string_view name) const
{
	for (std::size_t i = 0; i < m_WeaponInfos.size(); ++i)
	{
		const auto& type = m_WeaponInfos[i];

		if (type.Name.compare(0, type.Name.size(), name.data(), name.size()) == 0)
		{
			return static_cast<int>(i + 1);
		}
	}

	return -1;
}

const WeaponInfo* WeaponDataSystem::GetByIndex(int index) const
{
	--index;

	if (index >= 0 && static_cast<std::size_t>(index) < m_WeaponInfos.size())
	{
		return &m_WeaponInfos[index];
	}

	return nullptr;
}

const WeaponInfo* WeaponDataSystem::GetByName(std::string_view name) const
{
	return GetByIndex(IndexOf(name));
}

void WeaponDataSystem::Clear()
{
	m_WeaponInfos = {};
}

int WeaponDataSystem::Register(WeaponInfo&& info)
{
	if (info.Id == WEAPON_NONE || info.Id <= 0 || static_cast<std::size_t>(info.Id) >= MAX_WEAPONS)
	{
		assert(!"Invalid weapon id");
		CBasePlayerWeapon::WeaponsLogger->error("Invalid weapon id");
		return -1;
	}

	const eastl::string_view name{info.Name};

	if (name.empty() || std::find_if_not(name.begin(), name.end(), [](auto c)
							{ return std::isspace(c) != 0; }) == name.end())
	{
		assert(!"Invalid weapon name");
		CBasePlayerWeapon::WeaponsLogger->error("Invalid weapon name");
		return -1;
	}

	if (info.Slot < 0 || info.Slot >= MAX_WEAPON_SLOTS || info.Position < 0)
	{
		assert(!"Invalid weapon hud slot or position");
		CBasePlayerWeapon::WeaponsLogger->error("Invalid weapon hud slot or position");
		return -1;
	}

	if (info.MagazineSize1 < -1)
	{
		assert(!"Invalid weapon max magazine 1 value");
		CBasePlayerWeapon::WeaponsLogger->error("Invalid weapon max magazine 1 value");
		return -1;
	}

	auto& dest = m_WeaponInfos[info.Id - 1];

	if (dest.Id != WEAPON_NONE)
	{
		assert(!"Duplicate weapon registration or id");
		CBasePlayerWeapon::WeaponsLogger->error("Duplicate weapon registration or id");
		return -1;
	}

	dest = std::move(info);

	return info.Id;
}
