#pragma once

#include "system/angband.h"

enum class MimicKindType;
class PlayerType;
void reset_tim_flags(PlayerType *player_ptr);
bool set_acceleration(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool mod_acceleration(PlayerType *player_ptr, const TIME_EFFECT v, const bool do_dec);
bool set_shield(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_magicdef(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_blessed(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_hero(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_mimic(PlayerType *player_ptr, TIME_EFFECT v, MimicKindType mimic_race_idx, bool do_dec);
bool set_shero(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_wraith_form(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
bool set_tsuyoshi(PlayerType *player_ptr, TIME_EFFECT v, bool do_dec);
