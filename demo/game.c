#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>

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
#include "bounds.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};

Mix_Chunk *jump = NULL;
Mix_Chunk *slide = NULL;
Mix_Chunk *shot = NULL;


const int ARC_RESOLUTION = 10;

const double MAX_DT = 0.01;
const double MIN_DT = 1e-6;

const double BULLET_RADIUS = 6;
const double BULLET_MASS = 0.2;
const char *BULLET_SPRITE = "static/bullet.png";

const int POWERUP_INTERVAL = 15;
const int ENEMY_INTERVAL = 10;
const int SPEEDUP_INTERVAL = 5;

const double DEFAULT_SPEEDUP = -50;
const double MAX_SPEED = 700;

//const rgb_color_t PLAYER_BULLET_COLOR = {0, 1, 0};
const double PLAYER_SPEED = 600;
const double PLAYER_RADIUS = 30;
const double PLAYER_MASS = 10;
//const rgb_color_t PLAYER_COLOR = {0, 1, 0};
const char *PLAYER_SPRITE = "static/turtle_spritesheet2.png";
const double PLAYER_SCALE = 2;
const int PLAYER_FRAMES = 8;
const int PLAYER_FPS = 6;

const char *DEFAULT_FONT = "static/Sans.ttf";
const int TEXT_HEIGHT = 50;
const int SMALL_TEXT_HEIGHT = 30;
const int THICK_OUTLINE = 4;
const int THIN_OUTLINE = 2;
const int TEXT_SPACING = 80;
const int SMALL_TEXT_SPACING = 50;
const int TEXT_OFFSET = 10;

const char *SKY_IMG = "static/background_sky.png";
const char *GRASS_IMG = "static/background_grass.png";
const char *WATER_IMG = "static/background_water.png";
const char *BACKGROUND_IMG = "static/background.png";
const SDL_Rect BACKGROUND_FRAME = {0,0, 256, 128};

const vector_t DEFAULT_GRAVITY = {0, -800};
const vector_t DEFAULT_SCROLL_SPEED = {-200, 0};

const double ELASTIC_COLLISION = 1;
const double INELASTIC_COLLISION = 0;

double basic_score_calculation(double dt) {
    assert(dt >= 0);
    return dt * 10.0;
}

list_t *get_high_scores() {
    list_t *high_scores = list_init(5, list_free);
    FILE *fp = fopen("scores/highscore.txt", "r");
    if (fp == NULL) {
        printf("Unable to open scores/highscore.txt");
    }
    double **highscore = malloc(sizeof(double) * 5);
    for (int i = 0; i < 5; i++) {
        fscanf(fp, "%lf", &highscore[i]);
    }
    for (int j = 0; j < 5; j++) {
        list_add(high_scores, highscore[j]);
    }
    fclose(fp);
    free(highscore);
    return high_scores;
}

double advanced_score_calculation(double dt) {
    assert(dt >= 0);
    if (dt <= 10) {
        return 10;
    }
    if (dt > 10 && dt <= 20) {
        return 20;
    }
    if (dt > 20 && dt <= 30) {
        return 30;
    }
    if (dt > 30) {
        return 50;
    } 
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
    SDL_Rect *frame = malloc(sizeof(SDL_Rect));
    *frame = BACKGROUND_FRAME;
    sprite_t *back_info = sprite_scroll(img, speed, frame);
    body_set_draw(background, sdl_draw_scroll, back_info, sprite_free); 
    scene_add_body(scene, background);
}

void initialize_background(scene_t *scene){
    add_background(scene, SKY_IMG, abs((int)DEFAULT_SCROLL_SPEED.x /18));
    add_background(scene, GRASS_IMG, abs((int)DEFAULT_SCROLL_SPEED.x/ 9));
    add_background(scene, WATER_IMG, abs((int)DEFAULT_SCROLL_SPEED.x / 6));
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
    create_terrain_collisions(scene, floor);

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
    sprite_t *bullet_info = sprite_image(BULLET_SPRITE, 1, NULL);
    body_set_draw(bullet, (draw_func_t) sdl_draw_image, bullet_info, sprite_free);
    scene_add_body(scene, bullet);
    create_bounds_collisions(scene, bullet, BULLET_RADIUS);

    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        if (!strcmp(entity_get_type(entity), target_type)) {
            create_destructive_collision(scene, body, bullet);
        }
    }
}

