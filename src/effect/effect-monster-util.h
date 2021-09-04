﻿#pragma once

#include "system/angband.h"

typedef struct grid_type grid_type;
typedef struct monster_type monster_type;
typedef struct monster_race monster_race;
typedef struct effect_monster_type {
    grid_type *g_ptr;
    monster_type *m_ptr;
    monster_type *m_caster_ptr;
    monster_race *r_ptr;
    char killer[MAX_MONSTER_NAME];
    bool seen;
    bool seen_msg;
    bool slept;
    bool obvious;
    bool known;
    bool skipped;
    bool get_angry;
    bool do_polymorph;
    int do_dist;
    int do_conf;
    int do_stun;
    int do_sleep;
    int do_fear;
    int do_time;
    bool heal_leper;
    GAME_TEXT m_name[MAX_NLEN];
    char m_poss[10];
    PARAMETER_VALUE photo;
    concptr note;
    concptr note_dies;
    DEPTH caster_lev;

    MONSTER_IDX who;
    POSITION r;
    POSITION y;
    POSITION x;
    HIT_POINT dam;
    EFFECT_ID effect_type;
    BIT_FLAGS flag;
    bool see_s_msg;
} effect_monster_type;

struct player_type;
effect_monster_type *initialize_effect_monster(player_type *caster_ptr, effect_monster_type *em_ptr, MONSTER_IDX who, POSITION r, POSITION y, POSITION x,
    HIT_POINT dam, EFFECT_ID effect_type, BIT_FLAGS flag, bool see_s_msg);
