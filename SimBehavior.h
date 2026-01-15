#pragma once

#include "SimCreature.h"
#include "SimEnvironment.h"

/** @brief Creature behavior utilities used by the simulation. */
namespace CreatureBehaviour {
/**
 * @brief Increment creature age by its configured rate.
 * @param creature Creature to update.
 */
void updateAge(Creature& creature);
/**
 * @brief Decrement reproduction cooldown when active.
 * @param creature Creature to update.
 */
void updateCooldowns(Creature& creature);
/**
 * @brief Check hunger, age, and health survival conditions.
 * @param creature Creature to update.
 */
void checkSurvival(Creature& creature);
/**
 * @brief Update fleeing state based on nearby predators.
 * @param creature Creature to update.
 * @param environment Environment containing predators.
 */
void checkSafety(Creature& creature, Environment& environment);
/**
 * @brief Decide creature state (hunting, mating, resting, exploring).
 * @param creature Creature to update.
 */
void checkState(Creature& creature);
/**
 * @brief Move toward mate and reproduce when in range.
 * @param creature Creature to update.
 * @param environment Environment containing mates.
 * @param tracking Per-tick tracking accumulator.
 * @note Newly spawned creatures are stored in \c tracking.newborns.
 */
void goMate(Creature& creature, Environment& environment, Tracking& tracking);
/**
 * @brief Seek food or prey and consume when close enough.
 * @param creature Creature to update.
 * @param environment Environment containing food and prey.
 * @param tracking Per-tick tracking accumulator.
 */
void goHunt(Creature& creature, Environment& environment, Tracking& tracking);
/**
 * @brief Flee directly away from the current predator.
 * @param creature Creature to update.
 */
void goFlee(Creature& creature);
/**
 * @brief Recover from tiredness and reset rest state.
 * @param creature Creature to update.
 */
void goRest(Creature& creature);
/**
 * @brief Explore the environment with a wandering direction.
 * @param creature Creature to update.
 */
void goExplore(Creature& creature);
}