void sidescroll(scene_t *scene, vector_t *scroll_speed, double dt) {
    for (size_t i = 0; i < scene_bodies(scene); i++) {
        body_t *body = scene_get_body(scene, i);
        entity_t *entity = body_get_info(body);
        if(i < 3){
            sprite_t *sprite = body_get_draw_info(body);
            sprite_set_speed(sprite, (int)abs((int)(scroll_speed->x) *((int)i+1) /18));
        }
        //Applies a leftwards velocity to all objects with the "SCROLLABLE" tag
        if (entity_get_scrollable(entity) && !entity_is_scrolling(entity)) {
            vector_t scroll = {scroll_speed->x, body_get_velocity(body).y};
            body_set_velocity(body, scroll);
            //Ensures that enemies (which accelerate) only get a velocity assigned once
            if (!strcmp(entity_get_type(entity), "ENEMY")) entity_set_scrolling(entity);
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
                new_velocity.x = -PLAYER_SPEED;
                if (held_time < 0.2) {
                    Mix_PlayChannel(-1, slide, 0);
                }
                break;
            case 'd':
            case RIGHT_ARROW:
                new_velocity.x = PLAYER_SPEED;
                if (held_time < 0.2) {
                    Mix_PlayChannel(-1, slide, 0);
                }
                break;
            case 'w':
            case UP_ARROW:
                if (held_time < 0.2 && (entity_get_colliding(entity) ||
                                        !strcmp(entity_get_powerup(entity), "JUMP"))) {
                    new_velocity.y = 0.8 * PLAYER_SPEED;
                    Mix_PlayChannel(-1, jump, 0);
                    entity_set_colliding(entity, false);
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
                Mix_PlayChannel(-1, shot, 0);
                add_bullet(scene, center, vec_multiply(200, shoot), entity, "ENEMY");
                break;
        }
    }
}

void draw_background() {
    scene_t *menu = scene_init();
    add_background(menu, BACKGROUND_IMG, 0);
    sdl_render_scene(menu);
    free(menu);
}

void click_to_continue(char key, mouse_event_type_t type, double held_time,
                        void *window){
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK:
                SDL_DestroyWindow(window);
                SDL_Event *event = malloc(sizeof(event));
                event->type = SDL_QUIT;
                SDL_PushEvent(event);
                break;
        }
    }
}

