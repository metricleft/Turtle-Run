#include "frame.h"
#include "bounds.h"
#include "powerup.h"

const int TERRAIN_HEIGHT = 50;
const int PLATFORM_HEIGHT = 10;
const int TERRAIN_PAD = 10;
const vector_t NORMAL_GRAV = {0, 500};

/**
 * Simple frame that consists of a black box that acts as the floor.
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_0(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t){0.5*frame.x+frame_start, TERRAIN_PAD};
    entity_t *entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(floor_center, frame.x, TERRAIN_HEIGHT);
    body_t *floor = body_init_with_info(floor_coords, INFINITY,
                                                entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, floor);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene, 3), floor);
    create_terrain_collisions(scene, floor);
}

/**
 * Frame with a pitfall that occupies the first 1/4 of the ground
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_1(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t) {0.625*frame.x+frame_start, TERRAIN_PAD};
    entity_t *floor_entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(floor_center,0.75*frame.x,TERRAIN_HEIGHT);
    body_t *floor = body_init_with_info(floor_coords, INFINITY,
                                            floor_entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, floor);
    powerup_spawn_coin(scene, (vector_t){0.625*frame.x+frame_start, 2*TERRAIN_HEIGHT},
                       score);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene, 3), floor);
    create_terrain_collisions(scene, floor);
}

/**
 * Frame with 3 platforms in a staircase formation
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_2(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t) {0.625*frame.x+frame_start, TERRAIN_PAD};
    entity_t *floor_entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(floor_center,frame.x,TERRAIN_HEIGHT);
    body_t *floor = body_init_with_info(floor_coords, INFINITY,
                                            floor_entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, floor);
    powerup_spawn_coin(scene, (vector_t){0.625*frame.x+frame_start, 2*TERRAIN_HEIGHT},
                       score);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene, 3),floor);
    create_terrain_collisions(scene, floor);

    vector_t platform1_center = (vector_t){0.25*frame.x+frame_start,
                                           0.25*frame.y+TERRAIN_PAD};
    entity_t *platform1_entity = entity_init("TERRAIN",true,false);
    list_t *platform1_coords = compute_rect_points(platform1_center,frame.x/8.,
                                                   PLATFORM_HEIGHT);
    body_t *platform1 = body_init_with_info(platform1_coords,INFINITY,
                                                platform1_entity,entity_free);
    rgb_color_t *black1 = malloc(sizeof(rgb_color_t));
    *black1 = BLACK;
    body_set_draw(platform1,(draw_func_t) sdl_draw_polygon,black1,free);
    scene_add_body(scene,platform1);
    powerup_spawn_coin(scene,
                       (vector_t){0.25*frame.x+frame_start,
                                  0.25*frame.y+TERRAIN_PAD+TERRAIN_HEIGHT}, score);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene, 3),platform1);
    create_terrain_collisions(scene, platform1);
    
    vector_t platform2_center = (vector_t){0.5*frame.x + frame_start, 0.5*frame.y};
    entity_t *platform2_entity = entity_init("TERRAIN",true,false);
    list_t *platform2_coords = compute_rect_points(platform2_center, frame.x/8.,
                                                   TERRAIN_PAD);
    body_t *platform2 = body_init_with_info(platform2_coords,INFINITY,
                                                platform2_entity,entity_free);
    rgb_color_t *black2 = malloc(sizeof(rgb_color_t));
    *black2 = BLACK;
    body_set_draw(platform2,(draw_func_t) sdl_draw_polygon,black2,free);
    scene_add_body(scene,platform2);
    powerup_spawn_coin(scene, (vector_t){0.5*frame.x+frame_start,
                                         0.5*frame.y+TERRAIN_HEIGHT}, score);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene, 3),platform2);
    create_terrain_collisions(scene, platform2);
}

void frame_spawn_random(scene_t *scene, vector_t frame, double frame_start,
                        double *score) {
    int frame_num;

    frame_num = rand() % 3;
    if (frame_num == 0) {
        frame_0(scene, frame, frame_start, score);
    }
    if (frame_num == 1) {
        frame_1(scene,frame,frame_start, score);
    }
    if (frame_num == 2) {
        frame_2(scene, frame, frame_start, score);
    }
}
