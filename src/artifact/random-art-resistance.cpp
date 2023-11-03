/*!
 * @file random-art-resistance.cpp
 * @brief ランダムアーティファクトの耐性付加処理実装
 */

#include "artifact/random-art-resistance.h"
#include "artifact/random-art-bias-types.h"
#include "object-enchant/tr-types.h"
#include "object/tval-types.h"
#include "system/item-entity.h"
#include "util/bit-flags-calculator.h"

/*!
 * @brief オーラフラグを付与する
 * @param o_ptr フラグを付与するオブジェクト構造体へのポインタ
 * @param tr_sh_flag 付与するフラグ (TR_SH_*)
 * @return 続くフラグ付与の処理を打ち切る場合 true を返す
 */
static bool random_art_aura(ItemEntity *o_ptr, tr_type tr_sh_flag)
{
    if (!o_ptr->can_be_aura_protector() || o_ptr->art_flags.has(tr_sh_flag)) {
        return false;
    }

    o_ptr->art_flags.set(tr_sh_flag);
    return one_in_(2);
}

/*!
 * @brief 免疫フラグを付与する
 * @param o_ptr フラグを付与するオブジェクト構造体へのポインタ
 * @param tr_im_flag 付与するフラグ (TR_IM_*)
 * @return 続くフラグ付与の処理を打ち切る場合 true を返す
 * @details フラグの付与には 1/chance_bias の確率を通過する必要がある.
 * 更に、1/chance_immunity の確率を通過しなければ他の免疫フラグは消去される.
 */
static bool random_art_immunity(ItemEntity *o_ptr, tr_type tr_im_flag)
{
    constexpr auto chance_bias = 20;
    if (!one_in_(chance_bias) || o_ptr->art_flags.has(tr_im_flag)) {
        return false;
    }

    constexpr auto chance_immunity = 7;
    if (!one_in_(chance_immunity)) {
        o_ptr->art_flags.reset(TR_IM_ACID);
        o_ptr->art_flags.reset(TR_IM_ELEC);
        o_ptr->art_flags.reset(TR_IM_FIRE);
        o_ptr->art_flags.reset(TR_IM_COLD);
    }

    o_ptr->art_flags.set(tr_im_flag);
    return one_in_(2);
}

/*!
 * @brief 耐性フラグを付与する
 *
 * @param o_ptr フラグを付与するオブジェクト構造体へのポインタ
 * @param tr_res_flag 付与するフラグ (TR_RES_*)
 * @param skip true の場合このフラグは付与せず処理を終了し、続くフラグ付与の処理を継続する
 * @return 続くフラグ付与の処理を打ち切る場合 true を返す
 */
static bool random_art_resistance(ItemEntity *o_ptr, tr_type tr_res_flag, bool skip = false)
{
    if (skip || o_ptr->art_flags.has(tr_res_flag)) {
        return false;
    }

    o_ptr->art_flags.set(tr_res_flag);
    return one_in_(2);
}

