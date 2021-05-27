#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "sdl_wrapper.h"
#include "powerup.h"

const double POWERUP_MASS = 0.2;
const double POWERUP_RADIUS = 15;
const int POWERUP_PADDING = 50;
const char* MAGNET = "static/magnet_powerup.png";
const char* SLOW = "static/slow_powerup.png";
const char* JUMP = "static/jump_powerup.png";

void remove_old_powerup(char *powerup, vector_t *scroll_speed) {
    if (!strcmp(powerup, "SLOW")) {
        scroll_speed->x = 2 * scroll_speed->x;
    }
    //for MAGNET, all remaining on-screen coins will be magnetized, but newer ones won't.
    //JUMP requires no adjustment beyond removing the tag.
}

typedef struct powerup_info {
    scene_t *scene;
    vector_t *scroll_speed;
} powerup_info_t;

typedef struct param {
    double constant;
    body_t *body1;
    body_t *body2;
} param_t;

void magnet_gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1),
                              body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    force = vec_multiply(-aux->constant*mass_product / pow(sqrt(vec_dot(r,r)), 3.0) , r);
    body_add_force(aux->body1, force);
}

//Only applies newtonian gravity to body1
void create_magnet_gravity(scene_t *scene, double G, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){G, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, magnet_gravity_creator, force_param, bodies, 
                                   free);
}

void magnet_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    double gravity_const = 1000000;

    powerup_info_t *info = aux;
    scene_t *scene = info->scene;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "MAGNET")) {
        remove_old_powerup(entity_get_powerup(entity), info->scroll_speed);
        entity_set_powerup(entity, "MAGNET");
        for (size_t i = 0; i < scene_bodies(info->scene); i++) {
            body_t *body = scene_get_body(scene, i);
            if (!strcmp(entity_get_type(body_get_info(body)), "COIN")) {
                create_magnet_gravity(scene, gravity_const, player, body);
            }
        }
    }
    body_remove(powerup);
}

void slow_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    vector_t *scroll_speed = info->scroll_speed;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "SLOW")) {
        remove_old_powerup(entity_get_powerup(entity), scroll_speed);
        entity_set_powerup(entity, "SLOW");
        scroll_speed->x = 0.5 * scroll_speed->x;
    }
    body_remove(powerup);
}

void jump_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "JUMP")) {
        remove_old_powerup(entity_get_powerup(entity), info->scroll_speed);
        entity_set_powerup(entity, "JUMP");
    }
    body_remove(powerup);
}

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
                          vector_t *scroll_speed) {
    powerup_info_t *info = malloc(sizeof(powerup_info_t));
    info->scene = scene;
    info->scroll_speed = scroll_speed;
    int percent_max = 100;
    int percent_magnet = 10;
    int percent_slow = 60;
    int percent_jump = 100;
    int random_powerup = rand()%percent_max;
    body_t *player = scene_get_body(scene, 3);
    body_t *powerup = spawn_powerup(scene, MIN, MAX, info);
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