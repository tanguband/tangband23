﻿#pragma once

#include "system/angband.h"

#include "object-enchant/tr-flags.h"
#include "object-enchant/trg-types.h"
#include "system/object-type-definition.h"
#include "util/flag-group.h"
#include <map>
#include <string>

/*!
 * @class ArtifactType
 * @brief 固定アーティファクト情報の構造体 / Artifact structure.
 * @details is_generated とfloor_id フィールドのみセーブファイルへの保存対象
 */
enum class FixedArtifactId : short;
enum class RandomArtActType : short;
class ArtifactType {
public:
    ArtifactType() = default;

    std::string name; /*!< アーティファクト名 / Name */
    std::string text; /*!< アーティファクト解説 / Text */
    ItemKindType tval{}; /*!< ベースアイテム大項目ID / Artifact type */
    OBJECT_SUBTYPE_VALUE sval{}; /*!< ベースアイテム小項目ID / Artifact sub type */
    PARAMETER_VALUE pval{}; /*!< pval修正値 / Artifact extra info */
    HIT_PROB to_h{}; /*!< 命中ボーナス値 /  Bonus to hit */
    int to_d{}; /*!< ダメージボーナス値 / Bonus to damage */
    ARMOUR_CLASS to_a{}; /*!< ACボーナス値 / Bonus to armor */
    ARMOUR_CLASS ac{}; /*!< 上書きベースAC値 / Base armor */
    DICE_NUMBER dd{};
    DICE_SID ds{}; /*!< ダイス値 / Damage when hits */
    WEIGHT weight{}; /*!< 重量 / Weight */
    PRICE cost{}; /*!< 基本価格 / Artifact "cost" */
    TrFlags flags{}; /*! アイテムフラグ / Artifact Flags */
    EnumClassFlagGroup<ItemGenerationTraitType> gen_flags; /*! アイテム生成フラグ / flags for generate */
    DEPTH level{}; /*! 基本生成階 / Artifact level */
    RARITY rarity{}; /*! レアリティ / Artifact rarity */
    bool is_generated{}; /*! 生成済か否か (生成済でも、「保存モードON」かつ「帰還等で鑑定前に消滅」したら未生成状態に戻る) */
    FLOOR_IDX floor_id{}; /*! アイテムを落としたフロアのID / Leaved on this location last time */
    RandomArtActType act_idx{}; /*! 発動能力ID / Activative ability index */
};

extern std::map<FixedArtifactId, ArtifactType> a_info;
