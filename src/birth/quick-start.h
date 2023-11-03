#pragma once

#include "player-info/class-types.h"
#include "player-info/race-types.h"
#include "player/player-personality-types.h"
#include "player/player-sex.h"
#include "system/angband.h"
#include "system/system-variables.h"

/*
 * A structure to hold "rolled" information
 */
enum class Virtue : short;
struct birther {
    player_sex psex{}; /* Sex index */
    PlayerRaceType prace{}; /* Race index */
    PlayerClassType pclass{}; /* Class index */
    player_personality_type ppersonality{}; /* Seikaku index */
    int16_t realm1{}; /* First magic realm */
    int16_t realm2{}; /* Second magic realm */

    int16_t age{};
    int16_t ht{};
    int16_t wt{};
    int16_t sc{};

    PRICE au{}; /*!< 初期の所持金 */

    short stat_max[6]{}; /* Current "maximal" stat values */
    short stat_max_max[6]{}; /* Maximal "maximal" stat values */
    int player_hp[PY_MAX_LEVEL]{};

    int16_t chaos_patron{}; /*! カオスパトロンのID */

    Virtue vir_types[8]{};

    char history[4][60]{};

    bool quick_ok{};
};

extern birther previous_char;

class PlayerType;
bool ask_quick_start(PlayerType *player_ptr);
void save_prev_data(PlayerType *player_ptr, birther *birther_ptr);
void load_prev_data(PlayerType *player_ptr, bool swap);
