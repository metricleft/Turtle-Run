#include <stdlib.h>
#include <stdbool.h>
#include "sdl_wrapper.h"
#include <string.h>
#include <math.h>

#include "enemy.h"

const int GAME_ENEMY_MASS = 10;
const char* FROG = "static/frog_spritesheet.png";
const char* FLY = "static/dragonfly_spritesheet.png";

void create_bullet_collisions(scene_t *scene, body_t *enemy) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        if (!strcmp(entity_get_type(entity), "BULLET")) {
            create_destructive_collision(scene, body, enemy);
        }
    }
}

void spawn_goose(scene_t *scene, vector_t MIN, vector_t MAX, double radius) {
    //Spawns a goose that flies across the screen, speeding up
    double *drag_const = malloc(sizeof(double));
    *drag_const = -10;

    body_t *player = scene_get_body(scene, 0);
    vector_t center = {MAX.x + radius, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", true, false);
    list_t *goose_coords = compute_rect_points(center, 2*radius, 2*radius);
    body_t *goose = body_init_with_info(goose_coords, GAME_ENEMY_MASS, entity, entity_free);
    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(goose, (draw_func_t) sdl_draw_polygon, black, free);
    create_drag(scene, drag_const, goose, free);
    scene_add_body(scene, goose);
    create_destructive_collision(scene, player, goose);
    create_bullet_collisions(scene, goose);
}

void spawn_frog(scene_t *scene, vector_t MIN, vector_t MAX, double radius) {
    //Spawns a frog that bounces up and down the screen
    double *spring_const = malloc(sizeof(double));
    *spring_const = 20;

    body_t *player = scene_get_body(scene, 0);

    vector_t center = {MAX.x + radius, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", false, false);
    list_t *frog_coords = compute_rect_points(center, 2*radius, 2*radius);
    body_t *frog = body_init_with_info(frog_coords, GAME_ENEMY_MASS, entity, entity_free);
    sprite_t *frog_info = sprite_animated(FROG, 1, 8, 6);
    body_set_draw(frog, (draw_func_t) sdl_draw_animated, frog_info, sprite_free);
    center.y = MAX.y / 2;
    entity = entity_init("ANCHOR", true, false);
    list_t *anchor_coords = compute_circle_points(center, radius, radius);
    body_t *anchor = body_init_with_info(anchor_coords, INFINITY, entity, entity_free);

    create_spring(scene, spring_const, anchor, frog, free);

    scene_add_body(scene, frog);
    scene_add_body(scene, anchor);
    create_destructive_collision(scene, player, frog);
    create_bullet_collisions(scene, frog);
}

typedef struct param {
    double constant;
    body_t *body1;
    body_t *body2;
} param_t;

void one_way_gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1), body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    force = vec_multiply(- aux->constant * mass_product / pow(sqrt(vec_dot(r,r)), 3.0) , r);
    body_add_force(aux->body1, force);
}

//Only applies newtonian gravity to body1
void create_one_way_gravity(scene_t *scene, double G, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){G, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, one_way_gravity_creator, force_param, bodies, free);
}

void spawn_fly(scene_t *scene, vector_t MIN, vector_t MAX, double radius) {
    //Spawns a fly that lazily follows the player
    double gravity_const = 1000000;

    body_t *player = scene_get_body(scene, 0);
    vector_t center = {MAX.x + radius, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", true, false);
    list_t *fly_coords = compute_rect_points(center, 2*radius, 2*radius);
    body_t *fly = body_init_with_info(fly_coords, GAME_ENEMY_MASS,  entity, entity_free);
    sprite_t *fly_info = sprite_animated(FLY, 1, 2, 20);
    body_set_draw(fly, (draw_func_t) sdl_draw_animated, fly_info, sprite_free);
    create_one_way_gravity(scene, gravity_const, fly, player);
    scene_add_body(scene, fly);
    create_destructive_collision(scene, player, fly);
    create_bullet_collisions(scene, fly);
}

void spawn_random_enemy(scene_t *scene, vector_t MIN, vector_t MAX, double enemy_radius) {
    int percent_max = 100;
    int percent_goose = 10;
    int percent_frog = 60;
    int percent_fly = 100;
    int random_enemy = rand()%percent_max;
    if (random_enemy <= percent_goose) {
        spawn_goose(scene, MIN, MAX, enemy_radius);
    }
    else if (random_enemy <= percent_frog) {
        spawn_frog(scene, MIN, MAX, enemy_radius);
    }
    else if (random_enemy <= percent_fly) {
        spawn_fly(scene, MIN, MAX, enemy_radius);
    }
}
