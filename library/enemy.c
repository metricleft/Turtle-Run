#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "sdl_wrapper.h"
#include "bounds.h"
#include "enemy.h"
#include "entity.h"
#include "forces.h"
#include "shapelib.h"

const int GAME_ENEMY_MASS = 10;
const int ENEMY_RADIUS = 20;
const char* FROG = "static/frog_spritesheet.png";
const char* FLY = "static/dragonfly_spritesheet.png";
const char* GOOSE = "static/goose_spritesheet.png";

//Creates the collisions between player bullets and the enemy.
void create_bullet_collisions(scene_t *scene, body_t *enemy) {
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        if (!strcmp(entity_get_type(entity), "BULLET")) {
            create_destructive_collision(scene, body, enemy);
        }
    }
}

//Spawns a goose that flies across the screen, speeding up.
void spawn_goose(scene_t *scene, vector_t MIN, vector_t MAX) {
    double *drag_const = malloc(sizeof(double));
    *drag_const = -(rand()%15+5);

    body_t *player = scene_get_body(scene, 3);
    vector_t center = {MAX.x + ENEMY_RADIUS, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", true, false);
    list_t *goose_coords = compute_rect_points(center, 2*ENEMY_RADIUS, 2*ENEMY_RADIUS);
    body_t *goose = body_init_with_info(goose_coords, GAME_ENEMY_MASS, entity,
                                        entity_free);
    sprite_t *goose_info = sprite_animated(GOOSE, 1, 10, 12);
    body_set_draw(goose, (draw_func_t) sdl_draw_animated, goose_info, sprite_free);
    create_drag(scene, drag_const, goose, free);
    scene_add_body(scene, goose);
    create_destructive_collision(scene, player, goose);
    create_bullet_collisions(scene, goose);
    create_bounds_collisions(scene, goose, ENEMY_RADIUS);
}

//Spawns a frog that bounces up and down the screen.
void spawn_frog(scene_t *scene, vector_t MIN, vector_t MAX) {
    double *spring_const = malloc(sizeof(double));
    *spring_const = rand()%15+5;

    body_t *player = scene_get_body(scene, 3);

    vector_t center = {MAX.x + ENEMY_RADIUS, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", false, false);
    list_t *frog_coords = compute_rect_points(center, 2*ENEMY_RADIUS, 2*ENEMY_RADIUS);
    body_t *frog = body_init_with_info(frog_coords, GAME_ENEMY_MASS, entity, entity_free);
    sprite_t *frog_info = sprite_animated(FROG, 1, 8, 6);
    body_set_draw(frog, (draw_func_t) sdl_draw_animated, frog_info, sprite_free);
    center.y = MAX.y / 2;
    entity = entity_init("ANCHOR", true, false);
    list_t *anchor_coords = compute_circle_points(center, ENEMY_RADIUS, ENEMY_RADIUS);
    body_t *anchor = body_init_with_info(anchor_coords, INFINITY, entity, entity_free);

    create_spring(scene, spring_const, anchor, frog, free);

    scene_add_body(scene, frog);
    scene_add_body(scene, anchor);
    create_destructive_collision(scene, player, frog);
    create_bullet_collisions(scene, frog);
    create_bounds_collisions(scene, frog, ENEMY_RADIUS);
    create_bounds_collisions(scene, anchor, ENEMY_RADIUS);
}

//Temporary param struct for the fly's gravity creator.
typedef struct param {
    double constant;
    body_t *body1;
    body_t *body2;
} param_t;

//One-way gravity creator that attracts the fly to the player.
void one_way_gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1),
                              body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    force = vec_multiply(-aux->constant * mass_product / pow(sqrt(vec_dot(r,r)), 3.0), r);
    body_add_force(aux->body1, force);
}

//Attracts the fly to a player by applying gravity only to body1 (the fly).
void create_one_way_gravity(scene_t *scene, double G, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){G, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, one_way_gravity_creator, force_param, bodies,
                                   free);
}

//Spawns a fly that lazily follows the player and is attracted to the player.
void spawn_fly(scene_t *scene, vector_t MIN, vector_t MAX) {
    double gravity_const = rand()%500000+500000;

    body_t *player = scene_get_body(scene, 3);
    vector_t center = {MAX.x + ENEMY_RADIUS, rand()%((int)(MAX.y - MIN.y))};
    entity_t *entity = entity_init("ENEMY", true, false);
    list_t *fly_coords = compute_rect_points(center, ENEMY_RADIUS, ENEMY_RADIUS);
    body_t *fly = body_init_with_info(fly_coords, GAME_ENEMY_MASS,  entity, entity_free);
    sprite_t *fly_info = sprite_animated(FLY, 1, 2, 20);
    body_set_draw(fly, (draw_func_t) sdl_draw_animated, fly_info, sprite_free);
    create_one_way_gravity(scene, gravity_const, fly, player);
    scene_add_body(scene, fly);
    create_destructive_collision(scene, player, fly);
    create_bullet_collisions(scene, fly);
    create_bounds_collisions(scene, fly, ENEMY_RADIUS/2);
}

void enemy_spawn_random(scene_t *scene, vector_t MIN, vector_t MAX) {
    int percent_max = 100;
    int percent_goose = 10;
    int percent_frog = 60;
    int percent_fly = 100;
    int random_enemy = rand()%percent_max;
    if (random_enemy <= percent_goose) {
        spawn_goose(scene, MIN, MAX);
    }
    else if (random_enemy <= percent_frog) {
        spawn_frog(scene, MIN, MAX);
    }
    else if (random_enemy <= percent_fly) {
        spawn_fly(scene, MIN, MAX);
    }
}
