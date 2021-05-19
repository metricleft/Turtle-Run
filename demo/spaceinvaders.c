#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "body.h"
#include "scene.h"
#include "forces.h"
#include "sdl_wrapper.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};

const int ARC_RESOLUTION = 5;

const int NUM_ENEMIES = 22;
const double ENEMY_RADIUS = 35;
const double ENEMY_MASS = 10;
const double ENEMY_ARC_ANGLE = M_PI / 4;
const vector_t ENEMY_VELOCITY = {100, 0};
const double ENEMY_PADDING = 12;

const double BULLET_HEIGHT = 10;
const double BULLET_WIDTH = 2;
const double BULLET_MASS = 0.2;
const vector_t BULLET_VELOCITY = {0, -150};
const rgb_color_t ENEMY_BULLET_COLOR = {1, 0, 0};
const rgb_color_t PLAYER_BULLET_COLOR = {0, 1, 0};
const double BULLET_INTERVAL = 2;

const double PLAYER_RADIUS = 40;
const double PLAYER_MASS = 10;
const rgb_color_t PLAYER_COLOR = {0, 1, 0};

const double DEFAULT_VELOCITY_COMPONENT = 1000;

bool game_end() {
    sdl_on_key(NULL);
    exit(0);
}

list_t *compute_sector_points(vector_t center, double radius, double angle) {
    list_t *coords = list_init(ARC_RESOLUTION+2, free);

    vector_t *c = malloc(sizeof(vector_t));
    c->x = center.x;
    c->y = center.y;
    list_add(coords, c);
    
    vector_t *top_mouth = polar_to_cartesian(radius, angle / 2);
    top_mouth->x = top_mouth->x + center.x;
    top_mouth->y = top_mouth->y + center.y;
    list_add(coords, top_mouth);

    double d_theta = (M_PI - angle) / ARC_RESOLUTION;
    for (int i = 1; i < ARC_RESOLUTION; i++) {
        vector_t *next_point = polar_to_cartesian(radius, (angle/2) + i*d_theta);
        next_point->x = next_point->x + center.x;
        next_point->y = next_point->y + center.y;
        list_add(coords, next_point);
    }

    vector_t *bottom_mouth = polar_to_cartesian(radius, M_PI - angle / 2);
    bottom_mouth->x = bottom_mouth->x + center.x;
    bottom_mouth->y = bottom_mouth->y + center.y;
    list_add(coords, bottom_mouth);

    return coords;
}

list_t *compute_player_points(vector_t center, double radius) {
    //triangular player
    list_t *coords = list_init(3, free);

    vector_t *top_point = polar_to_cartesian(radius, M_PI / 2);
    top_point->x = top_point->x + center.x;
    top_point->y = top_point->y + center.y;
    list_add(coords, top_point);

    vector_t *right_point = polar_to_cartesian(radius, 0);
    right_point->x = right_point->x + center.x;
    right_point->y = right_point->y + center.y;
    list_add(coords, right_point);

    vector_t *left_point = polar_to_cartesian(radius, M_PI);
    left_point->x = left_point->x + center.x;
    left_point->y = left_point->y + center.y;
    list_add(coords, left_point);

    return coords;
}

list_t *compute_bullet_points (vector_t *center, double height, double width) {
    list_t *coords = list_init(4, free);

    vector_t *top_left = malloc(sizeof(vector_t));
    top_left->x = center->x - width/2;
    top_left->y = center->y + height/2;
    list_add(coords, top_left);

    vector_t *bottom_left = malloc(sizeof(vector_t));
    bottom_left->x = center->x - width/2;
    bottom_left->y = center->y - height/2;
    list_add(coords, bottom_left);

    vector_t *bottom_right = malloc(sizeof(vector_t));
    bottom_right->x = center->x + width/2;
    bottom_right->y = center->y - height/2;
    list_add(coords, bottom_right);

    vector_t *top_right = malloc(sizeof(vector_t));
    top_right->x = center->x + width/2;
    top_right->y = center->y + height/2;
    list_add(coords, top_right);

    return coords;
}

int initialize_enemies(scene_t *scene) {
    int rows = 1;

    body_t *player = scene_get_body(scene, 0);

    vector_t enemy_pos = {.x = ENEMY_RADIUS + ENEMY_PADDING,
                          .y = MAX.y - ENEMY_RADIUS - ENEMY_PADDING};

    for (int i = 0; i < NUM_ENEMIES; i++) {
        body_t *enemy = body_init_with_info(
            compute_sector_points(enemy_pos, ENEMY_RADIUS, ENEMY_ARC_ANGLE),
            ENEMY_MASS, random_color(), "ENEMY", free);
        body_set_velocity(enemy, ENEMY_VELOCITY);
        scene_add_body(scene, enemy);
        create_destructive_collision(scene, player, enemy);

        //start a new row and reset to leftmost position
        if (enemy_pos.x + 5*ENEMY_RADIUS> MAX.x) {
            if (i != NUM_ENEMIES - 1) {
                rows++;
            }
            enemy_pos.y = enemy_pos.y - ENEMY_RADIUS - ENEMY_PADDING;
            enemy_pos.x = ENEMY_RADIUS + ENEMY_PADDING;
        }
        else {
            enemy_pos.x = enemy_pos.x + 2*ENEMY_RADIUS + ENEMY_PADDING;
        }
    }
    return rows;
}

