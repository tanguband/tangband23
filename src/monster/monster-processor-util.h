/*!
 * @brief monster-processのための構造体群
 * @date 2020/03/07
 * @author Hourier
 */

#pragma once

#include "monster-race/race-ability-flags.h"
#include "monster-race/race-behavior-flags.h"
#include "monster-race/race-drop-flags.h"
#include "monster-race/race-feature-flags.h"
#include "monster-race/race-flags-resistance.h"
#include "monster-race/race-kind-flags.h"
#include "system/angband.h"
#include "util/flag-group.h"

enum class MonsterRaceId : int16_t;

struct turn_flags {
    bool see_m;
    bool aware;
    bool is_riding_mon;
    bool do_turn;
    bool do_move;
    bool do_view;
    bool do_take;
    bool must_alter_to_move;

    bool did_open_door;
    bool did_bash_door;
    bool did_take_item;
    bool did_kill_item;
    bool did_move_body;
    bool did_pass_wall;
    bool did_kill_wall;
};

// @details ダミーIDが渡されるとオブジェクトが生焼けになるので、ヘッダ側で全て初期化しておく.
struct old_race_flags {
    old_race_flags(MonsterRaceId monrace_id);

    BIT_FLAGS old_r_flags1 = 0;
    BIT_FLAGS old_r_flags2 = 0;
    BIT_FLAGS old_r_flags3 = 0;
    BIT_FLAGS old_r_flagsr = 0;
    EnumClassFlagGroup<MonsterAbilityType> old_r_ability_flags{};
    EnumClassFlagGroup<MonsterBehaviorType> old_r_behavior_flags{};
    EnumClassFlagGroup<MonsterKindType> old_r_kind_flags{};
    EnumClassFlagGroup<MonsterResistanceType> old_r_resistance_flags{};
    EnumClassFlagGroup<MonsterDropType> old_r_drop_flags{};
    EnumClassFlagGroup<MonsterFeatureType> old_r_feature_flags{};

    byte old_r_blows0 = 0;
    byte old_r_blows1 = 0;
    byte old_r_blows2 = 0;
    byte old_r_blows3 = 0;

    byte old_r_cast_spell = 0;
};

struct coordinate_candidate {
    POSITION gy;
    POSITION gx;
    POSITION gdis;
};

class MonsterEntity;
turn_flags *init_turn_flags(MONSTER_IDX riding_idx, MONSTER_IDX m_idx, turn_flags *turn_flags_ptr);
coordinate_candidate init_coordinate_candidate(void);

void store_enemy_approch_direction(int *mm, POSITION y, POSITION x);
void store_moves_val(int *mm, int y, int x);
