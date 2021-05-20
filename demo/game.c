#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "body.h"
#include "scene.h"
#include "forces.h"
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
    if (strcmp(body_get_info(scene_get_body(scene, 0)), "PLAYER")) {
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
    list_t *coords = compute_rect_points(center, 2 * PLAYER_RADIUS, 2 * PLAYER_RADIUS);
    body_t *player = body_init_with_info(coords, PLAYER_MASS, PLAYER_COLOR, "PLAYER", body_free);
    scene_add_body(scene, player);
    vector_t fc = (vector_t){MAX.x/2, 10};
    list_t *floor_coords = compute_rect_points(fc, MAX.x, 50);
    body_t *floor = body_init_with_info(floor_coords, INFINITY, BLACK, "FLOOR", body_free);
    scene_add_body(scene, floor);
    //create_newtonian_gravity()
}

void add_bullet (scene_t *scene, vector_t center, rgb_color_t color,
                    vector_t velocity, char *bullet_type, char *target_type) {
    body_t *bullet = body_init_with_info(
        compute_circle_points(center, BULLET_RADIUS, 0),
        BULLET_MASS, color, bullet_type, free);
    body_set_velocity(bullet, velocity);

    scene_add_body(scene, bullet);

    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (!strcmp(body_get_info(body), target_type)) {
            create_destructive_collision(scene, body, bullet);
        }
    }
}


void player_move (char key, key_event_type_t type, double held_time, void *scene) {
    body_t *player = scene_get_body(scene, 0);
    printf("check 3\n");
    vector_t new_velocity = {0, body_get_velocity(player).y};
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                if (body_get_centroid(player).x - PLAYER_RADIUS > MIN.x) {
                    new_velocity.x = -PLAYER_SPEED;
                }
                break;
            case RIGHT_ARROW:
                if (body_get_centroid(player).x + PLAYER_RADIUS < MAX.x) {
                    new_velocity.x = PLAYER_SPEED;
                }
                break;
            case (char)32: //spacebar
                if (held_time < 10) {
                    new_velocity.y = PLAYER_SPEED;
                }
                break;
        }
    }
    body_set_velocity(player, new_velocity);
}

void player_shoot(char key, mouse_event_type_t type, double held_time, void *scene){
    printf("check 3\n");
    body_t *player = scene_get_body(scene, 0);
    printf("check 4\n");
    vector_t new_velocity = {0, 0};
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK:
                vector_t mouse = sdl_mouse_pos();
                printf("%d %d \n", mouse.x, mouse.y);
                vector_t center = body_get_centroid(player);
                printf("center: %f %f \n", center.x, center.y);
                vector_t shoot = vec_unit(vec_subtract(mouse, center));
                printf("shoot: %f %f \n", shoot.x, shoot.y);
                add_bullet(scene, center, PLAYER_BULLET_COLOR,
                        vec_multiply(200, shoot), "BULLET", "ENEMY");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    scene_t *scene = scene_init();

    sdl_init(MIN,MAX);

    while (true) {
        scene = scene_init();
        printf("check 1");
        initialize_player(scene);
        printf("check 2");
        sdl_on_key(player_move);
        sdl_on_click(player_shoot);
        double time_since_last_bullet = 0;
        while (!check_game_end(scene)) {
            double dt = time_since_last_tick();
            scene_tick(scene, dt);
            if (sdl_is_done(scene)) {
                scene_free(scene);
                game_end();
            }
            sdl_render_scene(scene);
        }
        scene_free(scene);
    }
    return 0;
}