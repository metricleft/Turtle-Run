#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <stdbool.h>
#include "body.h"
#include "entity.h"
#include "scene.h"
#include "forces.h"
#include "shapelib.h"

/**
 * Spawns a random enemy beyond the right side of the screen.
 * 
 * @param scene a pointer to a scene
 * @param MIN the coordinates of the bottom-left corner of the screen
 * @param MAX the coordinate of the top-right corner of the screen
 * @param enemy_radius the radius of the enemy
 */
void spawn_random_enemy(scene_t *scene, vector_t MIN, vector_t MAX, double enemy_radius);

#endif // #ifndef __ENEMY_H__
