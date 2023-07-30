
#include "cbase.h"
#include "EnvModel.h"

BEGIN_DATAMAP(EnvModel)
	DEFINE_FIELD(m_iszSequence_On, FIELD_STRING),
	DEFINE_FIELD(m_iszSequence_Off, FIELD_STRING),
	DEFINE_FIELD(m_iAction_On, FIELD_BOOLEAN),
	DEFINE_FIELD(m_iAction_Off, FIELD_BOOLEAN),
END_DATAMAP();

LINK_ENTITY_TO_CLASS(env_model, EnvModel);

// ================================
// EnvModel::Spawn
// ================================
void EnvModel::Spawn()
{
	Precache();
	SetModel(STRING(pev->model));
	SetOrigin(pev->origin);

	if (pev->spawnflags & SF_Solid)
	{
		pev->solid = SOLID_SLIDEBOX;
		SetSize(Vector(-10, -10, -10), Vector(10, 10, 10));
	}

	if (pev->spawnflags & SF_DropToFloor)
	{
		pev->origin.z += 1;
		DROP_TO_FLOOR(ENT(pev));
	}

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	SetSequence();

	pev->nextthink = gpGlobals->time + 0.1f;
}

// ================================
// EnvModel::Precache
// ================================
void EnvModel::Precache()
{
	PrecacheModel(STRING(pev->model));
}

// ================================
// EnvModel::KeyValue
// ================================
bool EnvModel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
	{
		m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
	{
		m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_On"))
	{
		m_iAction_On = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAction_Off"))
	{
		m_iAction_Off = atoi(pkvd->szValue);
		return true;
	}
	return CBaseAnimating::KeyValue(pkvd);
}

// STATE seems to be a SoHL leftover, I'll see if I can make use of this someday

//STATE EnvModel::GetState()
//{
//	if (pev->spawnflags & SF_ENVMODEL_OFF)
//		return STATE_OFF;
//	else
//		return STATE_ON;
//}

// ================================
// EnvModel::Use
// ================================
void EnvModel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, IsOn()))
	{
		if (!IsOn())
			pev->spawnflags &= ~SF_Off;
		else
			pev->spawnflags |= SF_Off;

		SetSequence();
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

// ================================
// EnvModel::Think
// ================================
void EnvModel::Think()
{
	int iTemp;
	StudioFrameAdvance(); // set m_fSequenceFinished if necessary

	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		if (!IsOn())
			iTemp = m_iAction_Off;
		else
			iTemp = m_iAction_On;

		switch (iTemp)
		{
			//		case 1: // loop
			//			pev->animtime = gpGlobals->time;
			//			m_fSequenceFinished = false;
			//			m_flLastEventCheck = gpGlobals->time;
			//			pev->frame = 0;
			//			break;
		case 2: // change state
			if (!IsOn())
				pev->spawnflags &= ~SF_Off;
			else
				pev->spawnflags |= SF_Off;
			SetSequence();
			break;
		default: // remain frozen
			return;
		}
	}
	pev->nextthink = gpGlobals->time + 0.1f;
}

// ================================
// EnvModel::SetSequence
// ================================
void EnvModel::SetSequence()
{
	string_t iszSeq;

	if (!IsOn())
		iszSeq = m_iszSequence_Off;
	else
		iszSeq = m_iszSequence_On;

	if (iszSeq == string_t::Null)
		return;
	pev->sequence = LookupSequence(STRING(iszSeq));

	if (pev->sequence == -1)
	{
		if (pev->targetname != string_t::Null)
			Logger->error("env_model %s: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));
		else
			Logger->error("env_model: unknown sequence \"%s\"\n", STRING(pev->targetname), STRING(iszSeq));

		pev->sequence = 0;
	}

	pev->frame = 0;
	ResetSequenceInfo();

	if (!IsOn())
	{
		m_fSequenceLoops = m_iAction_Off;
	}
	else
	{
		m_fSequenceLoops = m_iAction_On;
	}
}
