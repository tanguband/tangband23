#include "player-info/mimic-info-table.h"
#include "player-info/race-info.h"

#ifdef JP
#define N(JAPANESE, ENGLISH) JAPANESE, ENGLISH
#else
#define N(JAPANESE, ENGLISH) ENGLISH
#endif

// clang-format off
/*!
 * @brief 変身種族情報
 */
const std::map<MimicKindType, player_race_info> mimic_info =
{
    {
        MimicKindType::NONE, {
        N("[標準形態]", "Default"), "N",
        {  0,  0,  0,  0,  0,  0 },
        0,  0,  0,  0,  0,  10,  0,  0,
        10,  100,
        0,  0,
        0,  0, 0, 0,
        0,  0, 0, 0,
        0,
        0x000000,
        PlayerRaceLifeType::LIVING,
        PlayerRaceFoodType::RATION,
        { },
        }
    },
    {
        MimicKindType::DEMON, {
        N("[悪魔]", "[Demon]"), "uU",
        {  5,  3,  2,  3,  4,  -6 },
        -5,  18, 20, -2,  3,  10, 40, 20,
        12,  0,
        0,  0,
        0,  0, 0, 0,
        0,  0, 0, 0,
        5,
        0x000003,
        PlayerRaceLifeType::DEMON,
        PlayerRaceFoodType::CORPSE,
        {
            { TR_RES_FIRE },
            { TR_RES_NETHER },
            { TR_RES_CHAOS },
            { TR_SEE_INVIS },
            { TR_HOLD_EXP },
            { TR_SPEED },
        },
        }
    },
    {
        MimicKindType::DEMON_LORD, {
        N("[魔王]", "[Demon lord]"), "U",
        {  20,  20,  20,  20,  20,  20 },
        20,  20, 25, -2,  3,  10, 70, 40,
        14,  0,
        0,  0,
        0,  0, 0, 0,
        0,  0, 0, 0,
        20,
        0x000003,
        PlayerRaceLifeType::DEMON,
        PlayerRaceFoodType::CORPSE,
        {
            { TR_IM_FIRE },
            { TR_RES_COLD },
            { TR_RES_ELEC },
            { TR_RES_ACID },
            { TR_RES_POIS },
            { TR_RES_CONF },
            { TR_RES_NETHER },
            { TR_RES_NEXUS },
            { TR_RES_CHAOS },
            { TR_RES_DISEN },
            { TR_RES_FEAR },
            { TR_SH_FIRE },
            { TR_SEE_INVIS },
            { TR_TELEPATHY },
            { TR_LEVITATION },
            { TR_HOLD_EXP },
            { TR_SPEED },
        },
        }
    },
    {
        MimicKindType::VAMPIRE, {
        N("[吸血鬼]", "[Vampire]"), "V",
        { 4, 4, 1, 1, 2, 3 },
        6, 12, 8, 6, 2, 12, 30, 20,
        11,  0,
        0,  0,
        0,  0, 0, 0,
        0,  0, 0, 0,
        5,
        0x000005,
        PlayerRaceLifeType::UNDEAD,
        PlayerRaceFoodType::BLOOD,
        {
            { TR_RES_COLD },
            { TR_RES_POIS },
            { TR_VUL_LITE },
            { TR_IM_DARK },
            { TR_RES_NETHER },
            { TR_SEE_INVIS },
            { TR_HOLD_EXP },
            { TR_LITE_1, 1, PlayerClassType::NINJA, true },
            { TR_SPEED },
        },
        }
    },
};

// clang-format on
