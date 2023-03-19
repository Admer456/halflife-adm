
#pragma once

// ================================
// EnvModel
//
// Displays a model.
// ================================
class EnvModel : public CBaseAnimating
{
public:
	// Hack: The SF_Off spawnflag is used as a regular flag
	static constexpr int SF_Off = 1 << 0;
	static constexpr int SF_DropToFloor = 1 << 1;
	static constexpr int SF_Solid = 1 << 2;

	int ObjectCaps() override { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	
	void Spawn() override;
	void Precache() override;

	bool KeyValue(KeyValueData* pkvd) override;
	//STATE GetState() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	void SetSequence();
	inline bool IsOn() const
	{
		return !(pev->spawnflags & SF_Off);
	}

private:
	string_t m_iszSequence_On;
	string_t m_iszSequence_Off;
	bool m_iAction_On{false};
	bool m_iAction_Off{false};
};
