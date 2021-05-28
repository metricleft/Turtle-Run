#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "body.h"
#include "polygon.h"
#include "scene.h"
#include "forces.h"
#include "entity.h"
#include "shapelib.h"
#include "sdl_wrapper.h"
#include "SDL2/SDL_mouse.h"
#include "enemy.h"
#include "frame.h"
#include "powerup.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};
Mix_Chunk *effects = NULL;

const int ARC_RESOLUTION = 10;

const double BULLET_RADIUS = 6;
const double BULLET_MASS = 0.2;
const char *BULLET_SPRITE = "static/bullet.png";

const int POWERUP_INTERVAL = 15;
const int ENEMY_INTERVAL = 5;
const int SPEEDUP_INTERVAL = 5;

const double DEFAULT_SPEEDUP = -10;

//const rgb_color_t PLAYER_BULLET_COLOR = {0, 1, 0};
const double PLAYER_SPEED = 600;
const double PLAYER_RADIUS = 30;
const double PLAYER_MASS = 10;
//const rgb_color_t PLAYER_COLOR = {0, 1, 0};
const char *PLAYER_SPRITE = "static/turtle_spritesheet2.png";
const double PLAYER_SCALE = 2;
const int PLAYER_FRAMES = 8;
const int PLAYER_FPS = 6;

const char* SKY_IMG = "static/background_sky.png";
const char* GRASS_IMG = "static/background_grass.png";
const char* WATER_IMG = "static/background_water.png";
const double BACKGROUND_SCALE = 2.0;

const vector_t DEFAULT_GRAVITY = {0, -500};
const vector_t DEFAULT_SCROLL_SPEED = {-200, 0};

const double ELASTIC_COLLISION = 1;
const double INELASTIC_COLLISION = 0;

bool game_end() {
    sdl_on_key(NULL);
    sdl_on_click(NULL);
    exit(0);
}

double basic_score_calculation(double dt) {
    assert(dt >= 0);
    return dt * 100.0;
}

bool check_game_end(scene_t *scene) {
    //Check if player is gone: lose condition.
    entity_t *entity = body_get_info(scene_get_body(scene, 3));
    if (strcmp(entity_get_type(entity), "PLAYER")) {
        //game_end();
        return true;
    }
    return false;
}

void add_background(scene_t *scene, const char* img, int speed){
    vector_t center = {MAX.x / 2, MAX.y / 2};
    list_t *window = compute_rect_points(center, MAX.x, MAX.y);
    entity_t *info = entity_init("BACKGROUND", false, false);
    body_t *background = body_init_with_info(window, INFINITY, info , entity_free);
    sprite_t *back_info = sprite_scroll(img, BACKGROUND_SCALE, 3, speed);
    body_set_draw(background, sdl_draw_scroll, back_info, sprite_free); 
    scene_add_body(scene, background);
}

void initialize_background(scene_t *scene){
    add_background(scene, SKY_IMG, abs((int)DEFAULT_SCROLL_SPEED.x /16));
    add_background(scene, GRASS_IMG, abs((int)DEFAULT_SCROLL_SPEED.x /8));
    add_background(scene, WATER_IMG, abs((int)DEFAULT_SCROLL_SPEED.x /6));
}

void initialize_player(scene_t *scene) {
    vector_t center = {MAX.x / 2, MAX.y - PLAYER_RADIUS};
    player_entity_t *entity = player_entity_init("PLAYER", false, true);
    list_t *coords = compute_rect_points(center, 2 * PLAYER_RADIUS, 2 * PLAYER_RADIUS);
    body_t *player = body_init_with_info(coords, PLAYER_MASS, entity, entity_free);
    sprite_t *sprite_player = sprite_animated(PLAYER_SPRITE, 
                                              PLAYER_SCALE, 
                                              PLAYER_FRAMES, 
                                              PLAYER_FPS);
    body_set_draw(player, (draw_func_t) sdl_draw_animated, sprite_player, sprite_free);
    scene_add_body(scene, player);
    vector_t *grav = malloc(sizeof(vector_t));
    *grav = DEFAULT_GRAVITY;
    create_constant_force(scene, grav, player, free);
}

void initialize_terrain(scene_t *scene) {
    body_t *player = scene_get_body(scene, 3);
    vector_t center = (vector_t){MAX.x/2, 10};
    entity_t *entity = entity_init("TERRAIN", true, false);
    list_t *floor_coords = compute_rect_points(center, MAX.x, 50);
    body_t *floor = body_init_with_info(floor_coords, INFINITY, entity, entity_free);
    scene_add_body(scene, floor);
    create_normal_collision(scene, vec_negate(DEFAULT_GRAVITY), player, floor);

    rgb_color_t *black = malloc(sizeof(rgb_color_t));
    *black = BLACK;
    body_set_draw(floor, (draw_func_t) sdl_draw_polygon, black, free);
}

