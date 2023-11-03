/*!
 * @file save.c
 * @brief セーブファイル書き込み処理 / Purpose: interact with savefiles
 * @date 2014/07/12
 * @author
 * <pre>
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * </pre>
 */

#include "save/save.h"
#include "core/object-compressor.h"
#include "dungeon/quest.h"
#include "floor/floor-town.h"
#include "floor/wild.h"
#include "game-option/text-display-options.h"
#include "inventory/inventory-slot-types.h"
#include "io/files-util.h"
#include "io/report.h"
#include "io/uid-checker.h"
#include "monster-race/monster-race.h"
#include "monster/monster-compaction.h"
#include "monster/monster-status.h"
#include "player/player-status.h"
#include "save/floor-writer.h"
#include "save/info-writer.h"
#include "save/item-writer.h"
#include "save/monster-writer.h"
#include "save/player-writer.h"
#include "save/save-util.h"
#include "store/store-owners.h"
#include "store/store-util.h"
#include "system/angband-version.h"
#include "system/artifact-type-definition.h"
#include "system/baseitem-info.h"
#include "system/item-entity.h"
#include "system/monster-race-info.h"
#include "system/player-type-definition.h"
#include "util/angband-files.h"
#include "view/display-messages.h"
#include "world/world.h"
#include <algorithm>
#include <sstream>
#include <string>

/*!
 * @brief セーブデータの書き込み /
 * Actually write a save-file
 * @param player_ptr プレイヤーへの参照ポインタ
 * @return 成功すればtrue
 */
