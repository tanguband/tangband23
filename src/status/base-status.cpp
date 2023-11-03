#include "status/base-status.h"
#include "avatar/avatar.h"
#include "core/window-redrawer.h"
#include "game-option/birth-options.h"
#include "inventory/inventory-slot-types.h"
#include "object-enchant/item-feeling.h"
#include "object-enchant/special-object-flags.h"
#include "perception/object-perception.h"
#include "player/player-status-flags.h"
#include "spell-kind/spells-floor.h"
#include "system/item-entity.h"
#include "system/player-type-definition.h"
#include "system/redrawing-flags-updater.h"
#include "util/bit-flags-calculator.h"
#include "view/display-messages.h"

/* Array of stat "descriptions" */
static concptr desc_stat_pos[] = {
    _("強く", "stronger"),
    _("知的に", "smarter"),
    _("賢く", "wiser"),
    _("器用に", "more dextrous"),
    _("健康に", "healthier"),
    _("美しく", "cuter")
};

/* Array of stat "descriptions" */
static concptr desc_stat_neg[] = {
    _("弱く", "weaker"),
    _("無知に", "stupider"),
    _("愚かに", "more naive"),
    _("不器用に", "clumsier"),
    _("不健康に", "more sickly"),
    _("醜く", "uglier")
};

/*!
 * @brief プレイヤーの基本能力値を増加させる / Increases a stat by one randomized level -RAK-
 * @param stat 上昇させるステータスID
 * @return 実際に上昇した場合TRUEを返す。
 * @details
 * Note that this function (used by stat potions) now restores\n
 * the stat BEFORE increasing it.\n
 */
bool inc_stat(PlayerType *player_ptr, int stat)
{
    auto value = player_ptr->stat_cur[stat];
    if (value >= player_ptr->stat_max_max[stat]) {
        return false;
    }

    if (value < 18) {
        value += (randint0(100) < 75) ? 1 : 2;
    } else if (value < (player_ptr->stat_max_max[stat] - 2)) {
        auto gain = (((player_ptr->stat_max_max[stat]) - value) / 2 + 3) / 2;
        if (gain < 1) {
            gain = 1;
        }

        value += randint1(gain) + gain / 2;
        if (value > (player_ptr->stat_max_max[stat] - 1)) {
            value = player_ptr->stat_max_max[stat] - 1;
        }
    } else {
        value++;
    }

    player_ptr->stat_cur[stat] = value;
    if (value > player_ptr->stat_max[stat]) {
        player_ptr->stat_max[stat] = value;
    }

    RedrawingFlagsUpdater::get_instance().set_flag(StatusRecalculatingFlag::BONUS);
    return true;
}

/*!
 * @brief プレイヤーの基本能力値を減少させる / Decreases a stat by an amount indended to vary from 0 to 100 percent.
 * @param stat 減少させるステータスID
 * @param amount 減少させる基本量
 * @param permanent TRUEならば現在の最大値を減少させる
 * @return 実際に減少した場合TRUEを返す。
 * @details
 *\n
 * Amount could be a little higher in extreme cases to mangle very high\n
 * stats from massive assaults.  -CWS\n
 *\n
 * Note that "permanent" means that the *given* amount is permanent,\n
 * not that the new value becomes permanent.  This may not work exactly\n
 * as expected, due to "weirdness" in the algorithm, but in general,\n
 * if your stat is already drained, the "max" value will not drop all\n
 * the way down to the "cur" value.\n
 */
bool dec_stat(PlayerType *player_ptr, int stat, int amount, int permanent)
{
    auto res = false;
    auto cur = player_ptr->stat_cur[stat];
    auto max = player_ptr->stat_max[stat];
    int same = (cur == max);
    if (cur > 3) {
        if (cur <= 18) {
            if (amount > 90) {
                cur--;
            }
            if (amount > 50) {
                cur--;
            }
            if (amount > 20) {
                cur--;
            }
            cur--;
        } else {
            int loss = (((cur - 18) / 2 + 1) / 2 + 1);
            if (loss < 1) {
                loss = 1;
            }

            loss = ((randint1(loss) + loss) * amount) / 100;
            if (loss < amount / 2) {
                loss = amount / 2;
            }

            cur = cur - loss;
            if (cur < 18) {
                cur = (amount <= 20) ? 18 : 17;
            }
        }

        if (cur < 3) {
            cur = 3;
        }

        if (cur != player_ptr->stat_cur[stat]) {
            res = true;
        }
    }

    if (permanent && (max > 3)) {
        chg_virtue(player_ptr, Virtue::SACRIFICE, 1);
        if (stat == A_WIS || stat == A_INT) {
            chg_virtue(player_ptr, Virtue::ENLIGHTEN, -2);
        }

        if (max <= 18) {
            if (amount > 90) {
                max--;
            }
            if (amount > 50) {
                max--;
            }
            if (amount > 20) {
                max--;
            }
            max--;
        } else {
            int loss = (((max - 18) / 2 + 1) / 2 + 1);
            loss = ((randint1(loss) + loss) * amount) / 100;
            if (loss < amount / 2) {
                loss = amount / 2;
            }

            max = max - loss;
            if (max < 18) {
                max = (amount <= 20) ? 18 : 17;
            }
        }

        if (same || (max < cur)) {
            max = cur;
        }

        if (max != player_ptr->stat_max[stat]) {
            res = true;
        }
    }

    if (res) {
        player_ptr->stat_cur[stat] = cur;
        player_ptr->stat_max[stat] = max;
        auto &rfu = RedrawingFlagsUpdater::get_instance();
        rfu.set_flag(MainWindowRedrawingFlag::ABILITY_SCORE);
        rfu.set_flag(StatusRecalculatingFlag::BONUS);
    }

    return res;
}

