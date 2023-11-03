#include "load/player-class-specific-data-loader.h"
#include "load/load-util.h"
#include "player-info/bard-data-type.h"
#include "player-info/bluemage-data-type.h"
#include "player-info/force-trainer-data-type.h"
#include "player-info/magic-eater-data-type.h"
#include "player-info/mane-data-type.h"
#include "player-info/monk-data-type.h"
#include "player-info/ninja-data-type.h"
#include "player-info/samurai-data-type.h"
#include "player-info/smith-data-type.h"
#include "player-info/sniper-data-type.h"
#include "player-info/spell-hex-data-type.h"
#include "util/enum-converter.h"

#include <tuple>
#include <vector>

namespace {

//! 職業間で配列データを共有して使っていた時の配列長。古いセーブデータのマイグレーション用。
constexpr int OLD_SAVEFILE_MAX_SPELLS = 108;

std::tuple<std::vector<int32_t>, std::vector<byte>> load_old_savfile_magic_num()
{
    std::vector<int32_t> magic_num1(OLD_SAVEFILE_MAX_SPELLS);
    std::vector<byte> magic_num2(OLD_SAVEFILE_MAX_SPELLS);

    if (loading_savefile_version_is_older_than(9)) {
        for (auto &item : magic_num1) {
            item = rd_s32b();
        }
        for (auto &item : magic_num2) {
            item = rd_byte();
        }
    }

    return std::make_tuple(std::move(magic_num1), std::move(magic_num2));
}

}

void PlayerClassSpecificDataLoader::operator()(no_class_specific_data &) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // マイグレーションすべきデータは無いので読み捨てる
        load_old_savfile_magic_num();
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<smith_data_type> &smith_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        for (auto i = 0; i < OLD_SAVEFILE_MAX_SPELLS; ++i) {
            if (magic_num1[i] > 0) {
                smith_data->essences[i2enum<SmithEssenceType>(i)] = static_cast<int16_t>(magic_num1[i]);
            }
        }
    } else {
        while (true) {
            auto essence = rd_s16b();
            auto amount = rd_s16b();
            if (essence < 0 && amount < 0) {
                break;
            }
            smith_data->essences[i2enum<SmithEssenceType>(essence)] = amount;
        }
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<force_trainer_data_type> &force_trainer_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        force_trainer_data->ki = magic_num1[0];
    } else {
        force_trainer_data->ki = rd_s32b();
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<bluemage_data_type> &bluemage_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        for (int i = 0, count = std::min(enum2i(MonsterAbilityType::MAX), OLD_SAVEFILE_MAX_SPELLS); i < count; ++i) {
            bluemage_data->learnt_blue_magics.set(i2enum<MonsterAbilityType>(i), magic_num2[i] != 0);
        }
    } else {
        rd_FlagGroup(bluemage_data->learnt_blue_magics, rd_byte);
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<magic_eater_data_type> &magic_eater_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        auto load_old_item_group = [magic_num1 = std::move(magic_num1), magic_num2 = std::move(magic_num2)](auto &item_group, int index) {
            constexpr size_t old_item_group_size = 36;
            int offset = old_item_group_size * index;
            for (auto i = 0U; i < std::min(item_group.size(), old_item_group_size); ++i) {
                item_group[i].charge = magic_num1[offset + i];
                item_group[i].count = magic_num2[offset + i];
            }
        };
        load_old_item_group(magic_eater_data->staves, 0);
        load_old_item_group(magic_eater_data->wands, 1);
        load_old_item_group(magic_eater_data->rods, 2);
    } else {
        auto load_item_group = [](auto &item_group) {
            auto item_count = rd_u16b();
            for (auto i = 0U; i < item_count; ++i) {
                auto charge = rd_s32b();
                auto count = rd_byte();
                if (i < item_group.size()) {
                    item_group[i].charge = charge;
                    item_group[i].count = count;
                }
            }
        };
        load_item_group(magic_eater_data->staves);
        load_item_group(magic_eater_data->wands);
        load_item_group(magic_eater_data->rods);
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<bard_data_type> &bird_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        bird_data->singing_song = i2enum<realm_song_type>(magic_num1[0]);
        bird_data->interrputing_song = i2enum<realm_song_type>(magic_num1[1]);
        bird_data->singing_duration = magic_num1[2];
        bird_data->singing_song_spell_idx = magic_num2[0];
    } else {
        bird_data->singing_song = i2enum<realm_song_type>(rd_s32b());
        bird_data->interrputing_song = i2enum<realm_song_type>(rd_s32b());
        bird_data->singing_duration = rd_s32b();
        bird_data->singing_song_spell_idx = rd_byte();
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<mane_data_type> &mane_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // 古いセーブファイルのものまね師のデータは magic_num には保存されていないので読み捨てる
        load_old_savfile_magic_num();
    } else {
        auto count = rd_s16b();
        for (; count > 0; --count) {
            auto spell = rd_s16b();
            auto damage = rd_s16b();
            mane_data->mane_list.push_back({ i2enum<MonsterAbilityType>(spell), damage });
        }
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<SniperData> &sniper_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // 古いセーブファイルのスナイパーのデータは magic_num には保存されていないので読み捨てる
        load_old_savfile_magic_num();
    } else {
        sniper_data->concent = rd_s16b();
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<samurai_data_type> &samurai_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // 古いセーブファイルの剣術家のデータは magic_num には保存されていないので読み捨てる
        load_old_savfile_magic_num();
    } else {
        samurai_data->stance = i2enum<SamuraiStanceType>(rd_byte());
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<monk_data_type> &monk_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // 古いセーブファイルの修行僧のデータは magic_num には保存されていないので読み捨てる
        load_old_savfile_magic_num();
    } else {
        monk_data->stance = i2enum<MonkStanceType>(rd_byte());
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<ninja_data_type> &ninja_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        // 古いセーブファイルの忍者のデータは magic_num には保存されていないので読み捨てる
        load_old_savfile_magic_num();
    } else {
        ninja_data->kawarimi = rd_byte() != 0;
        ninja_data->s_stealth = rd_byte() != 0;
    }
}

void PlayerClassSpecificDataLoader::operator()(std::shared_ptr<spell_hex_data_type> &spell_hex_data) const
{
    if (loading_savefile_version_is_older_than(9)) {
        auto [magic_num1, magic_num2] = load_old_savfile_magic_num();
        migrate_bitflag_to_flaggroup(spell_hex_data->casting_spells, magic_num1[0]);
        spell_hex_data->revenge_power = magic_num1[2];
        spell_hex_data->revenge_type = i2enum<SpellHexRevengeType>(magic_num2[1]);
        spell_hex_data->revenge_turn = magic_num2[2];
    } else {
        rd_FlagGroup(spell_hex_data->casting_spells, rd_byte);
        spell_hex_data->revenge_power = rd_s32b();
        spell_hex_data->revenge_type = i2enum<SpellHexRevengeType>(rd_byte());
        spell_hex_data->revenge_turn = rd_byte();
    }
}
