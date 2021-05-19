#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "scene.h"
#include "forces.h"
#include "sdl_wrapper.h"
#include "polygon.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};

const double NUM_BRICK_COLUMNS = 10;
const double NUM_BRICK_ROWS = 3;
const double BRICK_HEIGHT = 20;
const double BRICK_PADDING = 6;

const double BALL_RADIUS = 6;
const int ARC_RESOLUTION = 25;
const double BALL_MASS = 1;

const double PLAYER_WIDTH = 70;
const double PLAYER_HEIGHT = 10;
const double PLAYER_MASS = INFINITY;

const double BALL_SPEED = 200;
const double PLAYER_SPEED = 600;

const double ELASTIC_COLLISION = 1.0;
const double INELASTIC_COLLISION = 0.0;

const int DEFAULT_HEALTH = 3;

bool game_end() {
    sdl_on_key(NULL);
    exit(0);
}

list_t *compute_ball_points(vector_t center, double radius) {
    list_t *coords = list_init(ARC_RESOLUTION, free);

    double d_theta = (2*M_PI / ARC_RESOLUTION);
    for (int i = 0; i < ARC_RESOLUTION; i++) {
        vector_t *next_point = polar_to_cartesian(BALL_RADIUS, i*d_theta);
        next_point->x = next_point->x + center.x;
        next_point->y = next_point->y + center.y;
        list_add(coords, next_point);
    }
    return coords;
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

void initialize_ball(scene_t *scene) {
    body_t *ball = body_init_with_info(
                    compute_ball_points((vector_t){MAX.x / 2, MAX.y / 2}, BALL_RADIUS),
                    BALL_MASS, BLACK, "BALL", body_free);
    body_set_velocity(ball, (vector_t){BALL_SPEED, BALL_SPEED});
    scene_add_body(scene, ball);
}

void initialize_player(scene_t *scene) {
    body_t *ball = scene_get_body(scene, 0);

    vector_t center = {MAX.x / 2, 10};
    list_t *coords = compute_rect_points(center, PLAYER_WIDTH, PLAYER_HEIGHT);
    body_t *player = body_init_with_info(coords, PLAYER_MASS, BLACK, "PLAYER", body_free);
    scene_add_body(scene, player);

    create_physics_collision(scene, ELASTIC_COLLISION, player, ball);
}

void initialize_walls(scene_t *scene) {
    body_t *ball = scene_get_body(scene, 0);
    body_t *player = scene_get_body(scene, 1);
    double wall_height = MAX.y - MIN.y;
    double wall_width = MAX.x - MIN.x;

    //top wall
    vector_t wall_center = {wall_width/2, 1.5*wall_height};
    list_t *wall_shape = compute_rect_points(wall_center, wall_width, wall_height);
    body_t *wall = body_init_with_info(wall_shape, INFINITY, GREEN,
                                           "WALL", body_free);
    scene_add_body(scene, wall);
    create_physics_collision(scene, ELASTIC_COLLISION, wall, ball);

    //left wall
    wall_center = (vector_t){-0.5*wall_width, wall_height/2};
    wall_shape = compute_rect_points(wall_center, wall_width, wall_height);
    wall = body_init_with_info(wall_shape, INFINITY, GREEN,
                                           "WALL", body_free);
    scene_add_body(scene, wall);
    create_physics_collision(scene, ELASTIC_COLLISION, wall, ball);

    //right wall
    wall_center = (vector_t){1.5*wall_width, wall_height/2};
    wall_shape = compute_rect_points(wall_center, wall_width, wall_height);
    wall = body_init_with_info(wall_shape, INFINITY, GREEN,
                                           "WALL", body_free);
    scene_add_body(scene, wall);
    create_physics_collision(scene, ELASTIC_COLLISION, wall, ball);

    //bottom wall
    wall_center = (vector_t){0.5*wall_width, -0.5*wall_height};
    wall_shape = compute_rect_points(wall_center, wall_width, wall_height);
    wall = body_init_with_info(wall_shape, INFINITY, GREEN,
                                           "WALL", body_free);
    scene_add_body(scene, wall);
    create_oneway_destructive_collision(scene, ELASTIC_COLLISION, wall, ball);
}

void health(body_t *ball, body_t *brick, vector_t axis, void *aux) {
    int *brick_health = aux;

    double reduced_mass = calculate_reduced_mass(ball, brick);
    vector_t impulse = 
                vec_multiply(
                    reduced_mass * 
                    (1.0 + ELASTIC_COLLISION) *
                    (vec_dot(body_get_velocity(brick),axis)-
                    vec_dot(body_get_velocity(ball),axis)),
                    axis);
    if (body_get_mass(ball) != INFINITY) {
        body_add_impulse(ball, impulse);
    }
    if (*brick_health == 0) {
        body_remove(brick);
    }
    else {
        *brick_health = *brick_health - 1;
        body_set_color(brick, decrease_intensity(body_get_color(brick)));
    }
}

void initialize_bricks(scene_t *scene) {
    body_t *ball = scene_get_body(scene, 0);

    double brick_width = (MAX.x - MIN.x - (NUM_BRICK_COLUMNS+1)*BRICK_PADDING) /
                          NUM_BRICK_COLUMNS;

    vector_t center = {brick_width/2 + BRICK_PADDING,
                       MAX.y - BRICK_PADDING - BRICK_HEIGHT/2};
    rgb_color_t color = RED;

    for (int i = 0; i < NUM_BRICK_ROWS; i++) {
        for (int j = 0; j < NUM_BRICK_COLUMNS; j++) {
            int *brick_health = malloc(sizeof(int));
            *brick_health = DEFAULT_HEALTH;
            body_t *brick = body_init_with_info(
                compute_rect_points(center, brick_width, BRICK_HEIGHT),
                INFINITY, color, "BRICK", body_free);
            scene_add_body(scene, brick);
            create_collision(scene, ball, brick, health, brick_health, free);

            color = next_rainbow_color(color);
            center.x = center.x + brick_width + BRICK_PADDING;
        }
        center.y = center.y - BRICK_HEIGHT - BRICK_PADDING;
        center.x = brick_width/2 + BRICK_PADDING;
    }
}

bool check_game_end(scene_t *scene) {
    int num_bricks = 0;
    for (int i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);

        //Check if all bricks are gone: win condition
        if (!strcmp(body_get_info(body), "BRICK") && !body_is_removed(body)) {
            num_bricks++;
        }
    }
    if (num_bricks == 0) {
        return true;
    }

    //Check if the ball is gone: lose condition
    if (strcmp(body_get_info(scene_get_body(scene, 0)), "BALL")) {
        return true;
    }

    return false;
}

void player_move (char key, key_event_type_t type, double held_time, void *scene) {
    body_t *player = scene_get_body(scene, 1);
    vector_t new_velocity = {0, 0};
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW:
                new_velocity.x = -PLAYER_SPEED;
                break;
            case RIGHT_ARROW:
                new_velocity.x = PLAYER_SPEED;
                break;
        }
    }
    body_set_velocity(player, new_velocity);
}

int main(int argc, char *argv[]) {
    scene_t *scene = scene_init();

    sdl_init(MIN,MAX);

    while (true) {
        scene = scene_init();

        initialize_ball(scene);
        initialize_player(scene);
        initialize_walls(scene);
        initialize_bricks(scene);

        sdl_on_key(player_move);
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