static bool switch_random_art_resistance(ItemEntity *o_ptr)
{
    switch (o_ptr->artifact_bias) {
    case BIAS_ACID:
        return random_art_resistance(o_ptr, TR_RES_ACID) || random_art_immunity(o_ptr, TR_IM_ACID);
    case BIAS_ELEC:
        return random_art_resistance(o_ptr, TR_RES_ELEC) || random_art_aura(o_ptr, TR_SH_ELEC) || random_art_immunity(o_ptr, TR_IM_ELEC);
    case BIAS_FIRE:
        return random_art_resistance(o_ptr, TR_RES_FIRE) || random_art_aura(o_ptr, TR_SH_FIRE) || random_art_immunity(o_ptr, TR_IM_FIRE);
    case BIAS_COLD:
        return random_art_resistance(o_ptr, TR_RES_COLD) || random_art_aura(o_ptr, TR_SH_COLD) || random_art_immunity(o_ptr, TR_IM_COLD);
    case BIAS_POIS:
        return random_art_resistance(o_ptr, TR_RES_POIS);
    case BIAS_PRIESTLY:
        return random_art_resistance(o_ptr, TR_RES_CURSE);
    case BIAS_WARRIOR:
        return random_art_resistance(o_ptr, TR_RES_FEAR, one_in_(3)) || random_art_resistance(o_ptr, TR_NO_MAGIC, !one_in_(3));
    case BIAS_RANGER:
        return random_art_resistance(o_ptr, TR_RES_WATER) || random_art_resistance(o_ptr, TR_RES_POIS);
    case BIAS_NECROMANTIC:
        return random_art_resistance(o_ptr, TR_RES_NETHER) || random_art_resistance(o_ptr, TR_RES_POIS) || random_art_resistance(o_ptr, TR_RES_DARK);
    case BIAS_CHAOS:
        return random_art_resistance(o_ptr, TR_RES_CHAOS) || random_art_resistance(o_ptr, TR_RES_CONF) || random_art_resistance(o_ptr, TR_RES_DISEN);
    default:
        return false;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_acid(ItemEntity *o_ptr)
{
    if (!one_in_(CHANCE_STRENGTHENING)) {
        random_resistance(o_ptr);
        return;
    }

    o_ptr->art_flags.set(TR_IM_ACID);
    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_ACID;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_elec(ItemEntity *o_ptr)
{
    if (!one_in_(CHANCE_STRENGTHENING)) {
        random_resistance(o_ptr);
        return;
    }

    o_ptr->art_flags.set(TR_IM_ELEC);
    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_ELEC;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_cold(ItemEntity *o_ptr)
{
    if (!one_in_(CHANCE_STRENGTHENING)) {
        random_resistance(o_ptr);
        return;
    }

    o_ptr->art_flags.set(TR_IM_COLD);
    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_COLD;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_fire(ItemEntity *o_ptr)
{
    if (!one_in_(CHANCE_STRENGTHENING)) {
        random_resistance(o_ptr);
        return;
    }

    o_ptr->art_flags.set(TR_IM_FIRE);
    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_FIRE;
    }
}

static void set_bias_pois(ItemEntity *o_ptr)
{
    o_ptr->art_flags.set(TR_RES_POIS);
    if (!o_ptr->artifact_bias && !one_in_(4)) {
        o_ptr->artifact_bias = BIAS_POIS;
        return;
    }

    if (!o_ptr->artifact_bias && one_in_(2)) {
        o_ptr->artifact_bias = BIAS_NECROMANTIC;
        return;
    }

    if (!o_ptr->artifact_bias && one_in_(2)) {
        o_ptr->artifact_bias = BIAS_ROGUE;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_aura_elec(ItemEntity *o_ptr)
{
    if (o_ptr->can_be_aura_protector()) {
        o_ptr->art_flags.set(TR_SH_ELEC);
    } else {
        random_resistance(o_ptr);
    }

    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_ELEC;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_aura_fire(ItemEntity *o_ptr)
{
    if (o_ptr->can_be_aura_protector()) {
        o_ptr->art_flags.set(TR_SH_FIRE);
    } else {
        random_resistance(o_ptr);
    }

    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_FIRE;
    }
}

/* 一定確率で再試行する */
static void set_weird_bias_reflection(ItemEntity *o_ptr)
{
    switch (o_ptr->bi_key.tval()) {
    case ItemKindType::SHIELD:
    case ItemKindType::CLOAK:
    case ItemKindType::HELM:
    case ItemKindType::HARD_ARMOR:
        o_ptr->art_flags.set(TR_REFLECT);
        return;
    default:
        break;
    }

    random_resistance(o_ptr);
}

/* 一定確率で再試行する */
static void set_weird_bias_aura_cold(ItemEntity *o_ptr)
{
    if (o_ptr->can_be_aura_protector()) {
        o_ptr->art_flags.set(TR_SH_COLD);
    } else {
        random_resistance(o_ptr);
    }

    if (!o_ptr->artifact_bias) {
        o_ptr->artifact_bias = BIAS_COLD;
    }
}

/*!
 * @brief ランダムアーティファクト生成中、対象のオブジェクトに耐性を付加する。/ Add one resistance on generation of randam artifact.
 * @details 優先的に付加される耐性がランダムアーティファクトバイアスに依存して存在する。
 * 原則的候補は火炎、冷気、電撃、酸（以上免疫の可能性もあり）、
 * 毒、閃光、暗黒、破片、轟音、盲目、混乱、地獄、カオス、劣化、恐怖、火オーラ、冷気オーラ、電撃オーラ、反射。
 * 戦士系バイアスのみ反魔もつく。
 * @attention オブジェクトのtval、svalに依存したハードコーディング処理がある。
 * @param o_ptr 対象のオブジェクト構造体ポインタ
 */
void random_resistance(ItemEntity *o_ptr)
{
    if (switch_random_art_resistance(o_ptr)) {
        return;
    }

    switch (randint1(45)) {
    case 1:
        set_weird_bias_acid(o_ptr);
        break;
    case 2:
        set_weird_bias_elec(o_ptr);
        break;
    case 3:
        set_weird_bias_cold(o_ptr);
        break;
    case 4:
        set_weird_bias_fire(o_ptr);
        break;
    case 5:
    case 6:
    case 13:
        o_ptr->art_flags.set(TR_RES_ACID);
        if (!o_ptr->artifact_bias) {
            o_ptr->artifact_bias = BIAS_ACID;
        }

        break;
    case 7:
    case 8:
    case 14:
        o_ptr->art_flags.set(TR_RES_ELEC);
        if (!o_ptr->artifact_bias) {
            o_ptr->artifact_bias = BIAS_ELEC;
        }

        break;
    case 9:
    case 10:
    case 15:
        o_ptr->art_flags.set(TR_RES_FIRE);
        if (!o_ptr->artifact_bias) {
            o_ptr->artifact_bias = BIAS_FIRE;
        }

        break;
    case 11:
    case 12:
    case 16:
        o_ptr->art_flags.set(TR_RES_COLD);
        if (!o_ptr->artifact_bias) {
            o_ptr->artifact_bias = BIAS_COLD;
        }

        break;
    case 17:
    case 18:
        set_bias_pois(o_ptr);
        break;
    case 19:
    case 20:
        o_ptr->art_flags.set(TR_RES_FEAR);
        if (!o_ptr->artifact_bias && one_in_(3)) {
            o_ptr->artifact_bias = BIAS_WARRIOR;
        }

        break;
    case 21:
        o_ptr->art_flags.set(TR_RES_LITE);
        break;
    case 22:
        o_ptr->art_flags.set(TR_RES_DARK);
        break;
    case 23:
    case 24:
        o_ptr->art_flags.set(TR_RES_BLIND);
        break;
    case 25:
    case 26:
        o_ptr->art_flags.set(TR_RES_CONF);
        if (!o_ptr->artifact_bias && one_in_(6)) {
            o_ptr->artifact_bias = BIAS_CHAOS;
        }

        break;
    case 27:
    case 28:
        o_ptr->art_flags.set(TR_RES_SOUND);
        break;
    case 29:
    case 30:
        o_ptr->art_flags.set(TR_RES_SHARDS);
        break;
    case 31:
    case 32:
        o_ptr->art_flags.set(TR_RES_NETHER);
        if (!o_ptr->artifact_bias && one_in_(3)) {
            o_ptr->artifact_bias = BIAS_NECROMANTIC;
        }

        break;
    case 33:
    case 34:
        o_ptr->art_flags.set(TR_RES_NEXUS);
        break;
    case 35:
    case 36:
        o_ptr->art_flags.set(TR_RES_CHAOS);
        if (!o_ptr->artifact_bias && one_in_(2)) {
            o_ptr->artifact_bias = BIAS_CHAOS;
        }

        break;
    case 37:
    case 38:
        o_ptr->art_flags.set(TR_RES_DISEN);
        break;
    case 39:
        o_ptr->art_flags.set(TR_RES_TIME);
        break;
    case 40:
        o_ptr->art_flags.set(TR_RES_WATER);
        if (!o_ptr->artifact_bias && one_in_(6)) {
            o_ptr->artifact_bias = BIAS_RANGER;
        }
        break;
    case 41:
        o_ptr->art_flags.set(TR_RES_CURSE);
        if (!o_ptr->artifact_bias && one_in_(3)) {
            o_ptr->artifact_bias = BIAS_PRIESTLY;
        }
        break;
    case 42:
        set_weird_bias_aura_elec(o_ptr);
        break;
    case 43:
        set_weird_bias_aura_fire(o_ptr);
        break;
    case 44:
        set_weird_bias_reflection(o_ptr);
        break;
    case 45:
        set_weird_bias_aura_cold(o_ptr);
        break;
    }
}
