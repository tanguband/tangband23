﻿#pragma once
/*!
 * @file activation-execution.h
 * @brief アイテムの発動実行ヘッダ
 */

#include "system/angband.h"

class CapturedMonsterType;
class PlayerType;
void exe_activate(PlayerType *player_ptr, INVENTORY_IDX item, CapturedMonsterType *cap_mon_ptr);
