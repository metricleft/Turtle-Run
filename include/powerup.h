#ifndef __POWERUP_H__
#define __POWERUP_H__

#include <stdbool.h>
#include "scene.h"

/**
 * Spawns a random powerup beyond the right side of the screen.
 * 
 * @param scene a pointer to a scene
 * @param MIN the coordinates of the bottom-left corner of the screen
 * @param MAX the coordinate of the top-right corner of the screen
 * @param scroll_speed the scroll velocity of the game
 * @param achievements the achievements for the current game
 */
void powerup_spawn_random(scene_t *scene, vector_t MIN, vector_t MAX,
                          vector_t *scroll_speed, list_t *achievements);

/**
 * Spawns a coin at a specified position.
 * 
 * @param scene a pointer to a scene
 * @param center the center of the coin to be spawned
 * @param score a pointer to the score of the game
 * @param achievements the achievements for the current game
 */
void powerup_spawn_coin(scene_t *scene, vector_t center, double *score,
                        list_t *achievements);

#endif // #ifndef __POWERUP_H__
