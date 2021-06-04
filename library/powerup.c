#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "sdl_wrapper.h"
#include "powerup.h"
#include "bounds.h"
#include "entity.h"
#include "forces.h"
#include "shapelib.h"

const double POWERUP_MASS = 10;
const double POWERUP_RADIUS = 20;
const int POWERUP_PADDING = 50;
const double COIN_SCORE = 10000;
const double GRAVITY_CONST = 10000000;
const char* MAGNET = "static/magnet_powerup.png";
const char* SLOW = "static/slow_powerup.png";
const char* JUMP = "static/jump_powerup.png";
const char *COIN = "static/coin_spritesheet.png";

//Removes the functionality of the previous powerup.
void remove_old_powerup(char *powerup, vector_t *scroll_speed) {
    if (!strcmp(powerup, "SLOW")) {
        scroll_speed->x = 2 * scroll_speed->x;
    }
    //for MAGNET, all remaining on-screen coins will be magnetized, but newer ones won't.
    //JUMP requires no adjustment beyond removing the tag.
}

typedef struct powerup_info {
    scene_t *scene;
    double *score;
    vector_t *scroll_speed;
    list_t *achievements;
} powerup_info_t;

typedef struct param {
    double constant;
    body_t *body1;
    body_t *body2;
} param_t;

//The gravity creator between a player with the magnet powerup and coins.
void magnet_gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1),
                              body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    force = vec_multiply(-aux->constant*mass_product / pow(sqrt(vec_dot(r,r)), 3.0) , r);
    body_add_force(aux->body1, force);
}

//Attracts coins to a player with the magnet powerup using one-way gravity.
void create_magnet_gravity(scene_t *scene, double G, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){G, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, magnet_gravity_creator, force_param, bodies, 
                                   free);
}

//Collision handler for when player collects a magnet powerup. Attracts coins to player.
void magnet_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    *(double *)list_get(info->achievements, 3) =
        *(double *)list_get(info->achievements, 3) + 1;
    scene_t *scene = info->scene;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "MAGNET")) {
        remove_old_powerup(entity_get_powerup(entity), info->scroll_speed);
        entity_set_powerup(entity, "MAGNET");
        for (size_t i = 0; i < scene_bodies(info->scene); i++) {
            body_t *body = scene_get_body(scene, i);
            if (!strcmp(entity_get_type(body_get_info(body)), "COIN")) {
                create_magnet_gravity(scene, GRAVITY_CONST, body, player);
            }
        }
    }
    body_remove(powerup);
}

//Collision handler for when player collects a slow powerup. Slows the scroll speed.
void slow_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    *(double *)list_get(info->achievements, 3) =
        *(double *)list_get(info->achievements, 3) + 1;
    vector_t *scroll_speed = info->scroll_speed;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "SLOW")) {
        remove_old_powerup(entity_get_powerup(entity), scroll_speed);
        entity_set_powerup(entity, "SLOW");
        scroll_speed->x = 0.5 * scroll_speed->x;
    }
    body_remove(powerup);
}

//Collision handler for when player collects a jump powerup. Enables multi-jump.
void jump_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    *(double *)list_get(info->achievements, 3) =
        *(double *)list_get(info->achievements, 3) + 1;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "JUMP")) {
        remove_old_powerup(entity_get_powerup(entity), info->scroll_speed);
        entity_set_powerup(entity, "JUMP");
    }
    body_remove(powerup);
}

//Collision handler for when player collects a coin. Adds the coin's score to total score.
void coin_handler(body_t *player, body_t *coin, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    *(info->score) = *(info->score) + COIN_SCORE;
    *(double *)list_get(info->achievements, 2) =
        *(double *)list_get(info->achievements, 2) + 1;
    body_remove(coin);
}

//Creates the body of a powerup and adds it to the scene.
body_t *spawn_powerup(scene_t *scene, vector_t MIN, vector_t MAX, powerup_info_t *info) {
    vector_t center = {MAX.x + POWERUP_RADIUS,
        rand()%(int)((MAX.y - MIN.y - 2*POWERUP_PADDING) + POWERUP_PADDING)};
    entity_t *entity = entity_init("POWERUP", true, false);
    list_t *powerup_coords = compute_rect_points(center, 2*POWERUP_RADIUS,
                                                 2*POWERUP_RADIUS);
    body_t *powerup = body_init_with_info(powerup_coords, POWERUP_MASS, entity,
                                          entity_free);
    scene_add_body(scene, powerup);
    return powerup;
}

void powerup_spawn_random(scene_t *scene, vector_t MIN, vector_t MAX,
                          vector_t *scroll_speed, list_t *achievements) {
    powerup_info_t *info = malloc(sizeof(powerup_info_t));
    info->scene = scene;
    info->scroll_speed = scroll_speed;
    info->achievements = achievements;
    int percent_max = 100;
    int percent_magnet = 20;
    int percent_slow = 60;
    int percent_jump = 100;
    int random_powerup = rand()%percent_max;
    body_t *player = scene_get_body(scene, 3);
    body_t *powerup = spawn_powerup(scene, MIN, MAX, info);
    create_bounds_collisions(scene, powerup, POWERUP_RADIUS);
    if (random_powerup <= percent_magnet) {
        sprite_t *magnet_info = sprite_animated(MAGNET, 1, 1, 1);
        body_set_draw(powerup, (draw_func_t) sdl_draw_animated, magnet_info, sprite_free);
        create_collision(scene, player, powerup, magnet_handler, info, free);
    }
    else if (random_powerup <= percent_slow) {
        sprite_t *slow_info = sprite_animated(SLOW, 1, 1, 1);
        body_set_draw(powerup, (draw_func_t) sdl_draw_animated, slow_info, sprite_free);
        create_collision(scene, player, powerup, slow_handler, info, free);
    }
    else if (random_powerup <= percent_jump) {
        sprite_t *jump_info = sprite_animated(JUMP, 1, 1, 1);
        body_set_draw(powerup, (draw_func_t) sdl_draw_animated, jump_info, sprite_free);
        create_collision(scene, player, powerup, jump_handler, info, free);
    }
}

void powerup_spawn_coin(scene_t *scene, vector_t center, double *score,
                        list_t *achievements) {
    body_t *player = scene_get_body(scene, 3);
    powerup_info_t *info = malloc(sizeof(powerup_info_t));
    info->score = score;
    info->achievements = achievements;
    entity_t *entity = entity_init("COIN", true, false);
    list_t *coin_coords = compute_rect_points(center, 2*POWERUP_RADIUS,
                                                 2*POWERUP_RADIUS);
    body_t *coin = body_init_with_info(coin_coords, POWERUP_MASS, entity,
                                          entity_free);
    scene_add_body(scene, coin);
    sprite_t *coin_info = sprite_animated(COIN, 1, 6, 6);
    body_set_draw(coin, (draw_func_t) sdl_draw_animated, coin_info, sprite_free);
    create_collision(scene, player, coin, coin_handler, info, free);
    if (!strcmp(entity_get_powerup(body_get_info(player)), "MAGNET")) {
        create_magnet_gravity(scene, GRAVITY_CONST, coin, player);
    }
}
