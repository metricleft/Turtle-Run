#ifndef __POWERUP_H__
#define __POWERUP_H__

#include <stdbool.h>
#include "body.h"
#include "entity.h"
#include "scene.h"
#include "forces.h"
#include "shapelib.h"

/**
 * Spawns a random powerup beyond the right side of the screen.
 * 
 * @param scene a pointer to a scene
 * @param MIN the coordinates of the bottom-left corner of the screen
 * @param MAX the coordinate of the top-right corner of the screen
 * @param scroll_speed the scroll velocity of the game
 */
void powerup_spawn_random(scene_t *scene, vector_t MIN, vector_t MAX,
                          vector_t *scroll_speed);

#endif // #ifndef __POWERUP_H__