void display_main_menu(SDL_Window *window) {
    draw_background();

    //Draw title:
    vector_t center = {(MAX.x - MIN.x)/2, MAX.y};
    center = (vector_t){
        sdl_text_center(MIN, MAX, "TURTLE RUN", DEFAULT_FONT, TEXT_HEIGHT), MIN.y};
    sdl_draw_outlined_text(window, "TURTLE RUN", DEFAULT_FONT, LIME, INDIGO, TEXT_HEIGHT,
                           THICK_OUTLINE, center);
    
    //Draw options:
    center = (vector_t){
                    sdl_text_center(MIN, MAX, "Play Game", DEFAULT_FONT, TEXT_HEIGHT),
                                    TEXT_SPACING*2};
    sdl_draw_outlined_text(window, "Play Game", DEFAULT_FONT, LIME, INDIGO, TEXT_HEIGHT,
                           THIN_OUTLINE, center);
    center = (vector_t){
                    sdl_text_center(MIN, MAX, "Instructions", DEFAULT_FONT, TEXT_HEIGHT),
                                    TEXT_SPACING*3};
    sdl_draw_outlined_text(window, "Instructions", DEFAULT_FONT, LIME, INDIGO,TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    center = (vector_t){
                    sdl_text_center(MIN, MAX, "High Scores", DEFAULT_FONT, TEXT_HEIGHT),
                                    TEXT_SPACING*4};
    sdl_draw_outlined_text(window, "High Scores", DEFAULT_FONT, LIME, INDIGO, TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    center = (vector_t){
                    sdl_text_center(MIN, MAX, "Quit Game", DEFAULT_FONT, TEXT_HEIGHT),
                                    TEXT_SPACING*5};
    sdl_draw_outlined_text(window, "Quit Game", DEFAULT_FONT, LIME, INDIGO, TEXT_HEIGHT,
                           THIN_OUTLINE, center);
}

void menu_play_game() {
    //double total_score;
    char *filename = "scores/highscore.txt";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Unable to open %s", filename);
    }
    double *highscore = malloc(sizeof(double) * 5);
    for (int i = 0; i < 5; i++) {
        fscanf(fp, "%lf", &highscore[i]);
        printf("%lf\n", highscore[i]);
    }
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_key((event_handler_t) player_move);
    sdl_on_click((event_handler_t) player_shoot);

    scene_t *scene = scene_init();
    vector_t *scroll_speed = malloc(sizeof(vector_t));
    *scroll_speed = DEFAULT_SCROLL_SPEED;
    double *score = malloc(sizeof(double));
    *score = 0;

    initialize_background(scene);
    initialize_player(scene);
    initialize_bounds(scene, MIN, MAX);
    initialize_terrain(scene);
    frame_spawn_random(scene, MAX, MAX.x, score);

    body_t *player = scene_get_body(scene, 3);
    player_entity_t *player_entity = body_get_info(player);
    create_bounds_collisions(scene, player, PLAYER_RADIUS);

    double total_time = 0.0;
    double time_since_last_enemy = 0;
    double time_since_last_powerup = 0;
    double time_since_last_speedup = 0;
    double distance_since_last_frame = 0;

    //Every tick inside "Play Game":
    while (!sdl_is_done(scene)) {

        //double dt = fmax(fmin(time_since_last_tick(), MAX_DT), MIN_DT);
        double dt = time_since_last_tick();
        total_time += dt;
        time_since_last_enemy += dt;
        distance_since_last_frame += dt*(-(scroll_speed->x));
        time_since_last_powerup += dt;
        time_since_last_speedup += dt;
        if (time_since_last_enemy > ENEMY_INTERVAL) {
            enemy_spawn_random(scene, MIN, MAX);
            time_since_last_enemy = 0;
        }
        if (distance_since_last_frame >= MAX.x) {
            frame_spawn_random(scene, MAX, MAX.x, score);
            distance_since_last_frame = 0;
        }
        if (time_since_last_powerup > POWERUP_INTERVAL) {
            powerup_spawn_random(scene, MIN, MAX, scroll_speed);
            time_since_last_powerup = 0;
        }
        if (time_since_last_speedup > SPEEDUP_INTERVAL) {
            scroll_speed->x = fmin(
                scroll_speed->x + DEFAULT_SPEEDUP *
                (strcmp(entity_get_powerup(player_entity), "SLOW")? 1:0.5),
                MAX_SPEED);
            time_since_last_speedup = 0;
            for (int i = 0; i < 3 ; i++){
                sprite_t *sprite = body_get_draw_info(scene_get_body(scene, i));
                sprite_set_dt(sprite, 0);
            }
        }
        *score = *score + advanced_score_calculation(total_time);
        
        sidescroll(scene, scroll_speed, dt);
        scene_tick(scene, dt);
        sdl_render_scene(scene);
        if (check_game_end(scene)) {
            //total_score = *score;
            if (*score > highscore[4]) {
                printf("I am in this loop\n");
                highscore[4] = *score;
            }
            break;
        }
    }
    printf("%lf\n", highscore[4]);
    for (int k = 0; k < 5; k++) {
        for (int h = k + 1; h < 5; h++) {
            if (highscore[k] < highscore[h]) {
                double temp = highscore[k];
                highscore[k] = highscore[h];
                highscore[h] = temp;
            }
        }
    }
    FILE *fp2 = fopen(filename, "w");
    for (int j = 0; j < 5; j++) {
        fprintf(fp2, "%lf\n", highscore[j]);
    }
    fclose(fp2);
    fclose(fp);
    sdl_on_key(NULL);
    sdl_on_click(NULL);
    free(scene);
    free(scroll_speed);
    free(score);
    free(highscore);
    SDL_DestroyWindow(window);
}

void menu_instructions() {
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_click((event_handler_t)click_to_continue);
    draw_background();

    vector_t center = (vector_t){
        sdl_text_center(MIN, MAX, "INSTRUCTIONS", DEFAULT_FONT, TEXT_HEIGHT), MIN.y};
    sdl_draw_outlined_text(window, "INSTRUCTIONS", DEFAULT_FONT, LIME, INDIGO,TEXT_HEIGHT,
                           THICK_OUTLINE, center);
    
    char *message = "Press AD or arrow keys to move left and right.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*2};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);
    
    message = "Press W or the up arrow to jump.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*3};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    message = "Dodge enemies and avoid going off-screen.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*4};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);
    
    message = "Left-click to shoot water drops at enemies.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*5};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    message = "Collect powerups to gain special powers.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*6};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    message = "Collect coins and survive to gain points.";
    center = (vector_t){
                    sdl_text_center(MIN, MAX, message, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*7};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);
    
    message = "Left-click anywhere to continue...";
    center = (vector_t){MIN.x+TEXT_OFFSET, SMALL_TEXT_SPACING*9};
    sdl_draw_outlined_text(window, message, DEFAULT_FONT, LIME, INDIGO, SMALL_TEXT_HEIGHT,
                           THIN_OUTLINE, center);

    while (!sdl_is_done(window)) {
    }
}

