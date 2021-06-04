#include "bounds.h"
#include "entity.h"
#include "forces.h"
#include "shapelib.h"

const int BOUNDS_THICKNESS = 30;

void create_bounds_collisions(scene_t *scene, body_t *body, double radius) {
    //bounds walls range from indices 30-radius: 5-8, 10-radius: 9-12
    int lowerbound = 5;
    int upperbound = 8;
    if (radius <= 10) {
        lowerbound += 4;
        upperbound += 4;
    }
    for (size_t i = lowerbound; i <= upperbound; i++) {
        create_oneway_destructive_collision(scene, 0, scene_get_body(scene, i), body);
    }
}

void create_terrain_collisions(scene_t *scene, body_t *terrain_body) {
    create_oneway_destructive_collision(scene, 0, scene_get_body(scene, 4), terrain_body);
}

//Create four permanent off-screen walls.
void create_bounds(scene_t *scene, vector_t min, vector_t max, double radius) {
    //Left:
    vector_t center = (vector_t){-radius*2 - BOUNDS_THICKNESS/2, (max.y-min.y)/2};
    entity_t *entity = entity_init("BOUNDS", false, false);
    list_t *coords = compute_rect_points(center, BOUNDS_THICKNESS, max.y - min.y);
    body_t *bounds = body_init_with_info(coords, INFINITY, entity, entity_free);
    scene_add_body(scene, bounds);

    //Top:
    center = (vector_t){(max.x - min.x)/2, max.y + radius*2 + BOUNDS_THICKNESS/2};
    entity = entity_init("BOUNDS", false, false);
    coords = compute_rect_points(center, max.x - min.x, BOUNDS_THICKNESS);
    bounds = body_init_with_info(coords, INFINITY, entity, entity_free);
    scene_add_body(scene, bounds);

    //Bottom:
    center = (vector_t){(max.x - min.x)/2, -radius*2 - BOUNDS_THICKNESS/2};
    entity = entity_init("BOUNDS", false, false);
    coords = compute_rect_points(center, max.x - min.x, BOUNDS_THICKNESS);
    bounds = body_init_with_info(coords, INFINITY, entity, entity_free);
    scene_add_body(scene, bounds);

    //Right:
    center = (vector_t){max.x + radius*4 + BOUNDS_THICKNESS/2, (max.y-min.y)/2};
    entity = entity_init("BOUNDS", false, false);
    coords = compute_rect_points(center, BOUNDS_THICKNESS, max.y - min.y);
    bounds = body_init_with_info(coords, INFINITY, entity, entity_free);
    scene_add_body(scene, bounds);
}

void initialize_bounds(scene_t *scene, vector_t min, vector_t max) {
    //Terrain bounds:
    vector_t center = (vector_t){-(max.x-min.x) - BOUNDS_THICKNESS/2, (max.y-min.y)/2};
    entity_t *entity = entity_init("BOUNDS", false, false);
    list_t *coords = compute_rect_points(center, BOUNDS_THICKNESS, max.y - min.y);
    body_t *bounds = body_init_with_info(coords, INFINITY, entity, entity_free);
    scene_add_body(scene, bounds);

    create_bounds(scene, min, max, 30);
    create_bounds(scene, min, max, 10);
}