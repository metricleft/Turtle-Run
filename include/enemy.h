#ifndef __ENEMY_H__
#define __ENEMY_H__

#include <stdbool.h>
#include "scene.h"

/**
 * Spawns a random enemy beyond the right side of the screen.
 * 
 * @param scene a pointer to a scene
 * @param MIN the coordinates of the bottom-left corner of the screen
 * @param MAX the coordinate of the top-right corner of the screen
 */
void enemy_spawn_random(scene_t *scene, vector_t MIN, vector_t MAX);

#endif // #ifndef __ENEMY_H__