void add_bullet (scene_t *scene, vector_t center, vector_t velocity,
                 entity_t *bullet_entity, char *target_type) {
    body_t *bullet = body_init_with_info(
        compute_circle_points(center, BULLET_RADIUS, ARC_RESOLUTION),
        BULLET_MASS, bullet_entity, entity_free);
    body_set_velocity(bullet, velocity);
    sprite_t *bullet_info = sprite_image(BULLET_SPRITE, 1);
    body_set_draw(bullet, (draw_func_t) sdl_draw_image, bullet_info, sprite_free);
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
        if (entity_get_scrollable(entity) && !entity_is_scrolling(entity)) {
            vector_t scroll = {scroll_speed->x, body_get_velocity(body).y};
            body_set_velocity(body, scroll);
            //Ensures that every object only gets an initial velocity assigned once
            entity_set_scrolling(entity);
        }
    }
}

void player_move (char key, key_event_type_t type, double held_time, void *scene) {
    body_t *player = scene_get_body(scene, 3);
    player_entity_t *entity = body_get_info(player);
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
            case 'w':
            case UP_ARROW:
                if (held_time < 0.2 && (entity_get_colliding(entity) ||
                                        !strcmp(entity_get_powerup(entity), "JUMP"))) {
                    new_velocity.y = 0.8 * PLAYER_SPEED;
                    Mix_PlayChannel(-1, effects, 0);

                    
                }
                break;
        }
    }
    body_set_velocity(player, new_velocity);
}

void player_shoot(char key, mouse_event_type_t type, double held_time, void *scene){
    body_t *player = scene_get_body(scene, 3);
    vector_t new_velocity = {0, 0};
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK:
                vector_t mouse = sdl_mouse_pos();
                vector_t center = body_get_centroid(player);
                vector_t shoot = vec_unit(vec_subtract(mouse, center));
                entity_t *entity = entity_init("BULLET", false, false);
                add_bullet(scene, center, vec_multiply(200, shoot), entity, "ENEMY");
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    time_t t;
    srand((unsigned) time(&t));
    sdl_init(MIN,MAX);
    Mix_Music *soundtrack = loadMedia("sounds/imperial_march.mp3");
    effects = loadEffects("sounds/jump1.wav");
    Mix_PlayMusic(soundtrack, -1);
    //Mix_PlayChannel(-1, effect, 0);  

    while (true) {
        sdl_on_key((event_handler_t) player_move);
        sdl_on_click((event_handler_t) player_shoot);

        scene_t *scene = scene_init();
        vector_t *scroll_speed = malloc(sizeof(vector_t));
        *scroll_speed = DEFAULT_SCROLL_SPEED;
        double *score = malloc(sizeof(double));
        *score = 0;
        initialize_background(scene);
        initialize_player(scene);
        initialize_terrain(scene);
        frame_spawn_random(scene, MAX, MAX.x);

        body_t *player = scene_get_body(scene, 3);
        player_entity_t *player_entity = body_get_info(player);

        double time_since_last_enemy = 0;
        double time_since_last_frame = 0;
        double time_since_last_powerup = 0;
        double time_since_last_speedup = 0;
        while (!check_game_end(scene)) {
            // printf("scroll speed: %f\n", scroll_speed->x);
            entity_set_colliding(player_entity, false);
            

            double dt = time_since_last_tick();
            time_since_last_enemy += dt;
            time_since_last_frame += dt;
            time_since_last_powerup += dt;
            time_since_last_speedup += dt;
            if (time_since_last_enemy > ENEMY_INTERVAL) {
                enemy_spawn_random(scene, MIN, MAX);
                time_since_last_enemy = 0;
            }
            if (time_since_last_frame > MAX.x / -(scroll_speed->x)) {
                frame_spawn_random(scene, MAX, MAX.x);
                time_since_last_frame = 0;
            }
            if (time_since_last_powerup > POWERUP_INTERVAL) {
                powerup_spawn_random(scene, MIN, MAX, scroll_speed);
                time_since_last_powerup = 0;
            }
            if (time_since_last_speedup > SPEEDUP_INTERVAL) {
                scroll_speed->x = scroll_speed->x + DEFAULT_SPEEDUP *
                    (strcmp(entity_get_powerup(player_entity), "SLOW")? 1:0.5);
                time_since_last_speedup = 0;
            }
            *score += basic_score_calculation(dt);

            sidescroll(scene, scroll_speed);
            scene_tick(scene, dt);
            sdl_render_scene(scene);
            if (body_get_centroid(scene_get_body(scene,0)).y < MIN.y - PLAYER_RADIUS) {
                body_remove(scene_get_body(scene, 3));
            } else if (sdl_is_done(scene)) {
                game_end();
            }

        }
        
        free(scene);
        free(scroll_speed);
        free(score);
    }
    Mix_HaltMusic();

    return 0;
}