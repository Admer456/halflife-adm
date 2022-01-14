
#pragma once

// ================================
// UtilPrint
//
// Binds two entities together by origin, angles, or both
// ================================
class UtilMoveWith final : public CPointEntity
{
public:
	static constexpr int SF_StartOn = 1 << 0;
	static constexpr int SF_NoPosition = 1 << 1;
	static constexpr int SF_NoAngles = 1 << 2;

	void Spawn() override;
	void Activate() override;

	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value) override;
	void Think() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	bool Validate() const;

private:
	string_t m_ParentTarget{iStringNull};
	string_t m_ChildTarget{iStringNull};

	CBaseEntity* m_ParentEntity{nullptr};
	CBaseEntity* m_ChildEntity{nullptr};

	bool m_Enabled{false};
};
