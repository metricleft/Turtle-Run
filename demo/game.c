#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "body.h"
#include "polygon.h"
#include "scene.h"
#include "forces.h"
#include "entity.h"
#include "sdl_wrapper.h"
#include "SDL2/SDL_mouse.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};

const int ARC_RESOLUTION = 10;

const double BULLET_RADIUS = 6;
const double BULLET_MASS = 0.2;

const rgb_color_t ENEMY_BULLET_COLOR = {1, 0, 0};

const rgb_color_t PLAYER_BULLET_COLOR = {0, 1, 0};
const double PLAYER_SPEED = 600;
const double PLAYER_RADIUS = 16;
const double PLAYER_MASS = 10;
const rgb_color_t PLAYER_COLOR = {0, 1, 0};

const vector_t DEFAULT_GRAVITY = {0, -200};
const vector_t DEFAULT_SCROLL_SPEED = {-0.1, 0};

const double ELASTIC_COLLISION = 1;
const double INELASTIC_COLLISION = 0;

bool game_end() {
    sdl_on_key(NULL);
    sdl_on_click(NULL);
    exit(0);
}

list_t *compute_circle_points(vector_t center, double radius) {
     list_t *coords = list_init(ARC_RESOLUTION, free);

    double d_theta = (2*M_PI / ARC_RESOLUTION);
    for (int i = 0; i < ARC_RESOLUTION; i++) {
        vector_t *next_point = polar_to_cartesian(radius, i*d_theta);
        next_point->x = next_point->x + center.x;
        next_point->y = next_point->y + center.y;
        list_add(coords, next_point);
    }
    return coords;
}

bool check_game_end(scene_t *scene) {
    //Check if player is gone: lose condition.
    entity_t *entity = body_get_info(scene_get_body(scene, 0));
    if (strcmp(entity_get_type(entity), "PLAYER")) {
        game_end();
        return true;
    }
    return false;
}

list_t *compute_rect_points(vector_t center, double width, double height) {
    vector_t half_width  = {.x = width / 2, .y = 0.0},
             half_height = {.x = 0.0, .y = height / 2};
    list_t *rect = list_init(4, free);
    vector_t *v = malloc(sizeof(*v));
    *v = vec_add(half_width, half_height);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_height, half_width);
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_negate(*(vector_t *) list_get(rect, 0));
    list_add(rect, v);
    v = malloc(sizeof(*v));
    *v = vec_subtract(half_width, half_height);
    list_add(rect, v);

    polygon_translate(rect, center);
    return rect;
}

void initialize_player(scene_t *scene) {
    vector_t center = {MAX.x / 2, MAX.y / 2};
    entity_t *entity = entity_init("PLAYER", false, true);
    list_t *coords = compute_rect_points(center, 2 * PLAYER_RADIUS, 2 * PLAYER_RADIUS);
    body_t *player = body_init_with_info(coords, PLAYER_MASS, PLAYER_COLOR, entity, entity_free);
    body_add_force(player, vec_multiply(body_get_mass(player), DEFAULT_GRAVITY));
    scene_add_body(scene, player);
}

//TODO: this function will be changed into derek's terrain implementation eventually
void initialize_terrain(scene_t *scene) {
    body_t *player = scene_get_body(scene, 0);
    vector_t fc = (vector_t){MAX.x/2, 10};
    entity_t *entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(fc, MAX.x, 50);
    body_t *floor = body_init_with_info(floor_coords, INFINITY, BLACK, entity, entity_free);
    scene_add_body(scene, floor);
    create_physics_collision(scene, INELASTIC_COLLISION, player, floor);
}

void add_bullet (scene_t *scene, vector_t center, rgb_color_t color,
                    vector_t velocity, entity_t *bullet_entity, char *target_type) {
    body_t *bullet = body_init_with_info(
        compute_circle_points(center, BULLET_RADIUS),
        BULLET_MASS, color, bullet_entity, entity_free);
    body_set_velocity(bullet, velocity);

    scene_add_body(scene, bullet);

    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        if (!strcmp(entity_get_type(entity), target_type)) {
            create_destructive_collision(scene, body, bullet);
        }
    }
}

void sidescroll(scene_t *scene, vector_t *scroll_speed) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        //Applies a leftwards velocity to all objects with the "SCROLLABLE" tag
        if (entity_get_scrollable(entity)) {
            body_set_velocity(body, vec_add(body_get_velocity(body), *scroll_speed));
        }
        //Applies a downwards force (gravity) to all objects with the "FALLABLE" tag
        if (entity_get_fallable(entity)) {
            body_add_force(body, vec_multiply(body_get_mass(body), DEFAULT_GRAVITY));
        }
    }
}

void player_move (char key, key_event_type_t type, double held_time, void *scene) {
    body_t *player = scene_get_body(scene, 0);
    vector_t new_velocity = {0, body_get_velocity(player).y};
    if (type == KEY_PRESSED) {
        switch (key) {
            case 'a':
            case LEFT_ARROW:
                if (body_get_centroid(player).x - PLAYER_RADIUS > MIN.x) {
                    new_velocity.x = -PLAYER_SPEED;
                }
                break;
            case 'd':
            case RIGHT_ARROW:
                if (body_get_centroid(player).x + PLAYER_RADIUS < MAX.x) {
                    new_velocity.x = PLAYER_SPEED;
                }
                break;
            case (char)32: //spacebar
                if (held_time < 0.2) {
                    new_velocity.y = PLAYER_SPEED/3;
                }
                break;
        }
    }
    body_set_velocity(player, new_velocity);
}

void player_shoot(char key, mouse_event_type_t type, double held_time, void *scene){
    body_t *player = scene_get_body(scene, 0);
    vector_t new_velocity = {0, 0};
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK:
                vector_t mouse = sdl_mouse_pos();
                vector_t center = body_get_centroid(player);
                vector_t shoot = vec_unit(vec_subtract(mouse, center));
                entity_t *entity = entity_init("BULLET", false, false);
                add_bullet(scene, center, PLAYER_BULLET_COLOR,
                        vec_multiply(200, shoot), entity, "ENEMY");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    scene_t *scene = scene_init();

    vector_t *scroll_speed = malloc(sizeof(vector_t));
    *scroll_speed = DEFAULT_SCROLL_SPEED;

    sdl_init(MIN,MAX);
    sdl_on_key(player_move);
    sdl_on_click(player_shoot);

    while (true) {
        scene = scene_init();
        initialize_player(scene);
        initialize_terrain(scene);
        //double time_since_last_enemy = 0;
        while (!check_game_end(scene)) {
            double dt = time_since_last_tick();
            sidescroll(scene, scroll_speed);
            scene_tick(scene, dt);
            if (sdl_is_done(scene)) {
                scene_free(scene);
                game_end();
            }
            sdl_render_scene(scene);
        }
        scene_free(scene);
    }

    free(scroll_speed);
    return 0;
}