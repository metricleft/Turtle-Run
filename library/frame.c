#include "frame.h"
#include "bounds.h"
#include "powerup.h"

const int TERRAIN_HEIGHT = 50;
const int PLATFORM_HEIGHT = 10;
const int TERRAIN_PAD = 10;
const vector_t NORMAL_GRAV = {0, 800};

/**
 * Creates a block of terrain in the specified position
 * @param scene the scene to add the terrain to
 * @param center vector_t to the center of the rectangular block
 * @param width width of the rectangle
 * @param height height of the rectangle
 */
void create_terrain_rect(scene_t *scene, vector_t center,
                            double width, double height) {
    entity_t *entity = entity_init("TERRAIN", true, false);
    list_t *rect_coords = compute_rect_points(center, width, height);
    body_t *body = body_init_with_info(rect_coords, INFINITY,
                                            entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(body, sdl_draw_polygon, black, free);
    scene_add_body(scene, body);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene,3), body);
    create_terrain_collisions(scene, body);
}

/**
 * Creates a platform in the specified position
 * @param scene the scene to add the terrain to
 * @param center vector_t to the center of the rectangular block
 * @param width width of the rectangle
 * @param height height of the rectangle
 */
void create_platform(scene_t *scene, vector_t center,
                            double width, double height) {
    entity_t *entity = entity_init("PLATFORM", true, false);
    list_t *rect_coords = compute_rect_points(center, width, height);
    body_t *body = body_init_with_info(rect_coords, INFINITY,
                                            entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(body, sdl_draw_polygon, black, free);
    scene_add_body(scene, body);
    create_normal_collision(scene, NORMAL_GRAV, scene_get_body(scene,3), body);
    create_terrain_collisions(scene, body);
}

/**
 * Simple frame that consists of a black box that acts as the floor.
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_0(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t){0.5*frame.x+frame_start, TERRAIN_PAD};
    create_terrain_rect(scene, floor_center, frame.x, TERRAIN_HEIGHT);
}

/**
 * Frame with a pitfall that occupies the first 2/5 of the ground
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_1(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t) {0.7*frame.x+frame_start, TERRAIN_PAD};
    create_terrain_rect(scene, floor_center, 0.6*frame.x, TERRAIN_HEIGHT);
    powerup_spawn_coin(scene, (vector_t){0.625*frame.x+frame_start, 2*TERRAIN_HEIGHT},
                       score);
}

/**
 * Frame with 3 platforms in a staircase formation
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_2(scene_t *scene, vector_t frame, double frame_start, double *score) {
    vector_t floor_center = (vector_t) {0.5*frame.x+frame_start, TERRAIN_PAD};
    create_terrain_rect(scene, floor_center, frame.x, TERRAIN_HEIGHT);
    powerup_spawn_coin(scene, (vector_t){frame.x+frame_start, 2*TERRAIN_HEIGHT},
                       score);

    vector_t platform1_center = (vector_t){0.25*frame.x+frame_start,
                                           0.25*frame.y+TERRAIN_PAD};
    create_platform(scene, platform1_center, frame.x/6., PLATFORM_HEIGHT);
    powerup_spawn_coin(scene,
                       (vector_t){0.25*frame.x+frame_start,
                                  0.25*frame.y+TERRAIN_PAD+TERRAIN_HEIGHT}, score);
    
    vector_t platform2_center = (vector_t){0.5*frame.x + frame_start, 0.5*frame.y};
    create_platform(scene, platform2_center, frame.x/6., PLATFORM_HEIGHT);
    powerup_spawn_coin(scene, (vector_t){0.5*frame.x+frame_start,
                                         0.5*frame.y+TERRAIN_HEIGHT}, score);

    vector_t platform3_center = (vector_t){0.75*frame.x + frame_start,
                                            0.75*frame.y - TERRAIN_PAD};
    create_platform(scene, platform3_center, frame.x/6., PLATFORM_HEIGHT);
}

/**
 * Frame with 2 platforms, no floor
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_3(scene_t *scene, vector_t frame, double frame_start, double *score){
    vector_t platform1_center = {0.25*frame.x+frame_start, 0.25*frame.y};
    create_platform(scene, platform1_center, 0.5*frame.x, PLATFORM_HEIGHT);

    vector_t platform2_center = {0.75*frame.x+frame_start, 0.5*frame.y};
    create_platform(scene, platform2_center, 0.5*frame.x, PLATFORM_HEIGHT);
}

/**
 * Small pitfall with blocking terrain, forcing a jump
 * @param scene the scene to add the frame to
 * @param frame a vector_t describing the size of the frame.
 * @param frame_start the x coordinate starting point of the frame
 */
void frame_4(scene_t *scene, vector_t frame, double frame_start, double *score){
    vector_t floor_center = {0.625*frame.x+frame_start, TERRAIN_PAD};
    create_terrain_rect(scene, floor_center, 0.75*frame.x, TERRAIN_HEIGHT);

    vector_t wall_center = {0.625*frame.x+frame_start, 0.15*frame.y};
    create_terrain_rect(scene, wall_center, 0.25*frame.x, 0.25*frame.y);
}

void frame_spawn_random(scene_t *scene, vector_t frame, double frame_start,
                        double *score) {
    int frame_num;

    frame_num = rand() % 5;
    /* //Stuff for debugging frames
    printf("%d\n",frame_num);

    vector_t debug_center = (vector_t) {frame_start, 50};
    entity_t *debug_entity = entity_init("TERRAIN",true,false);
    list_t *debug_coords = compute_rect_points(debug_center, 10, 50);
    body_t *debug_body = body_init_with_info(debug_coords, INFINITY, debug_entity, entity_free);
    rgb_color_t *red = malloc(sizeof(rgb_color_t));
    *red = RED;
    body_set_draw(debug_body, sdl_draw_polygon, red, free);
    scene_add_body(scene, debug_body);
    create_terrain_collisions(scene,debug_body);
    */
    frame_4(scene,frame,frame_start,score);
    /*
    if (frame_num == 0) {
        frame_0(scene, frame, frame_start, score);
    }
    if (frame_num == 1) {
        frame_1(scene,frame,frame_start, score);
    }
    if (frame_num == 2) {
        frame_2(scene, frame, frame_start, score);
    }
    if (frame_num == 3) {
        frame_3(scene, frame, frame_start, score);
    }
    if (frame_num == 4) {
        frame_4(scene, frame, frame_start, score);
    }
    */
}
