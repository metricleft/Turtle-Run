#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "sdl_wrapper.h"
#include "powerup.h"

const int GAME_POWERUP_MASS = 0.2;
const int POWERUP_RADIUS = 15;
const char* MAGNET = "static/frog_spritesheet.png";
const char* SLOW = "static/dragonfly_spritesheet.png";
const char* JUMP = "static/goose_spritesheet.png";

void remove_old_powerup(char *powerup, vector_t *scroll_speed) {
    if (!strcmp(powerup, "SLOW")) {
        *scroll_speed = vec_multiply(2, *scroll_speed);
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

void magnet_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    scene_t *scene = info->scene;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "MAGNET")) {
        remove_old_powerup(scene, entity_get_powerup(entity), info->scroll_speed);
        entity_set_powerup(entity, "MAGNET");
        for (size_t i = 0; i < scene_get_bodies(scene); i++) {
            body_t *body = scene_get_body(scene, i);
            if (!strcmp(entity_get_type(body_get_info(body)), "COIN")) {
                //TODO: make gravity between it and the player
                //uses all the gravity stuff we just made
            }
        }
    }
    body_remove(powerup);
}

void slow_handler(body_t *player, body_t *powerup, vector_t axis, void *aux) {
    powerup_info_t *info = aux;
    double *scroll_speed = info->scroll_speed;
    player_entity_t *entity = body_get_info(player);
    if (strcmp(entity_get_powerup(entity), "SLOW")) {
        remove_old_powerup(entity_get_powerup(entity), scroll_speed);
        entity_set_powerup(entity, "SLOW");
        *scroll_speed = 0.5 * *scroll_speed;
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

void spawn_magnet(scene_t *scene, vector_t MIN, vector_t MAX) {
    //TODO: create the powerup body off the edge of the screen at some random elevation
    //Animation frame thingy
    //Create collision between it and the player using the right handler
    //return the body
}

void spawn_slow(scene_t *scene, vector_t MIN, vector_t MAX) {
    //TODO: create the powerup body off the edge of the screen at some random elevation
    //Animation frame thingy
    //Create collision between it and the player using the right handler
    //return the body
} 

void spawn_jump(scene_t *scene, vector_t MIN, vector_t MAX) {
    //TODO: create the powerup body off the edge of the screen at some random elevation
    //Animation frame thingy
    //Create collision between it and the player using the right handler
    //return the body
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
    body_t *power;
    if (random_powerup <= percent_magnet) {
        spawn_magnet(scene, MIN, MAX, info);
    }
    else if (random_powerup <= percent_slow) {
        spawn_slow(scene, MIN, MAX, info);
    }
    else {
        spawn_jump(scene, MIN, MAX, info);
    }
    free(info);
}