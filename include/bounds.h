#ifndef __BOUNDS_H__
#define __BOUNDS_H__

#include <stdbool.h>
#include "scene.h"

/**
 * Creates three sets of 4 walls that destroy bodies when they fully go off-screen.
 * Terrain bounds, 30-radius bounds, 10-radius bounds.
 * Terrain bounds destroys terrain. 30-radius destroys player/enemies/powerups/coins.
 * 10-radius destroys fly enemies and player bullets.
 * 
 * @param scene a pointer to a scene
 * @param MIN the coordinates of the bottom-left corner of the screen
 * @param MAX the coordinate of the top-right corner of the screen
 */
void initialize_bounds(scene_t *scene, vector_t min, vector_t max);

/**
 * Causes the specified body to be destroyed when off-screen.
 * 
 * @param scene a pointer to a scene
 * @param body the body that needs to collide with bounds
 * @param radius the distance between the left-most point and the centroid of the body
 */
void create_bounds_collisions(scene_t *scene, body_t *body, double radius);

/**
 * Causes the specified terrain body to be destroyed when off-screen.
 * 
 * @param scene a pointer to a scene
 * @param body the terrain body that needs to be destroyed
 */
void create_terrain_collisions(scene_t *scene, body_t *terrain_body);

#endif // #ifndef __ENEMY_H__
