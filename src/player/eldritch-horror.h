﻿#pragma once

typedef struct monster_type monster_type;
struct player_type;
void sanity_blast(player_type *creature_ptr, monster_type *m_ptr, bool necro);