static bool wr_savefile_new(PlayerType *player_ptr, SaveType type)
{
    compact_objects(player_ptr, 0);
    compact_monsters(player_ptr, 0);

    uint32_t now = (uint32_t)time((time_t *)0);
    w_ptr->sf_system = 0L;
    w_ptr->sf_when = now;
    w_ptr->sf_saves++;

    save_xor_byte = 0;
    auto variant_length = VARIANT_NAME.length();
    wr_byte(static_cast<byte>(variant_length));
    for (auto i = 0U; i < variant_length; i++) {
        save_xor_byte = 0;
        wr_byte(VARIANT_NAME[i]);
    }

    save_xor_byte = 0;
    wr_byte(H_VER_MAJOR);
    wr_byte(H_VER_MINOR);
    wr_byte(H_VER_PATCH);
    wr_byte(H_VER_EXTRA);

    byte tmp8u = (byte)Rand_external(256);
    wr_byte(tmp8u);
    v_stamp = 0L;
    x_stamp = 0L;

    wr_u32b(w_ptr->sf_system);
    wr_u32b(w_ptr->sf_when);
    wr_u16b(w_ptr->sf_lives);
    wr_u16b(w_ptr->sf_saves);

    wr_u32b(SAVEFILE_VERSION);
    wr_u16b(0);
    wr_byte(0);

#ifdef JP
#ifdef EUC
    wr_byte(2);
#endif
#ifdef SJIS
    wr_byte(3);
#endif
#else
    wr_byte(1);
#endif

    wr_randomizer();
    wr_options(type);
    uint32_t tmp32u = message_num();
    if ((compress_savefile || (type == SaveType::DEBUG)) && (tmp32u > 40)) {
        tmp32u = 40;
    }

    wr_u32b(tmp32u);
    for (int i = tmp32u - 1; i >= 0; i--) {
        wr_string(*message_str(i));
    }

    uint16_t tmp16u = static_cast<uint16_t>(monraces_info.size());
    wr_u16b(tmp16u);
    for (auto r_idx = 0; r_idx < tmp16u; r_idx++) {
        wr_lore(i2enum<MonsterRaceId>(r_idx));
    }

    tmp16u = static_cast<uint16_t>(baseitems_info.size());
    wr_u16b(tmp16u);
    for (short bi_id = 0; bi_id < tmp16u; bi_id++) {
        wr_perception(bi_id);
    }

    tmp16u = static_cast<uint16_t>(towns_info.size());
    wr_u16b(tmp16u);

    const auto &quest_list = QuestList::get_instance();
    tmp16u = static_cast<uint16_t>(quest_list.size());
    wr_u16b(tmp16u);

    tmp8u = MAX_RANDOM_QUEST - MIN_RANDOM_QUEST;
    wr_byte(tmp8u);

    for (const auto &[q_idx, quest] : quest_list) {
        wr_s16b(enum2i(q_idx));
        wr_s16b(enum2i(quest.status));
        wr_s16b((int16_t)quest.level);
        wr_byte((byte)quest.complev);
        wr_u32b(quest.comptime);

        auto is_quest_running = quest.status == QuestStatusType::TAKEN;
        is_quest_running |= quest.status == QuestStatusType::COMPLETED;
        is_quest_running |= !QuestType::is_fixed(q_idx);
        if (!is_quest_running) {
            continue;
        }

        wr_s16b((int16_t)quest.cur_num);
        wr_s16b((int16_t)quest.max_num);
        wr_s16b(enum2i(quest.type));
        wr_s16b(enum2i(quest.r_idx));
        wr_s16b(enum2i(quest.reward_artifact_idx));
        wr_byte((byte)quest.flags);
        wr_byte((byte)quest.dungeon);
    }

    wr_s32b(player_ptr->wilderness_x);
    wr_s32b(player_ptr->wilderness_y);
    wr_bool(player_ptr->wild_mode);
    wr_bool(player_ptr->ambush_flag);
    wr_s32b(w_ptr->max_wild_x);
    wr_s32b(w_ptr->max_wild_y);
    for (int i = 0; i < w_ptr->max_wild_x; i++) {
        for (int j = 0; j < w_ptr->max_wild_y; j++) {
            wr_u32b(wilderness[j][i].seed);
        }
    }

    auto max_a_num = enum2i(artifacts_info.rbegin()->first);
    tmp16u = max_a_num + 1;
    wr_u16b(tmp16u);
    for (auto i = 0U; i < tmp16u; i++) {
        const auto a_idx = i2enum<FixedArtifactId>(i);
        const auto &artifact = ArtifactsInfo::get_instance().get_artifact(a_idx);
        wr_bool(artifact.is_generated);
        wr_s16b(artifact.floor_id);
    }

    wr_u32b(w_ptr->sf_play_time);
    wr_FlagGroup(w_ptr->sf_winner, wr_byte);
    wr_FlagGroup(w_ptr->sf_retired, wr_byte);

    wr_player(player_ptr);
    tmp16u = PY_MAX_LEVEL;
    wr_u16b(tmp16u);
    for (int i = 0; i < tmp16u; i++) {
        wr_s16b((int16_t)player_ptr->player_hp[i]);
    }

    wr_u32b(player_ptr->spell_learned1);
    wr_u32b(player_ptr->spell_learned2);
    wr_u32b(player_ptr->spell_worked1);
    wr_u32b(player_ptr->spell_worked2);
    wr_u32b(player_ptr->spell_forgotten1);
    wr_u32b(player_ptr->spell_forgotten2);
    wr_s16b(player_ptr->learned_spells);
    wr_s16b(player_ptr->add_spells);
    for (int i = 0; i < 64; i++) {
        wr_byte((byte)player_ptr->spell_order[i]);
    }

    for (int i = 0; i < INVEN_TOTAL; i++) {
        auto *o_ptr = &player_ptr->inventory_list[i];
        if (!o_ptr->is_valid()) {
            continue;
        }

        wr_u16b((uint16_t)i);
        wr_item(o_ptr);
    }

    wr_u16b(0xFFFF);
    tmp16u = static_cast<uint16_t>(towns_info.size());
    wr_u16b(tmp16u);

    tmp16u = MAX_STORES;
    wr_u16b(tmp16u);
    for (size_t i = 1; i < towns_info.size(); i++) {
        for (auto sst : STORE_SALE_TYPE_LIST) {
            wr_store(&towns_info[i].stores[sst]);
        }
    }

    wr_s16b(player_ptr->pet_follow_distance);
    wr_s16b(player_ptr->pet_extra_flags);
    if (screen_dump && (player_ptr->wait_report_score || !player_ptr->is_dead)) {
        wr_string(screen_dump);
    } else {
        wr_string("");
    }

    if (!player_ptr->is_dead) {
        if (!wr_dungeon(player_ptr)) {
            return false;
        }

        wr_ghost();
        wr_s32b(0);
    }

    wr_u32b(v_stamp);
    wr_u32b(x_stamp);
    return !ferror(saving_savefile) && (fflush(saving_savefile) != EOF);
}

