#include "effect/effect-monster-spirit.h"
#include "effect/effect-monster-util.h"
#include "hpmp/hp-mp-processor.h"
#include "monster-race/monster-race.h"
#include "monster-race/race-ability-mask.h"
#include "monster-race/race-flags1.h"
#include "monster-race/race-flags2.h"
#include "monster-race/race-flags3.h"
#include "monster/monster-describer.h"
#include "monster/monster-info.h"
#include "monster/monster-status-setter.h"
#include "monster/monster-status.h"
#include "system/grid-type-definition.h"
#include "system/monster-entity.h"
#include "system/monster-race-info.h"
#include "system/player-type-definition.h"
#include "system/redrawing-flags-updater.h"
#include "util/bit-flags-calculator.h"
#include "util/string-processor.h"
#include "view/display-messages.h"

ProcessResult effect_monster_drain_mana(PlayerType *player_ptr, EffectMonster *em_ptr)
{
    if (em_ptr->seen) {
        em_ptr->obvious = true;
    }
    auto ability_flags = em_ptr->r_ptr->ability_flags;
    bool has_mana = ability_flags.reset(RF_ABILITY_NOMAGIC_MASK).any();
    if (!has_mana) {
        if (em_ptr->see_s_msg) {
            msg_format(_("%sには効果がなかった。", "%s is unaffected."), em_ptr->m_name);
        }

        em_ptr->dam = 0;
        return ProcessResult::PROCESS_CONTINUE;
    }

    if (em_ptr->who <= 0) {
        msg_format(_("%sから精神エネルギーを吸いとった。", "You draw psychic energy from %s."), em_ptr->m_name);
        (void)hp_player(player_ptr, em_ptr->dam);
        em_ptr->dam = 0;
        return ProcessResult::PROCESS_CONTINUE;
    }

    if (em_ptr->m_caster_ptr->hp >= em_ptr->m_caster_ptr->maxhp) {
        em_ptr->dam = 0;
        return ProcessResult::PROCESS_CONTINUE;
    }

    em_ptr->m_caster_ptr->hp += em_ptr->dam;
    if (em_ptr->m_caster_ptr->hp > em_ptr->m_caster_ptr->maxhp) {
        em_ptr->m_caster_ptr->hp = em_ptr->m_caster_ptr->maxhp;
    }

    auto &rfu = RedrawingFlagsUpdater::get_instance();
    if (player_ptr->health_who == em_ptr->who) {
        rfu.set_flag(MainWindowRedrawingFlag::HEALTH);
    }

    if (player_ptr->riding == em_ptr->who) {
        rfu.set_flag(MainWindowRedrawingFlag::UHEALTH);
    }

    if (em_ptr->see_s_msg) {
        angband_strcpy(em_ptr->killer, monster_desc(player_ptr, em_ptr->m_caster_ptr, 0), sizeof(em_ptr->killer));
        msg_format(_("%s^は気分が良さそうだ。", "%s^ appears healthier."), em_ptr->killer);
    }

    em_ptr->dam = 0;
    return ProcessResult::PROCESS_CONTINUE;
}

ProcessResult effect_monster_mind_blast(PlayerType *player_ptr, EffectMonster *em_ptr)
{
    if (em_ptr->seen) {
        em_ptr->obvious = true;
    }
    if (!em_ptr->who) {
        msg_format(_("%sをじっと睨んだ。", "You gaze intently at %s."), em_ptr->m_name);
    }

    bool has_immute = em_ptr->r_ptr->kind_flags.has(MonsterKindType::UNIQUE);
    has_immute |= em_ptr->r_ptr->resistance_flags.has(MonsterResistanceType::NO_CONF);
    has_immute |= (em_ptr->r_ptr->level > randint1(std::max(1, em_ptr->caster_lev - 10)) + 10);

    if (has_immute) {
        if (em_ptr->r_ptr->resistance_flags.has(MonsterResistanceType::NO_CONF)) {
            if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
                em_ptr->r_ptr->resistance_flags.set(MonsterResistanceType::NO_CONF);
            }
        }

        em_ptr->note = _("には効果がなかった。", " is unaffected.");
        em_ptr->dam = 0;
    } else if (em_ptr->r_ptr->flags2 & RF2_EMPTY_MIND) {
        if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
            em_ptr->r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
        }
        em_ptr->note = _("には完全な耐性がある！", " is immune.");
        em_ptr->dam = 0;
    } else if (em_ptr->r_ptr->flags2 & RF2_WEIRD_MIND) {
        if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
            em_ptr->r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
        }
        em_ptr->note = _("には耐性がある。", " resists.");
        em_ptr->dam /= 3;
    } else {
        em_ptr->note = _("は精神攻撃を食らった。", " is blasted by psionic energy.");
        em_ptr->note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");

        if (em_ptr->who > 0) {
            em_ptr->do_conf = randint0(4) + 4;
        } else {
            em_ptr->do_conf = randint0(8) + 8;
        }
    }

    return ProcessResult::PROCESS_CONTINUE;
}

ProcessResult effect_monster_brain_smash(PlayerType *player_ptr, EffectMonster *em_ptr)
{
    if (em_ptr->seen) {
        em_ptr->obvious = true;
    }
    if (!em_ptr->who) {
        msg_format(_("%sをじっと睨んだ。", "You gaze intently at %s."), em_ptr->m_name);
    }

    bool has_immute = em_ptr->r_ptr->kind_flags.has(MonsterKindType::UNIQUE);
    has_immute |= em_ptr->r_ptr->resistance_flags.has(MonsterResistanceType::NO_CONF);
    has_immute |= (em_ptr->r_ptr->level > randint1(std::max(1, em_ptr->caster_lev - 10)) + 10);

    if (has_immute) {
        if (em_ptr->r_ptr->resistance_flags.has(MonsterResistanceType::NO_CONF)) {
            if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
                em_ptr->r_ptr->resistance_flags.set(MonsterResistanceType::NO_CONF);
            }
        }

        em_ptr->note = _("には効果がなかった。", " is unaffected.");
        em_ptr->dam = 0;
    } else if (em_ptr->r_ptr->flags2 & RF2_EMPTY_MIND) {
        if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
            em_ptr->r_ptr->r_flags2 |= (RF2_EMPTY_MIND);
        }

        em_ptr->note = _("には完全な耐性がある！", " is immune.");
        em_ptr->dam = 0;
    } else if (em_ptr->r_ptr->flags2 & RF2_WEIRD_MIND) {
        if (is_original_ap_and_seen(player_ptr, em_ptr->m_ptr)) {
            em_ptr->r_ptr->r_flags2 |= (RF2_WEIRD_MIND);
        }

        em_ptr->note = _("には耐性がある！", " resists!");
        em_ptr->dam /= 3;
    } else {
        em_ptr->note = _("は精神攻撃を食らった。", " is blasted by psionic energy.");
        em_ptr->note_dies = _("の精神は崩壊し、肉体は抜け殻となった。", " collapses, a mindless husk.");
        if (em_ptr->who > 0) {
            em_ptr->do_conf = randint0(4) + 4;
            em_ptr->do_stun = randint0(4) + 4;
        } else {
            em_ptr->do_conf = randint0(8) + 8;
            em_ptr->do_stun = randint0(8) + 8;
        }

        (void)set_monster_slow(player_ptr, em_ptr->g_ptr->m_idx, em_ptr->m_ptr->get_remaining_deceleration() + 10);
    }

    return ProcessResult::PROCESS_CONTINUE;
}
