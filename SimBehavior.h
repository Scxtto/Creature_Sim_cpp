#pragma once

#include "SimCreature.h"
#include "SimEnvironment.h"

namespace CreatureBehaviour {
void updateAge(Creature& creature);
void updateCooldowns(Creature& creature);
void checkSurvival(Creature& creature);
void checkSafety(Creature& creature, Environment& environment);
void checkState(Creature& creature);
void goMate(Creature& creature, Environment& environment, Tracking& tracking);
void goHunt(Creature& creature, Environment& environment, Tracking& tracking);
void goFlee(Creature& creature);
void goRest(Creature& creature);
void goExplore(Creature& creature);
}