/*!
 * @brief セーブデータ書き込みのサブルーチン
 * @param player_ptr プレイヤーへの参照ポインタ
 * @param path セーブデータのフルパス
 * @param type セーブ後の処理種別
 * @return セーブの成功可否
 */
static bool save_player_aux(PlayerType *player_ptr, const std::filesystem::path &path, SaveType type)
{
    safe_setuid_grab();
    auto fd = fd_make(path);
    safe_setuid_drop();

    bool is_save_successful = false;
    saving_savefile = nullptr;
    if (fd >= 0) {
        (void)fd_close(fd);
        safe_setuid_grab();
        saving_savefile = angband_fopen(path, FileOpenMode::WRITE, true);
        safe_setuid_drop();
        if (saving_savefile) {
            if (wr_savefile_new(player_ptr, type)) {
                is_save_successful = true;
            }

            if (angband_fclose(saving_savefile)) {
                is_save_successful = false;
            }
        }

        safe_setuid_grab();
        if (!is_save_successful) {
            (void)fd_kill(path);
        }

        safe_setuid_drop();
    }

    if (!is_save_successful) {
        return false;
    }

    counts_write(player_ptr, 0, w_ptr->play_time);
    w_ptr->character_saved = true;
    return true;
}

/*!
 * @brief セーブデータ書き込みのメインルーチン
 * @param player_ptr プレイヤーへの参照ポインタ
 * @return 成功すればtrue
 * @details 以下の順番で書き込みを実行する.
 * 1. hoge.new にセーブデータを書き込む
 * 2. hoge をhoge.old にリネームする
 * 3. hoge.new をhoge にリネームする
 * 4. hoge.old を削除する
 */
bool save_player(PlayerType *player_ptr, SaveType type)
{
    std::stringstream ss_new;
    ss_new << savefile.string() << ".new";
    auto savefile_new = ss_new.str();
    safe_setuid_grab();
    fd_kill(savefile_new);
    if (type == SaveType::DEBUG) {
        const auto debug_save_dir = std::filesystem::path(debug_savefile).remove_filename();
        std::error_code ec;
        std::filesystem::create_directory(debug_save_dir, ec);
    }
    safe_setuid_drop();
    update_playtime();
    bool result = false;
    if (save_player_aux(player_ptr, savefile_new.data(), type)) {
        std::stringstream ss_old;
        ss_old << savefile.string() << ".old";
        auto savefile_old = ss_old.str();
        safe_setuid_grab();
        fd_kill(savefile_old);
        const auto &path = type == SaveType::DEBUG ? debug_savefile : savefile;
        fd_move(path, savefile_old);
        fd_move(savefile_new, path);
        fd_kill(savefile_old);
        safe_setuid_drop();
        w_ptr->character_loaded = true;
        result = true;
    }

    if (type != SaveType::CLOSE_GAME) {
        w_ptr->is_loading_now = false;
        update_creature(player_ptr);
        mproc_init(player_ptr->current_floor_ptr);
        w_ptr->is_loading_now = true;
    }

    return result;
}