/*!
 * @brief プレイヤーの基本能力値を回復させる / Restore a stat.  Return TRUE only if this actually makes a difference.
 * @param stat 回復ステータスID
 * @return 実際に回復した場合TRUEを返す。
 */
bool res_stat(PlayerType *player_ptr, int stat)
{
    if (player_ptr->stat_cur[stat] != player_ptr->stat_max[stat]) {
        player_ptr->stat_cur[stat] = player_ptr->stat_max[stat];
        auto &rfu = RedrawingFlagsUpdater::get_instance();
        rfu.set_flag(StatusRecalculatingFlag::BONUS);
        rfu.set_flag(MainWindowRedrawingFlag::ABILITY_SCORE);
        return true;
    }

    return false;
}

/*
 * Lose a "point"
 */
bool do_dec_stat(PlayerType *player_ptr, int stat)
{
    bool sust = false;
    switch (stat) {
    case A_STR:
        if (has_sustain_str(player_ptr)) {
            sust = true;
        }
        break;
    case A_INT:
        if (has_sustain_int(player_ptr)) {
            sust = true;
        }
        break;
    case A_WIS:
        if (has_sustain_wis(player_ptr)) {
            sust = true;
        }
        break;
    case A_DEX:
        if (has_sustain_dex(player_ptr)) {
            sust = true;
        }
        break;
    case A_CON:
        if (has_sustain_con(player_ptr)) {
            sust = true;
        }
        break;
    case A_CHR:
        if (has_sustain_chr(player_ptr)) {
            sust = true;
        }
        break;
    }

    if (sust && (!ironman_nightmare || randint0(13))) {
        msg_format(_("%sなった気がしたが、すぐに元に戻った。", "You feel %s for a moment, but the feeling passes."), desc_stat_neg[stat]);
        return true;
    }

    if (dec_stat(player_ptr, stat, 10, (ironman_nightmare && !randint0(13)))) {
        msg_format(_("ひどく%sなった気がする。", "You feel %s."), desc_stat_neg[stat]);
        return true;
    }

    return false;
}

/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(PlayerType *player_ptr, int stat)
{
    if (res_stat(player_ptr, stat)) {
        msg_format(_("元通りに%sなった気がする。", "You feel %s."), desc_stat_pos[stat]);
        return true;
    }

    return false;
}

/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(PlayerType *player_ptr, int stat)
{
    bool res = res_stat(player_ptr, stat);
    if (inc_stat(player_ptr, stat)) {
        if (stat == A_WIS) {
            chg_virtue(player_ptr, Virtue::ENLIGHTEN, 1);
            chg_virtue(player_ptr, Virtue::FAITH, 1);
        } else if (stat == A_INT) {
            chg_virtue(player_ptr, Virtue::KNOWLEDGE, 1);
            chg_virtue(player_ptr, Virtue::ENLIGHTEN, 1);
        } else if (stat == A_CON) {
            chg_virtue(player_ptr, Virtue::VITALITY, 1);
        }

        msg_format(_("ワーオ！とても%sなった！", "Wow! You feel %s!"), desc_stat_pos[stat]);
        return true;
    }

    if (res) {
        msg_format(_("元通りに%sなった気がする。", "You feel %s."), desc_stat_pos[stat]);
        return true;
    }

    return false;
}

/*
 * Forget everything
 */
bool lose_all_info(PlayerType *player_ptr)
{
    chg_virtue(player_ptr, Virtue::KNOWLEDGE, -5);
    chg_virtue(player_ptr, Virtue::ENLIGHTEN, -5);
    for (int i = 0; i < INVEN_TOTAL; i++) {
        auto *o_ptr = &player_ptr->inventory_list[i];
        if (!o_ptr->is_valid() || o_ptr->is_fully_known()) {
            continue;
        }

        o_ptr->feeling = FEEL_NONE;
        o_ptr->ident &= ~(IDENT_EMPTY);
        o_ptr->ident &= ~(IDENT_KNOWN);
        o_ptr->ident &= ~(IDENT_SENSE);
    }

    auto &rfu = RedrawingFlagsUpdater::get_instance();
    static constexpr auto flags_srf = {
        StatusRecalculatingFlag::BONUS,
        StatusRecalculatingFlag::COMBINATION,
        StatusRecalculatingFlag::REORDER,
    };
    rfu.set_flags(flags_srf);
    static constexpr auto flags_swrf = {
        SubWindowRedrawingFlag::INVENTORY,
        SubWindowRedrawingFlag::EQUIPMENT,
        SubWindowRedrawingFlag::PLAYER,
        SubWindowRedrawingFlag::FLOOR_ITEMS,
        SubWindowRedrawingFlag::FOUND_ITEMS,
    };
    rfu.set_flags(flags_swrf);
    wiz_dark(player_ptr);
    return true;
}
