/*
 * Copyright (C) 2008-2019 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "PassiveAI.h"
#include "Player.h"
#include "MoveSpline.h"
#include "blackwing_descent.h"

enum Spells
{
    // Atramedes
    SPELL_ROARING_BREATH                    = 81573,
    SPELL_DEVASTATION_TRIGGER               = 78898,
    SPELL_SOUND_BAR                         = 89683,
    SPELL_DEVASTATION                       = 78868,
    SPELL_SONAR_PULSE                       = 77672,
    SPELL_MODULATION                        = 77612,
    SPELL_SEARING_FLAME                     = 77840,
    SPELL_SONIC_BREATH                      = 78075,
    SPELL_SONIC_BREATH_CAST                 = 78098,
    SPELL_TAKE_OFF_ANIM_KIT                 = 86915,
    SPELL_SONAR_PULSE_TRIGGER               = 92519,
    SPELL_SONAR_BOMB                        = 92765,
    SPELL_ROARING_FLAME_BREATH              = 78207,
    SPELL_RESONATING_CLASH_AIR_CLEAR        = 78958,

    // Sonar Pulse
    SPELL_SONAR_PULSE_PERIODIC_TRIGGER      = 77674,

    // Tracking Flames & Reverberating Flame
    SPELL_TRACKING                          = 78092,

    // Reverberating Flame
    SPELL_ROARING_FLAME_BREATH_REVERSE_CAST = 78230,
    SPELL_ROARING_FLAME_SUMMON              = 78272,
    SPELL_AGGRO_CREATOR                     = 63709,
    SPELL_SONIC_FLAMES_FLIGHT               = 78945,

    // Player
    SPELL_RESONATING_CLASH_GROUND           = 77611,
    SPELL_RESONATING_CLASH_AIR              = 78168,
    SPELL_RESONATING_CLASH_RESET_ENERGY     = 77709,
    SPELL_NOISY                             = 78897
};

enum Texts
{
    // Atramedes
    SAY_AGGRO                   = 0,
    SAY_ANNOUNCE_SEARING_FLAME  = 1,
    SAY_SEARING_FLAME           = 2,
    SAY_FLIGHT_PHASE            = 3,
};

enum Sounds
{
    SOUND_ID_ATRAMEDES_VERTIGO = 20828
};

enum Events
{
    // Atramedes
    EVENT_ROARING_BREATH = 1,
    EVENT_CLOSE_DOOR,
    EVENT_FLY_TO_INTRO_LAND_POSITION,
    EVENT_SONAR_PULSE,
    EVENT_MODULATION,
    EVENT_SEARING_FLAME,
    EVENT_SONIC_BREATH,
    EVENT_LIFTOFF,
    EVENT_RESUME_REVERBERATING_FLAME_MOVEMENT,
    EVENT_LAND,
    EVENT_REENGAGE_PLAYERS
};

enum Actions
{
    // Atramedes
    ACTION_START_INTRO              = 0,
    ACTION_HALT_REVERBERATING_FLAME = 1
};

enum MovePoints
{
    POINT_NONE = 0,
    POINT_CAST_ROARING_BREATH,
    POINT_PREPARE_LAND_INTRO,
    POINT_LAND_INTRO,
    POINT_LIFTOFF,
    POINT_LAND
};

enum Phases
{
    PHASE_INTRO     = 0,
    PHASE_GROUND    = 1,
    PHASE_AIR       = 2
};

enum Data
{
    // Setter
    DATA_LAST_ANCIENT_DWARVEN_SHIELD    = 0,
    DATA_ADD_NOISY_PLAYER               = 1,
    DATA_REMOVE_NOISY_PLAYER            = 2,
    DATA_LAST_SHIELD_USER               = 3,

    // Getter
    DATA_IS_IN_AIR                      = 0,
    DATA_HAS_NOISY_PLAYER               = 1
};

Position const IntroFlightPosition1 = { 249.432f, -223.616f, 98.6447f };
Position const IntroFlightPosition2 = { 214.531f, -223.918f, 93.4661f };
Position const IntroLandingPosition = { 214.531f, -223.918f, 74.7668f };
Position const LiftoffPosition      = { 130.655f, -226.637f, 113.21f  };
Position const LandPosition         = { 124.575f, -224.797f, 75.4534f };

struct boss_atramedes : public BossAI
{
    boss_atramedes(Creature* creature) : BossAI(creature, DATA_ATRAMEDES)
    {
        Initialize();
    }

    ~boss_atramedes()
    {
        delete _lastAncientDwarvenShieldGUIDs;
    }

    void Initialize()
    {
        _allowVertigoCast = false;
        _lastAncientDwarvenShieldGUIDs = new std::queue<ObjectGuid>();
    }

    void Reset() override
    {
        _Reset();
        events.SetPhase(PHASE_INTRO);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _JustEngagedWith();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
        Talk(SAY_AGGRO);
        DoCastSelf(SPELL_DEVASTATION_TRIGGER);
        DoCastSelf(SPELL_SOUND_BAR);
        events.SetPhase(PHASE_GROUND);
        events.ScheduleEvent(EVENT_CLOSE_DOOR, 5s, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_SONAR_PULSE, 14s + 500ms, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_MODULATION, 13s, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_SEARING_FLAME, 46s, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_SONIC_BREATH, 24s, 0, PHASE_GROUND);
        events.ScheduleEvent(EVENT_LIFTOFF, 1min + 31s, 0, PHASE_GROUND);

        _allowVertigoCast = !me->HasByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        _EnterEvadeMode();
        summons.DespawnAll();
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        instance->SetBossState(DATA_ATRAMEDES, FAIL);
        CleanupEncounter();
        me->DespawnOrUnsummon();
    }

    void JustSummoned(Creature* summon) override
    {
        summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_SONAR_PULSE:
                summon->m_Events.AddEventAtOffset([summon]()
                {
                    Unit* summoner = summon->ToTempSummon()->GetSummoner();
                    if (!summoner)
                        return;

                    summon->CastSpell(summon, SPELL_SONAR_PULSE_PERIODIC_TRIGGER);
                    summon->m_Events.AddEventAtOffset([summon, summoner]()
                    {
                        Position pos = summon->GetPosition();
                        pos.m_positionZ += 2.0f; // avoid hickups due to uneven terrain
                        float angle = summon->GetAngle(summoner) - summon->GetOrientation();

                        summon->MovePositionToFirstCollision(pos, 100.0f, angle);
                        summon->GetMotionMaster()->MovePoint(POINT_NONE, pos, false);
                        if (uint32 duration = summon->movespline->Duration())
                            summon->DespawnOrUnsummon(duration);
                    }, 800ms);
                }, 400ms);
                break;
            case NPC_TRACKING_FLAMES:
                if (Unit* summoner = summon->ToTempSummon()->GetSummoner())
                {
                    summon->CastSpell(summoner, SPELL_TRACKING);
                    summon->GetMotionMaster()->MoveFollow(summoner, 0.0f, ChaseAngle(0.0f, 0.0f));
                    me->SetFacingToObject(summon);
                    DoCast(summon, SPELL_SONIC_BREATH_CAST);
                }
                break;
            case NPC_SONAR_PULSE_BOMB:
                DoCast(summon, SPELL_SONAR_BOMB, true);
                break;
            case NPC_REVERBERATING_FLAME:
                if (Unit* summoner = summon->ToTempSummon()->GetSummoner())
                {
                    summon->CastSpell(summon, SPELL_ROARING_FLAME_BREATH_REVERSE_CAST);
                    summon->CastSpell(summon, SPELL_AGGRO_CREATOR);
                    summon->CastSpell(summoner, SPELL_TRACKING);
                    summon->GetMotionMaster()->MoveFollow(summoner, 0.0f, ChaseAngle(0.0f, 0.0f));
                    _reverberatingFlameGUID = summon->GetGUID();
                }
                break;
            default:
                break;
        }
    }

    uint32 GetData(uint32 type) const override
    {
        switch (type)
        {
            case DATA_IS_IN_AIR:
                return (uint8(events.IsInPhase(PHASE_AIR)));
            case DATA_HAS_NOISY_PLAYER:
                return (uint8(!_noisyPlayerGUIDs.empty()));
        }

        return 0;
    }

    void SetGUID(ObjectGuid const& guid, int32 type) override
    {
        switch (type)
        {
            case DATA_LAST_ANCIENT_DWARVEN_SHIELD:
                _lastAncientDwarvenShieldGUIDs->push(guid);
                break;
            case DATA_ADD_NOISY_PLAYER:
                _noisyPlayerGUIDs.insert(guid);
                break;
            case DATA_REMOVE_NOISY_PLAYER:
                _noisyPlayerGUIDs.erase(guid);
                break;
            case DATA_LAST_SHIELD_USER:
                _lastShieldUserGUID = guid;
                break;
            default:
                break;
        }
    }

    ObjectGuid GetGUID(int32 type) const override
    {
        switch (type)
        {
            case DATA_LAST_ANCIENT_DWARVEN_SHIELD:
            {
                if (_lastAncientDwarvenShieldGUIDs->empty())
                    return ObjectGuid::Empty;

                ObjectGuid guid = _lastAncientDwarvenShieldGUIDs->front();
                _lastAncientDwarvenShieldGUIDs->pop();
                return guid;
            }
        }

        return ObjectGuid::Empty;
    }

    void MovementInform(uint32 motionType, uint32 pointId) override
    {
        if (motionType != POINT_MOTION_TYPE && motionType != EFFECT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_CAST_ROARING_BREATH:
                events.ScheduleEvent(EVENT_ROARING_BREATH, 2s);
                break;
            case POINT_PREPARE_LAND_INTRO:
                me->GetMotionMaster()->MoveLand(POINT_LAND_INTRO, IntroLandingPosition);
                break;
            case POINT_LAND_INTRO:
                me->SetDisableGravity(false);
                me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                me->SetHover(false);
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                me->SetReactState(REACT_AGGRESSIVE);
                _allowVertigoCast = true;
                break;
            case POINT_LIFTOFF:
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                me->SetDisableGravity(true);
                me->SendSetPlayHoverAnim(true);
                Talk(SAY_FLIGHT_PHASE);
                DoCastSelf(SPELL_SONAR_PULSE_TRIGGER);
                DoCastSelf(SPELL_ROARING_FLAME_BREATH);
                events.ScheduleEvent(EVENT_LAND, 31s, 0, PHASE_AIR);
                break;
            case POINT_LAND:
                me->RemoveByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                me->SetDisableGravity(false);
                me->SendSetPlayHoverAnim(false);
                events.SetPhase(PHASE_GROUND);
                events.ScheduleEvent(EVENT_REENGAGE_PLAYERS, 800ms, 0, PHASE_GROUND);
                events.ScheduleEvent(EVENT_SONAR_PULSE, 14s, 0, PHASE_GROUND);
                events.ScheduleEvent(EVENT_MODULATION, 13s, 0, PHASE_GROUND);
                events.ScheduleEvent(EVENT_SEARING_FLAME, 51s, 0, PHASE_GROUND);
                events.ScheduleEvent(EVENT_SONIC_BREATH, 22s, 0, PHASE_GROUND);
                events.ScheduleEvent(EVENT_LIFTOFF, 1min + 33s, 0, PHASE_GROUND);
                break;
            default:
                break;
        }
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_START_INTRO:
                me->SetReactState(REACT_PASSIVE);
                me->SetByteFlag(UNIT_FIELD_BYTES_1, 3, UNIT_BYTE1_FLAG_HOVER);
                me->GetMotionMaster()->MovePoint(POINT_CAST_ROARING_BREATH, IntroFlightPosition1, false);
                break;
            case ACTION_HALT_REVERBERATING_FLAME:
                if (Creature* flame = ObjectAccessor::GetCreature(*me, _reverberatingFlameGUID))
                {
                    flame->m_Events.AddEventAtOffset([flame]()
                    {
                        flame->InterruptNonMeleeSpells(true);
                        flame->GetMotionMaster()->Clear();
                        flame->StopMoving();
                        flame->CastSpell(flame, SPELL_SONIC_FLAMES_FLIGHT, true);
                    }, 1s);

                    events.CancelEvent(EVENT_RESUME_REVERBERATING_FLAME_MOVEMENT);
                    events.ScheduleEvent(EVENT_RESUME_REVERBERATING_FLAME_MOVEMENT, 7s, 0, PHASE_AIR);
                }
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && (!events.IsInPhase(PHASE_INTRO)))
            return;

        events.Update(diff);

        if ((me->HasUnitState(UNIT_STATE_CASTING) && !(events.IsInPhase(PHASE_AIR))) || me->HasUnitState(UNIT_STATE_STUNNED))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ROARING_BREATH:
                    DoCastSelf(SPELL_ROARING_BREATH);
                    events.ScheduleEvent(EVENT_FLY_TO_INTRO_LAND_POSITION, 4s + 300ms);
                    break;
                case EVENT_CLOSE_DOOR:
                    if (GameObject* door = instance->GetGameObject(DATA_ATHENAEUM_DOOR))
                        door->SetGoState(GO_STATE_READY);
                    break;
                case EVENT_FLY_TO_INTRO_LAND_POSITION:
                    me->GetMotionMaster()->MovePoint(POINT_PREPARE_LAND_INTRO, IntroFlightPosition2, false);
                    break;
                case EVENT_SONAR_PULSE:
                    DoCastAOE(SPELL_SONAR_PULSE);
                    events.Repeat(11s);
                    break;
                case EVENT_MODULATION:
                    DoCastAOE(SPELL_MODULATION);
                    events.Repeat(22s, 26s);
                    break;
                case EVENT_SEARING_FLAME:
                    Talk(SAY_ANNOUNCE_SEARING_FLAME);
                    Talk(SAY_SEARING_FLAME);
                    me->StopMoving();
                    DoCastSelf(SPELL_SEARING_FLAME);
                    // Patch 4.1: Searing Flame will put Modulation on a 6 seconds cooldown
                    events.RescheduleEvent(EVENT_MODULATION, 6s);
                    break;
                case EVENT_SONIC_BREATH:
                    DoCastAOE(SPELL_SONIC_BREATH);
                    events.Repeat(42s, 43s);
                    break;
                case EVENT_LIFTOFF:
                    events.SetPhase(PHASE_AIR);
                    me->AttackStop();
                    me->SetReactState(REACT_PASSIVE);
                    DoCastSelf(SPELL_TAKE_OFF_ANIM_KIT);
                    me->GetMotionMaster()->MoveTakeoff(POINT_LIFTOFF, LiftoffPosition);
                    break;
                case EVENT_RESUME_REVERBERATING_FLAME_MOVEMENT:
                    if (Unit* target = ObjectAccessor::GetUnit(*me, _lastShieldUserGUID))
                    {
                        if (Unit* flame = ObjectAccessor::GetUnit(*me, _reverberatingFlameGUID))
                        {
                            flame->CastSpell(target, SPELL_TRACKING);
                            flame->GetMotionMaster()->MoveFollow(target, 0.0f, ChaseAngle(0.0f, 0.0f));
                        }
                    }
                    break;
                case EVENT_LAND:
                    me->RemoveAurasDueToSpell(SPELL_SONAR_PULSE_PERIODIC_TRIGGER);
                    me->InterruptNonMeleeSpells(true);
                    summons.DespawnEntry(NPC_REVERBERATING_FLAME);
                    me->GetMotionMaster()->MoveLand(POINT_LAND, LandPosition);
                    break;
                case EVENT_REENGAGE_PLAYERS:
                    me->SetReactState(REACT_AGGRESSIVE);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    void CleanupEncounter()
    {
        if (GameObject* door = instance->GetGameObject(DATA_ATHENAEUM_DOOR))
            door->SetGoState(GO_STATE_ACTIVE);
    }

    bool _allowVertigoCast;
    std::queue<ObjectGuid>* _lastAncientDwarvenShieldGUIDs;
    GuidSet _noisyPlayerGUIDs;
    ObjectGuid _lastShieldUserGUID;
    ObjectGuid _reverberatingFlameGUID;
};

struct npc_atramedes_ancient_dwarven_shield : public NullCreatureAI
{
    npc_atramedes_ancient_dwarven_shield(Creature* creature) : NullCreatureAI(creature), _instance(me->GetInstanceScript()) { }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(4s);
    }

    void OnSpellClick(Unit* clicker, bool& /*result*/) override
    {
        Creature* atramedes = _instance->GetCreature(DATA_ATRAMEDES);
        if (!atramedes)
            return;

        if (atramedes->AI()->GetData(DATA_IS_IN_AIR))
            clicker->CastSpell(clicker, SPELL_RESONATING_CLASH_AIR, true, nullptr, nullptr, me->GetGUID());
        else
            DoCastSelf(SPELL_RESONATING_CLASH_GROUND);

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
    }

