#include "frame.h"

/**
 * Simple frame that consists of a black box that acts as the floor.
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_0(scene_t *scene, vector_t frame, double frame_start) {
    vector_t new_floor_center = (vector_t){0.5*frame.x+frame_start, 10};
    entity_t *entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(new_floor_center,frame.x,50);
    body_t *new_floor = body_init_with_info(floor_coords, INFINITY,
                                                entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(new_floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, new_floor);
    create_normal_collision(scene, (vector_t) {0, 500},
                            scene_get_body(scene,0), new_floor);
}

/**
 * Frame with a pitfall that occupies the first 1/4 of the ground
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_1(scene_t *scene, vector_t frame, double frame_start) {
    vector_t floor_center = (vector_t) {0.625*frame.x+frame_start, 10};
    entity_t *floor_entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(floor_center,0.75*frame.x,50);
    body_t *floor = body_init_with_info(floor_coords, INFINITY,
                                            floor_entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, floor);
    create_normal_collision(scene, (vector_t) {0,500},
                                scene_get_body(scene,0), floor);
}

/**
 * Frame with 3 platforms in a staircase formation
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_2(scene_t *scene, vector_t frame, double frame_start) {
    vector_t floor_center = (vector_t) {0.625*frame.x+frame_start, 10};
    entity_t *floor_entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(floor_center,0.75*frame.x,50);
    body_t *floor = body_init_with_info(floor_coords, INFINITY,
                                            floor_entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
    scene_add_body(scene, floor);
    create_normal_collision(scene, (vector_t) {0,500},
                                scene_get_body(scene,0), floor);

    vector_t platform1_center = (vector_t){0.25*frame.x+frame_start,0.25*frame.y};
    entity_t *platform1_entity = entity_init("TERRAIN",true,false);
    list_t *platform1_coords = compute_rect_points(platform1_center,frame.x/8.,10);
    body_t *platform1 = body_init_with_info(platform1_coords,INFINITY,
                                                platform1_entity,entity_free);
    rgb_color_t *black1 = malloc(sizeof(rgb_color_t));
    *black1 = BLACK;
    body_set_draw(platform1,(draw_func_t) sdl_draw_polygon,black1,free);
    scene_add_body(scene,platform1);
    create_normal_collision(scene, (vector_t) {0,500},
                                scene_get_body(scene,0),platform1);
}

void frame_spawn_random(scene_t *scene, vector_t frame, double frame_start) {
    int frame_num;
    time_t t;

    srand((unsigned) time(&t));

    frame_num = rand() % 3;
    // if (frame_num == 0) {
    //     frame_0(scene, frame, frame_start);
    // }
    // if (frame_num == 1) {
    //     frame_1(scene,frame,frame_start);
    // }
    // if (frame_num == 2) {
    //     frame_2(scene, frame, frame_start);
    // }
    frame_2(scene,frame,frame_start);
}


void frame_remove_bodies(scene_t *scene, list_t *frame) {
    for (int i = 0; i < list_size(frame); i++) {
        for (int j = 0; j < scene_bodies(scene); j++) {
            if (scene_get_body(scene, j) == list_get(frame,j)) {
                scene_remove_body(scene, j);
            }
        }
    }
}