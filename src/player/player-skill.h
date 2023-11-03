#pragma once

#include "system/angband.h"

#include "util/enum-range.h"

#include <array>
#include <map>
#include <string>
#include <vector>

enum class PlayerSkillKindType {
    MARTIAL_ARTS = 0,
    TWO_WEAPON = 1,
    RIDING = 2,
    SHIELD = 3,
    MAX,
};

enum class PlayerSkillRank {
    UNSKILLED = 0,
    BEGINNER = 1,
    SKILLED = 2,
    EXPERT = 3,
    MASTER = 4,
};

constexpr auto MAX_SKILLS = 10;
constexpr auto PLAYER_SKILL_KIND_TYPE_RANGE = EnumRange(PlayerSkillKindType::MARTIAL_ARTS, PlayerSkillKindType::SHIELD);

enum class ItemKindType : short;

/*
 * Information about "skill"
 */
struct skill_table {
    std::map<ItemKindType, std::array<SUB_EXP, 64>> w_start{}; /* start weapon exp */
    std::map<ItemKindType, std::array<SUB_EXP, 64>> w_max{}; /* max weapon exp */
    std::map<PlayerSkillKindType, SUB_EXP> s_start{}; /* start skill */
    std::map<PlayerSkillKindType, SUB_EXP> s_max{}; /* max skill */
};

extern std::vector<skill_table> class_skills_info;

class MonsterRaceInfo;
class ItemEntity;
class PlayerType;

class PlayerSkill {
public:
    PlayerSkill(PlayerType *player_ptr);

    static SUB_EXP weapon_exp_at(PlayerSkillRank rank);
    static SUB_EXP spell_exp_at(PlayerSkillRank rank);
    static bool valid_weapon_exp(int weapon_exp);
    static PlayerSkillRank weapon_skill_rank(int weapon_exp);
    static PlayerSkillRank riding_skill_rank(int riding_exp);
    static PlayerSkillRank spell_skill_rank(int spell_exp);
    static concptr skill_name(PlayerSkillKindType skill);
    static concptr skill_rank_str(PlayerSkillRank rank);

    void gain_melee_weapon_exp(const ItemEntity *o_ptr);
    void gain_range_weapon_exp(const ItemEntity *o_ptr);
    void gain_martial_arts_skill_exp();
    void gain_two_weapon_skill_exp();
    void gain_riding_skill_exp_on_melee_attack(const MonsterRaceInfo *r_ptr);
    void gain_riding_skill_exp_on_range_attack();
    void gain_riding_skill_exp_on_fall_off_check(int dam);
    void gain_spell_skill_exp(int realm, int spell_idx);
    void gain_continuous_spell_skill_exp(int realm, int spell_idx);
    PlayerSkillRank gain_spell_skill_exp_over_learning(int spell_idx);

    EXP exp_of_spell(int realm, int spell_idx) const;

    void apply_special_weapon_skill_max_values();
    void limit_weapon_skills_by_max_value();

private:
    PlayerType *player_ptr;
};