private:
    InstanceScript* _instance;
};

class spell_atramedes_modulation : public SpellScript
{
    PrepareSpellScript(spell_atramedes_modulation);

    void ChangeDamage(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        int32 damage = GetHitDamage();
        AddPct(damage, target->GetPower(POWER_ALTERNATE_POWER));
        SetHitDamage(damage);
    }

    void Register() override
    {
        OnEffectLaunchTarget += SpellEffectFn(spell_atramedes_modulation::ChangeDamage, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};

class spell_atramedes_roaring_flame_breath_reverse_cast : public SpellScript
{
    PrepareSpellScript(spell_atramedes_roaring_flame_breath_reverse_cast);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        if (Unit* caster = GetCaster())
            GetHitUnit()->CastSpell(caster, GetSpellInfo()->Effects[effIndex].BasePoints);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_atramedes_roaring_flame_breath_reverse_cast::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_atramedes_roaring_flame_breath : public AuraScript
{
    PrepareAuraScript(spell_atramedes_roaring_flame_breath);

    void HandleTick(AuraEffect const* aurEff)
    {
        PreventDefaultAction();
        GetTarget()->CastSpell(GetTarget(), GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_atramedes_roaring_flame_breath::HandleTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class spell_atramedes_roaring_flame_breath_fire_periodic : public SpellScript
{
    PrepareSpellScript(spell_atramedes_roaring_flame_breath_fire_periodic);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_ROARING_FLAME_SUMMON });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            GetCaster()->CastSpell(GetCaster(), SPELL_ROARING_FLAME_SUMMON, true);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_atramedes_roaring_flame_breath_fire_periodic::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

class spell_atramedes_resonating_clash_ground : public SpellScript
{
    PrepareSpellScript(spell_atramedes_resonating_clash_ground);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();
        Creature* target = GetHitCreature();
        if (!target || !caster || !target->IsAIEnabled)
            return;

        target->AI()->SetGUID(caster->GetGUID(), DATA_LAST_ANCIENT_DWARVEN_SHIELD);

        // Verify this if it's blizzlike to destroy the shield immediately or just leave it not selectable...
        target->RemoveAurasDueToSpell(sSpellMgr->GetSpellIdForDifficulty(GetSpellInfo()->Effects[effIndex].BasePoints, target));
        target->CastSpell(target, GetSpellInfo()->Effects[effIndex].BasePoints, true);
        target->PlayDirectSound(SOUND_ID_ATRAMEDES_VERTIGO);

        // Atramedes has a interrupt mechanic immunity so we interrupt him manually
        target->InterruptNonMeleeSpells(true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_atramedes_resonating_clash_ground::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_atramedes_resonating_clash_air : public SpellScript
{
    PrepareSpellScript(spell_atramedes_resonating_clash_air);

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Creature* target = GetHitCreature();
        if (!target || !caster || !target->IsAIEnabled)
            return;

        if (Unit* shield = GetSpell()->GetOriginalCaster())
            target->AI()->SetGUID(shield->GetGUID(), DATA_LAST_ANCIENT_DWARVEN_SHIELD);

        target->AI()->SetGUID(caster->GetGUID(), DATA_LAST_SHIELD_USER);

        target->AI()->DoAction(ACTION_HALT_REVERBERATING_FLAME);
        target->PlayDirectSound(SOUND_ID_ATRAMEDES_VERTIGO);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_atramedes_resonating_clash_air::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_atramedes_resonating_clash: public SpellScript
{
    PrepareSpellScript(spell_atramedes_resonating_clash);

    void HandleScriptEffect(SpellEffIndex effIndex)
    {
        GetHitUnit()->RemoveAurasDueToSpell(GetSpellInfo()->Effects[effIndex].BasePoints);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_atramedes_resonating_clash::HandleScriptEffect, EFFECT_2, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

class spell_atramedes_sound_bar : public AuraScript
{
    PrepareAuraScript(spell_atramedes_sound_bar);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_NOISY });
    }

    void HandleNoisyAura(AuraEffect const* aurEff)
    {
        Unit* target = GetTarget();
        if (target->GetPower(POWER_ALTERNATE_POWER) == target->GetMaxPower(POWER_ALTERNATE_POWER))
        {
            if (!target->HasAura(SPELL_NOISY))
                if (InstanceScript* instance = target->GetInstanceScript())
                    if (Creature* atramedes = instance->GetCreature(DATA_ATRAMEDES))
                        atramedes->AI()->SetGUID(target->GetGUID(), DATA_ADD_NOISY_PLAYER);

            target->CastSpell(target, SPELL_NOISY, true, nullptr, aurEff);
        }
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_atramedes_sound_bar::HandleNoisyAura, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

class spell_atramedes_noisy : public AuraScript
{
    PrepareAuraScript(spell_atramedes_noisy);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (InstanceScript* instance = GetTarget()->GetInstanceScript())
            if (Creature* atramedes = instance->GetCreature(DATA_ATRAMEDES))
                atramedes->AI()->SetGUID(GetTarget()->GetGUID(), DATA_REMOVE_NOISY_PLAYER);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_atramedes_noisy::AfterRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_atramedes_vertigo : public AuraScript
{
    PrepareAuraScript(spell_atramedes_vertigo);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        GetTarget()->CastSpell(GetTarget(), GetSpellInfo()->Effects[EFFECT_1].BasePoints, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_atramedes_vertigo::AfterRemove, EFFECT_1, SPELL_AURA_MOD_STUN, AURA_EFFECT_HANDLE_REAL);
    }
};

class spell_atramedes_sonic_flames : public SpellScript
{
    PrepareSpellScript(spell_atramedes_sonic_flames);

    void SetTarget(WorldObject*& target)
    {
        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* atramedes = instance->GetCreature(DATA_ATRAMEDES))
                if (Creature* shield = ObjectAccessor::GetCreature(*GetCaster(), atramedes->AI()->GetGUID(DATA_LAST_ANCIENT_DWARVEN_SHIELD)))
                    target = shield;
    }

    void Register() override
    {
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_atramedes_sonic_flames::SetTarget, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
    }
};

class spell_atramedes_sonic_flames_AuraScript : public AuraScript
{
    PrepareAuraScript(spell_atramedes_sonic_flames_AuraScript);

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        Unit* caster = GetCaster();
        if (!caster)
            return;

        PreventDefaultAction();
        caster->CastSpell(GetTarget(), GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, true, nullptr, aurEff);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_atramedes_sonic_flames_AuraScript::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

class SonicFlamesGuidCheck
{
public:
    SonicFlamesGuidCheck(ObjectGuid guid) : _guid(guid) { }

    bool operator()(WorldObject* object)
    {
        return object->GetGUID() != _guid;
    }
private:
    ObjectGuid _guid;
};

class spell_atramedes_sonic_flames_flight : public SpellScript
{
    PrepareSpellScript(spell_atramedes_sonic_flames_flight);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        if (InstanceScript* instance = GetCaster()->GetInstanceScript())
            if (Creature* atramedes = instance->GetCreature(DATA_ATRAMEDES))
                if (ObjectGuid guid = atramedes->AI()->GetGUID(DATA_LAST_ANCIENT_DWARVEN_SHIELD))
                    targets.remove_if(SonicFlamesGuidCheck(guid));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_atramedes_sonic_flames_flight::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

class spell_atramedes_devastation_trigger : public AuraScript
{
    PrepareAuraScript(spell_atramedes_devastation_trigger);

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        if (Creature* target = GetTarget()->ToCreature())
            if (target->IsAIEnabled)
                if (!target->AI()->GetData(DATA_HAS_NOISY_PLAYER))
                    PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_atramedes_devastation_trigger::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_boss_atramedes()
{
    RegisterBlackwingDescentCreatureAI(boss_atramedes);
    RegisterBlackwingDescentCreatureAI(npc_atramedes_ancient_dwarven_shield);
    RegisterSpellScript(spell_atramedes_modulation);
    RegisterSpellScript(spell_atramedes_roaring_flame_breath_reverse_cast);
    RegisterAuraScript(spell_atramedes_roaring_flame_breath);
    RegisterSpellScript(spell_atramedes_roaring_flame_breath_fire_periodic);
    RegisterSpellScript(spell_atramedes_resonating_clash_ground);
    RegisterSpellScript(spell_atramedes_resonating_clash_air);
    RegisterSpellScript(spell_atramedes_resonating_clash);
    RegisterAuraScript(spell_atramedes_sound_bar);
    RegisterAuraScript(spell_atramedes_noisy);
    RegisterAuraScript(spell_atramedes_vertigo);
    RegisterSpellAndAuraScriptPair(spell_atramedes_sonic_flames, spell_atramedes_sonic_flames_AuraScript);
    RegisterSpellScript(spell_atramedes_sonic_flames_flight);
    RegisterAuraScript(spell_atramedes_devastation_trigger);
}