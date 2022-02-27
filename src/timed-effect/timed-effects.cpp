﻿#include "timed-effect/timed-effects.h"
#include "timed-effect/player-confusion.h"
#include "timed-effect/player-cut.h"
#include "timed-effect/player-hallucination.h"
#include "timed-effect/player-stun.h"

TimedEffects::TimedEffects()
    : player_cut(std::make_shared<PlayerCut>())
    , player_stun(std::make_shared<PlayerStun>())
    , player_confusion(std::make_shared<PlayerConfusion>())
    , player_hallucination(std::make_shared<PlayerHallucination>())
{
}

std::shared_ptr<PlayerCut> TimedEffects::cut() const
{
    return this->player_cut;
}

std::shared_ptr<PlayerStun> TimedEffects::stun() const
{
    return this->player_stun;
}

std::shared_ptr<PlayerConfusion> TimedEffects::confusion() const
{
    return this->player_confusion;
}

std::shared_ptr<PlayerHallucination> TimedEffects::hallucination() const
{
    return this->player_hallucination;
}
