#ifndef __FRAME_H__
#define __FRAME_H__

#include "scene.h"

/**
 * Spawns a random frame of terrain and enemies from a certain point
 * @param scene the scene to spawn the frame in
 * @param frame_width the width of the frame to be spawned
 * @param frame_start the start point of the frame
 * @param score a pointer to the score of the game
 * @param achievements the achievements for the current game
 */
void frame_spawn_random(scene_t *scene, vector_t frame, double frame_start,
                        double *score, list_t *achievements);

#endif