int is_in_button_bounds(double x, double y, vector_t box, int width) {
    return x > box.x && x < box.x+width &&
           y > box.y-TEXT_SPACING+TEXT_OFFSET && y < box.y-TEXT_OFFSET;
}

void menu_mouse_handler(char key, mouse_event_type_t type, double held_time,
                        void *window);

void show_window(SDL_Window *window) {
    sdl_set_window(window);
    SDL_ShowWindow(window);
    sdl_on_click((event_handler_t)menu_mouse_handler);
    display_main_menu(window);
}

void menu_mouse_handler(char key, mouse_event_type_t type, double held_time,
                        void *window){
    vector_t box1 = {sdl_text_center(MIN, MAX, "Play Game", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*2};
    int width1 = sdl_text_width("Play Game", DEFAULT_FONT, TEXT_HEIGHT);
    vector_t box2 = {sdl_text_center(MIN, MAX, "Instructions", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*3};
    int width2 = sdl_text_width("Play Game", DEFAULT_FONT, TEXT_HEIGHT);
    vector_t box3 = {sdl_text_center(MIN, MAX, "High Scores", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*4};
    int width3 = sdl_text_width("Play Game", DEFAULT_FONT, TEXT_HEIGHT);
    vector_t box4 = {sdl_text_center(MIN, MAX, "Quit Game", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*5};
    int width4 = sdl_text_width("Play Game", DEFAULT_FONT, TEXT_HEIGHT);

    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK:
                vector_t mouse_coords = sdl_mouse_pos();
                double x = mouse_coords.x;
                double y = mouse_coords.y;
                if (is_in_button_bounds(x, y, box1, width1)) {
                    SDL_HideWindow(window);
                    menu_play_game();
                    show_window(window);
                }
                else if (is_in_button_bounds(x, y, box2, width2)) {
                    SDL_HideWindow(window);
                    menu_instructions();
                    show_window(window);
                }
                else if (is_in_button_bounds(x, y, box3, width3)) {
                    printf("HIGH SCORES\n");
                    //TODO
                }
                else if (is_in_button_bounds(x, y, box4, width4)) {
                    SDL_DestroyWindow(window);
                    Mix_HaltMusic();
                    exit(0);
                }
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    //double newscore = 293857629346;
    list_t *hi = get_high_scores();
    printf("%lf\n", list_get(hi, 0));
    printf("%lf\n", list_get(hi, 1));
    printf("%lf\n", list_get(hi, 2));
    printf("%lf\n", list_get(hi, 3));
    printf("%lf\n", list_get(hi, 4));


    time_t t;
    srand((unsigned) time(&t));
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
    Mix_Music *soundtrack = loadMedia("sounds/synth.wav");
    jump = loadEffects("sounds/jump1.wav");
    slide = loadEffects("sounds/sliding.wav");
    shot = loadEffects("sounds/shoot.wav");

    SDL_Window *window = sdl_init(MIN,MAX);
    display_main_menu(window);
    Mix_PlayMusic(soundtrack, -1);
    sdl_on_click((event_handler_t) menu_mouse_handler);

    while (!sdl_is_done(window)) {
    }
    return 0;
}