void update_enemy_positions (scene_t *scene, int rows) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);

        //Updates position if body is an enemy
        if (!strcmp(body_get_info(body), "ENEMY")) {
            list_t *body_points = body_get_shape(body);

            //Translates body down the columns upon wall collision
            vector_t translate = {0, 0};
            vector_t new_velocity = body_get_velocity(body);
            if (new_velocity.x > 0 &&
                body_get_centroid(body).x + ENEMY_RADIUS > MAX.x) {
                translate.y = -rows * (ENEMY_RADIUS + ENEMY_PADDING);
                new_velocity.x = -new_velocity.x;
            }
            else if (new_velocity.x < 0 &&
                     body_get_centroid(body).x - ENEMY_RADIUS < MIN.x) {
                translate.y = -rows * (ENEMY_RADIUS + ENEMY_PADDING);
                new_velocity.x = -new_velocity.x;
            }
            body_set_centroid(body, vec_add(body_get_centroid(body), translate));
            body_set_velocity(body, new_velocity);
            free(body_points);
        }
    }
}

void update_bullets (scene_t *scene) {
    //remove them if they go off the top or bottom of screen
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (!strcmp(body_get_info(body), "BULLET")) {
            vector_t centroid = body_get_centroid(body);
            if (centroid.y + BULLET_HEIGHT/2 > MAX.y ||
                    centroid.y - BULLET_HEIGHT/2 < MIN.y) {
                body_remove(body);
            }
        }
    }
}

void add_bullet (scene_t *scene, vector_t *center, rgb_color_t color,
                    vector_t velocity, char *bullet_type, char *target_type) {
    body_t *bullet = body_init_with_info(
        compute_bullet_points(center, BULLET_HEIGHT, BULLET_WIDTH),
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

body_t *random_enemy (scene_t *scene) {
    list_t *enemies = list_init(DEFAULT_LIST_SIZE, body_free);
    for (int i = 1; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        if (!strcmp(body_get_info(body), "ENEMY") && !body_is_removed(body)) {
            list_add(enemies, body);
        }
    }
    if (list_size(enemies) == 0) {
        game_end();
    }
    int random = (int)((rand()%list_size(enemies)));
    body_t *return_enemy = list_get(enemies, random);
    free(enemies);
    return return_enemy;
}

void ensure_bounds (body_t *player) {
    vector_t centroid = body_get_centroid(player);
    if (centroid.x - PLAYER_RADIUS < MIN.x) {
        centroid.x = PLAYER_RADIUS;
    }
    else if (centroid.x + PLAYER_RADIUS > MAX.x) {
        centroid.x = MAX.x - PLAYER_RADIUS;
    }
    body_set_centroid(player, centroid);
}

void check_game_end(scene_t *scene) {
    //Check if player is gone: lose condition. This condition also triggers if
    //any enemy collides with the player. This condition triggers first if the
    //player and the enemy die at the same time.
    if (strcmp(body_get_info(scene_get_body(scene, 0)), "PLAYER")) {
        game_end();
    }

    int num_enemies = 0;
    for (int i = 1; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);

        //Check if all enemies are gone: win condition
        if (!strcmp(body_get_info(body), "ENEMY") && !body_is_removed(body)) {
            num_enemies++;
        }
    }
    if (num_enemies == 0) {
        game_end();
    }
}

void player_move (char key, key_event_type_t type, double held_time, void *scene) {
    body_t *player = scene_get_body(scene, 0);
    vector_t new_velocity = {0, 0};
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                new_velocity.x = -DEFAULT_VELOCITY_COMPONENT;
                break;
            case RIGHT_ARROW:
                new_velocity.x = DEFAULT_VELOCITY_COMPONENT;
                break;
            case (char)32: //spacebar
                list_t *player_shape = body_get_shape(player);
                vector_t *center = list_get(player_shape, 0);
                center->y = center->y + BULLET_HEIGHT/2;
                add_bullet(scene, center, PLAYER_BULLET_COLOR,
                        vec_negate(BULLET_VELOCITY), "BULLET", "ENEMY");
                free(player_shape);
                break;
        }
    }
    body_set_velocity(player, new_velocity);
}

int main(int argc, char *argv[]) {
    vector_t start_pos = {.x = (MAX.x - MIN.y)/2,
                          .y = 10};

    scene_t *scene = scene_init();
    sdl_init(MIN,MAX);

    body_t *player = body_init_with_info(
        compute_player_points(start_pos, PLAYER_RADIUS), PLAYER_MASS, PLAYER_COLOR,
                              "PLAYER", free);
    scene_add_body(scene, player);

    int rows = initialize_enemies(scene);

    sdl_on_key(player_move);
    double time_since_last_bullet = 0;
    while (!sdl_is_done(scene)) {
        double dt = time_since_last_tick();
        time_since_last_bullet += dt;
        if (time_since_last_bullet > BULLET_INTERVAL) {
            body_t *enemy = random_enemy(scene);
            //If no more enemies, no more bullets will fire
            if (enemy != NULL) {
                list_t *enemy_shape = body_get_shape(enemy);
                vector_t *center = list_get(enemy_shape, 0);
                center->y = center->y - BULLET_HEIGHT/2;
                add_bullet(scene, center, ENEMY_BULLET_COLOR,
                    BULLET_VELOCITY, "BULLET", "PLAYER");
                free(enemy_shape);
            }
            time_since_last_bullet = 0;
        }

        scene_tick(scene, dt);
        update_enemy_positions(scene, rows);
        update_bullets(scene);
        check_game_end(scene);
        ensure_bounds(player);

        sdl_render_scene(scene);
    }
    scene_free(scene);
    return 0;
}