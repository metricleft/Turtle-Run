#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <float.h>

#include "sdl_wrapper.h"
#include "enemy.h"
#include "entity.h"
#include "frame.h"
#include "entity.h"
#include "powerup.h"
#include "shapelib.h"
#include "bounds.h"
#include "forces.h"

const vector_t MIN = {.x = 0, .y = 0};
const vector_t MAX = {.x = 1000, .y = 500};

const char *JUMP_ADD = "sounds/jump1.wav";
const char *SLIDE_ADD = "sounds/sliding.wav";
const char *SOUNDTRACK_ADD = "sounds/synth.wav";
const char *SHOOT_ADD = "sounds/shoot.wav";

const int ARC_RESOLUTION = 10;

const double MAX_DT = 0.01;
const double MIN_DT = 1e-6;

const double BULLET_RADIUS = 6;
const double BULLET_MASS = 0.2;
const char *BULLET_SPRITE = "static/bullet.png";

const int POWERUP_INTERVAL = 15;
const int ENEMY_INTERVAL = 10;
const int SPEEDUP_INTERVAL = 5;

const vector_t DEFAULT_GRAVITY = {0, -1000};
const vector_t DEFAULT_SCROLL_SPEED = {-200, 0};
const double DEFAULT_SPEEDUP = -50;
const double MAX_SPEED = 700;

const double PLAYER_SPEED = 600;
const double PLAYER_RADIUS = 30;
const double PLAYER_MASS = 10;
const char *PLAYER_SPRITE = "static/turtle_spritesheet2.png";
const double PLAYER_SCALE = 2;
const int PLAYER_FRAMES = 8;
const int PLAYER_FPS = 6;

const char *DEFAULT_FONT = "static/Arcade.ttf";
const int TEXT_HEIGHT = 40;
const int SMALL_TEXT_HEIGHT = 20;
const int THICK_OUTLINE = 4;
const int THIN_OUTLINE = 2;
const int TEXT_SPACING = 80;
const int SMALL_TEXT_SPACING = 45;
const int TEXT_OFFSET = 10;

const int NUM_HIGHSCORES = 5;
const int NUM_ACHIEVEMENTS = 4;
const char *HIGHSCORES_FILE = "saves/highscore.tr";
const char *ACHIEVEMENTS_FILE = "saves/lifetime.tr";

const char *SKY_IMG = "static/background_sky.png";
const char *GRASS_IMG = "static/background_grass.png";
const char *WATER_IMG = "static/background_water.png";
const char *BACKGROUND_IMG = "static/background.png";
const SDL_Rect BACKGROUND_FRAME = {0,0, 256, 128};

const double ELASTIC_COLLISION = 1;
const double INELASTIC_COLLISION = 0;

list_t *get_global_achievements() {
    // Feteches all achievements from the achievements file.
    list_t *global_achievements = list_init(NUM_ACHIEVEMENTS, free);
    FILE *fp = fopen(ACHIEVEMENTS_FILE, "r");
    if (fp == NULL) {
        printf("Unable to open %s\n", ACHIEVEMENTS_FILE);
    }
    double **achievement = malloc(sizeof(double *) * NUM_ACHIEVEMENTS);
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        achievement[i] = malloc(sizeof(double));
        fscanf(fp, "%lf", achievement[i]);
    }
    for (int j = 0; j < NUM_ACHIEVEMENTS; j++) {
        list_add(global_achievements, achievement[j]);
    }
    fclose(fp);
    free(achievement);
    return global_achievements;
}


list_t *get_high_scores() {
    // Fetches all high scores from the high score file.
    list_t *high_scores = list_init(NUM_HIGHSCORES, free);
    FILE *fp = fopen(HIGHSCORES_FILE, "r");
    if (fp == NULL) {
        printf("Unable to open %s\n", HIGHSCORES_FILE);
    }
    double **highscore = malloc(sizeof(double *) * NUM_HIGHSCORES);
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        highscore[i] = malloc(sizeof(double));
        fscanf(fp, "%lf", highscore[i]);
    }
    for (int j = 0; j < NUM_HIGHSCORES; j++) {
        list_add(high_scores, highscore[j]);
    }
    fclose(fp);
    free(highscore);
    return high_scores;
}

double advanced_score_calculation(double dt) {
    // Perfoerms advanced score calculation based on total time survived.
    assert(dt >= 0);
    if (dt <= 10) {
        return 10;
    }
    else if (dt <= 20) {
        return 20;
    }
    else if (dt <= 30) {
        return 30;
    }
    return 50;
}

bool check_game_end(scene_t *scene) {
    //Check if player is gone: lose condition.
    entity_t *entity = body_get_info(scene_get_body(scene, 3));
    if (strcmp(entity_get_type(entity), "PLAYER")) {
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

void add_text(scene_t *scene, vector_t coords, char *text, bool outlined, bool small) {
    char *text_copy = malloc(sizeof(char)*strlen(text)+1);
    strcpy(text_copy, text);
    text_info_t *info;
    if (outlined) 
    {
        info = outlined_text_info_init(text_copy, DEFAULT_FONT, LIME, BLACK,
                                       small? SMALL_TEXT_HEIGHT:TEXT_HEIGHT,
                                       small? THIN_OUTLINE:THICK_OUTLINE,
                                       coords);
    }
    else {
        info = text_info_init(text_copy, DEFAULT_FONT, LIME,
                              small? SMALL_TEXT_HEIGHT:TEXT_HEIGHT,
                              coords);
    }
    double h = small? SMALL_TEXT_HEIGHT:TEXT_HEIGHT;
    double w = sdl_text_width(text_copy, DEFAULT_FONT, (int)h);
    body_t *body = body_init(compute_rect_points(coords, w, h), INFINITY);
    body_set_draw(body, outlined? sdl_draw_outlined_text:sdl_draw_text, info,
                  text_info_free);
    scene_add_body(scene, body);
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
    Mix_Chunk *jump = loadEffects(JUMP_ADD);
    Mix_Chunk *slide = loadEffects(SLIDE_ADD);
    body_t *player = scene_get_body(scene, 3);
    player_entity_t *entity = body_get_info(player);
    vector_t new_velocity = {0, body_get_velocity(player).y};
    if (type == KEY_PRESSED) {
        switch (key) {
            case LEFT_ARROW: {
                new_velocity.x = -PLAYER_SPEED;
                if (held_time < 0.2) {
                    Mix_PlayChannel(-1, slide, 0);
                }
                break;
            }
            case RIGHT_ARROW: {
                new_velocity.x = PLAYER_SPEED;
                if (held_time < 0.2) {
                    Mix_PlayChannel(-1, slide, 0);
                }
                break;
            }
            case UP_ARROW: {
                if (held_time < 0.2 && (entity_get_colliding(entity) ||
                                        !strcmp(entity_get_powerup(entity), "JUMP"))) {
                    new_velocity.y = PLAYER_SPEED;
                    Mix_PlayChannel(-1, jump, 0);
                    entity_set_colliding(entity, false);
                }
                break;
            }
        }
    }
    body_set_velocity(player, new_velocity);
}

void player_shoot(char key, mouse_event_type_t type, double held_time, void *scene){
    Mix_Chunk *shot = loadEffects(SHOOT_ADD);
    body_t *player = scene_get_body(scene, 3);
    vector_t new_velocity = {0, 0};
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK: {
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
}

void click_to_continue(char key, mouse_event_type_t type, double held_time,
                        void *window){
    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK: {
                SDL_Event *event = malloc(sizeof(event));
                event->type = SDL_QUIT;
                SDL_PushEvent(event);
                break;
            }
        }
    }
}

void display_main_menu(SDL_Window *window) {
    scene_t *scene = scene_init();
    add_background(scene, BACKGROUND_IMG, 0);

    char *text = "TURTLE RUN";
    vector_t center = {sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT),
                       TEXT_HEIGHT};
    add_text(scene, center, text, true, false);

    text = "Play Game";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT),
                        TEXT_SPACING*2};
    add_text(scene, center, text, true, false);

    text = "Instructions";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT),
                        TEXT_SPACING*3};
    add_text(scene, center, text, true, false);

    text = "High Scores";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT),
                        TEXT_SPACING*4};
    add_text(scene, center, text, true, false);

    text = "Quit Game";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT),
                        TEXT_SPACING*5};
    add_text(scene, center, text, true, false);

    sdl_render_scene(scene);
    scene_free(scene);
}

void display_score(list_t *achievements, double *score) {
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_click((event_handler_t)click_to_continue);

    scene_t *scene = scene_init();
    add_background(scene, BACKGROUND_IMG, 0);

    char *text = "FINAL SCORE";
    vector_t center = (vector_t){
        sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT), TEXT_HEIGHT};
    add_text(scene, center, text, true, false);

    text = "Congratulations!";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*3};
    add_text(scene, center, text, true, true);
    
    text = malloc(sizeof(char)*(strlen("Your final score is: ") + DBL_DIG + 1));
    snprintf(text, strlen("Your final score is: ") + DBL_DIG + 1,
             "Your final score is: %.0f", *score);
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*4};
    add_text(scene, center, text, true, true);
    free(text);
    free(score);

    text = malloc(sizeof(char)*(strlen("You collected  coins") + DBL_DIG + 1));
    if (*(double *)list_get(achievements, 2)==1) {
        snprintf(text, strlen("You collected 1 coin") + 1,
                 "You collected 1 coin");
    }
    else {
        snprintf(text, strlen("You collected  coins") + DBL_DIG + 1,
            "You collected %.0f coins", fmax(0, *(double *)list_get(achievements, 2)));
    }
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*5};
    add_text(scene, center, text, true, true);
    free(text);

    text = malloc(sizeof(char)*(strlen("You collected  powerups") + DBL_DIG+1));
    if (*(double *)list_get(achievements, 3) == 1) {
        snprintf(text, strlen("You collected 1 powerup") + 1,
                 "You collected 1 powerup");
    }
    else {
        snprintf(text, strlen("You collected  powerups") + DBL_DIG + 1,
            "You collected %.0f powerups", fmax(0, *(double *)list_get(achievements, 3)));
    }
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*6};
    add_text(scene, center, text, true, true);
    free(text);
    list_free(achievements);

    text = "Left-click anywhere to continue...";
    center = (vector_t){MIN.x+TEXT_OFFSET, SMALL_TEXT_SPACING*10};
    add_text(scene, center, text, true, true);

    sdl_render_scene(scene);
    scene_free(scene);

    while (!sdl_is_done(window)) {
    }
    SDL_DestroyWindow(window);
}

void menu_play_game() {
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_key((event_handler_t) player_move);
    sdl_on_click((event_handler_t) player_shoot);

    scene_t *scene = scene_init();
    vector_t *scroll_speed = malloc(sizeof(vector_t));
    *scroll_speed = DEFAULT_SCROLL_SPEED;
    double *score = malloc(sizeof(double));
    *score = 0;
    list_t *achievements = list_init(NUM_ACHIEVEMENTS, free);
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++) {
        double *temp = malloc(sizeof(double));
        *temp = 0.0;
        list_add(achievements, temp);
    }

    initialize_background(scene);
    initialize_player(scene);
    initialize_bounds(scene, MIN, MAX);
    initialize_terrain(scene);
    frame_spawn_random(scene, MAX, MAX.x, score, achievements);

    body_t *player = scene_get_body(scene, 3);
    player_entity_t *player_entity = body_get_info(player);
    create_bounds_collisions(scene, player, PLAYER_RADIUS);

    double total_time = 0.0;
    double time_since_last_enemy = 0;
    double time_since_last_powerup = 0;
    double time_since_last_speedup = 0;
    double distance_since_last_frame = 0;

    char *score_text = malloc(sizeof(char)*(DBL_DIG) + 1);
    sprintf(score_text, "%.0f", *score);
    vector_t score_coords = {TEXT_OFFSET, TEXT_OFFSET};
    text_info_t *score_text_info = text_info_init(score_text, DEFAULT_FONT,
            RED, TEXT_HEIGHT, score_coords);
    
    char *coins_text = malloc(sizeof(char)*(DBL_DIG) + 1);
    sprintf(coins_text, "%.0f", *(double *) list_get(achievements, 2));
    vector_t coins_coords = {TEXT_OFFSET, TEXT_HEIGHT + TEXT_OFFSET};
    text_info_t *coins_text_info = text_info_init(
        coins_text,
        DEFAULT_FONT,
        YELLOW,
        SMALL_TEXT_HEIGHT,
        coins_coords
    );

    char *powerup_text = malloc(sizeof(char)*(DBL_DIG) + 1);
    sprintf(powerup_text, "%s", entity_get_powerup(player_entity));
    vector_t powerup_coords = {TEXT_OFFSET, TEXT_HEIGHT+SMALL_TEXT_HEIGHT+TEXT_OFFSET};
    text_info_t *powerup_text_info = text_info_init(
        powerup_text,
        DEFAULT_FONT,
        LIME,
        SMALL_TEXT_HEIGHT,
        powerup_coords
    );

    //Every tick inside "Play Game":
    while (!sdl_is_done(scene)) {
        double dt = fmax(fmin(time_since_last_tick(), MAX_DT), MIN_DT);
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
            frame_spawn_random(scene, MAX, MAX.x, score, achievements);
            distance_since_last_frame = 0;
        }
        if (time_since_last_powerup > POWERUP_INTERVAL) {
            powerup_spawn_random(scene, MIN, MAX, scroll_speed, achievements);
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
        sprintf(score_text, "%.0f", *score);
        sprintf(coins_text, "%.0f", *(double *) list_get(achievements, 2));
        sprintf(powerup_text, "%s", entity_get_powerup(player_entity));

        sidescroll(scene, scroll_speed, dt);
        scene_tick(scene, dt);
        sdl_render_scene_with_score(scene, score_text_info, coins_text_info,
                powerup_text_info);
        if (check_game_end(scene)) {
            break;
        }
    }
    *(double *)list_get(achievements, 1) = 1;
    *(double *)list_get(achievements, 0) = *score;

    list_t *global_achievements = get_global_achievements();
    FILE *lifetime_file = fopen(ACHIEVEMENTS_FILE, "w");
    for (int i = 0; i < NUM_ACHIEVEMENTS; i++){
        *(double *)list_get(global_achievements, i) =
                                        *(double *)list_get(global_achievements, i) +
                                        *(double *)list_get(achievements, i);
        fprintf(lifetime_file, "%lf\n", *(double *)list_get(global_achievements, i));
    }
    fclose(lifetime_file);
    list_free(global_achievements);

    list_t *highscores = get_high_scores();
    FILE *highscores_file = fopen(HIGHSCORES_FILE, "w");
    if (*score > *(double *)list_get(highscores, 4)) {
            *(double *)list_get(highscores, 4) = *score;
    }
    for (int k = 0; k < NUM_HIGHSCORES; k++) {
        for (int h = k + 1; h < NUM_HIGHSCORES; h++) {
            if (*(double *)list_get(highscores, k) < *(double *)list_get(highscores, h)) {
                double temp = *(double *)list_get(highscores, k);
                *(double *)list_get(highscores, k) = *(double *)list_get(highscores, h);
                *(double *)list_get(highscores, h) = temp;
            }
        }
    }
    for (int j = 0; j < NUM_HIGHSCORES; j++) {
        fprintf(highscores_file, "%lf\n", *(double *)list_get(highscores, j));
    }
    fclose(highscores_file);
    list_free(highscores);

    sdl_on_key(NULL);
    sdl_on_click(NULL);
    scene_free(scene);
    free(scroll_speed);
    SDL_DestroyWindow(window);

    display_score(achievements, score);
}

void menu_instructions() {
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_click((event_handler_t)click_to_continue);

    scene_t *scene = scene_init();
    add_background(scene, BACKGROUND_IMG, 0);

    char *text = "INSTRUCTIONS";
    vector_t center = (vector_t){
        sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT), TEXT_HEIGHT};
    add_text(scene, center, text, true, false);

    text = "Press AD or arrow keys to move left and right.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*3};
    add_text(scene, center, text, true, true);
    
    text = "Press W or the up arrow to jump.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*4};
    add_text(scene, center, text, true, true);

    text = "Dodge enemies and avoid going off-screen.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*5};
    add_text(scene, center, text, true, true);
    
    text = "Left-click to shoot water drops at enemies.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*6};
    add_text(scene, center, text, true, true);

    text = "Collect powerups to gain special powers.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*7};
    add_text(scene, center, text, true, true);

    text = "Collect coins and survive to gain points.";
    center = (vector_t){sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                        SMALL_TEXT_SPACING*8};
    add_text(scene, center, text, true, true);
    
    text = "Left-click anywhere to continue...";
    center = (vector_t){MIN.x + TEXT_OFFSET, SMALL_TEXT_SPACING*10};
    add_text(scene, center, text, true, true);

    sdl_render_scene(scene);
    scene_free(scene);

    while (!sdl_is_done(window)) {
    }
    SDL_DestroyWindow(window);
}

void menu_highscores() {
    SDL_Window *window = sdl_init(MIN, MAX);
    sdl_on_click((event_handler_t)click_to_continue);

    scene_t *scene = scene_init();
    add_background(scene, BACKGROUND_IMG, 0);

    list_t *achievements = get_global_achievements();

    char *text = "HIGH SCORES";
    vector_t center = (vector_t){
        sdl_text_center(MIN, MAX, text, DEFAULT_FONT, TEXT_HEIGHT), TEXT_HEIGHT};
    add_text(scene, center, text, true, false);

    list_t *scores = get_high_scores();
    for (int i = 0; i < NUM_HIGHSCORES; i++) {
        text = malloc(sizeof(char)*(DBL_DIG+strlen("1. ")) + 1);
        snprintf(text, DBL_DIG + strlen("1. ") + 1, "%d. %.0f", i+1,
                 fmax(0, *(double *)list_get(scores, i)));

        center = (vector_t){
                    sdl_text_center(MIN, MAX, text, DEFAULT_FONT, SMALL_TEXT_HEIGHT),
                    SMALL_TEXT_SPACING*(i*0.8+2.5)};
        add_text(scene, center, text, true, true);
        free(text);
    }

    text = malloc(sizeof(char)*(strlen("Lifetime score: ") + DBL_DIG+1));
    snprintf(text, strlen("Lifetime score: ") + DBL_DIG + 1,
             "Lifetime score: %.0f", fmax(0, *(double *)list_get(achievements, 0)));
    center = (vector_t){MIN.x+TEXT_OFFSET, SMALL_TEXT_SPACING*7};
    add_text(scene, center, text, true, true);
    free(text);

    text = malloc(sizeof(char)*(strlen("Lifetime games: ") + DBL_DIG+1));
    snprintf(text, strlen("Lifetime games: ") + DBL_DIG + 1,
             "Lifetime games: %.0f", fmax(0, *(double *)list_get(achievements, 1)));
    center = (vector_t){MIN.x+TEXT_OFFSET, SMALL_TEXT_SPACING*8};
    add_text(scene, center, text, true, true);
    free(text);

    text = malloc(sizeof(char)*(strlen("Lifetime coins: ") + DBL_DIG+1));
    snprintf(text, strlen("Lifetime coins: ") + DBL_DIG + 1,
             "Lifetime coins: %.0f", fmax(0, *(double *)list_get(achievements, 2)));
    center = (vector_t){(MAX.x-MIN.x)/2+TEXT_OFFSET, SMALL_TEXT_SPACING*7};
    add_text(scene, center, text, true, true);
    free(text);

    text = malloc(sizeof(char)*(strlen("Lifetime powerups: ") + DBL_DIG+1));
    snprintf(text, strlen("Lifetime powerups: ") + DBL_DIG + 1,
             "Lifetime powerups: %.0f", fmax(0, *(double *)list_get(achievements, 3)));
    center = (vector_t){(MAX.x-MIN.x)/2+TEXT_OFFSET, SMALL_TEXT_SPACING*8};
    add_text(scene, center, text, true, true);
    free(text);

    text = "Left-click anywhere to continue...";
    center = (vector_t){MIN.x+TEXT_OFFSET, SMALL_TEXT_SPACING*10};
    add_text(scene, center, text, true, true);

    sdl_render_scene(scene);
    scene_free(scene);

    while (!sdl_is_done(window)) {
    }
    SDL_DestroyWindow(window);
}

int is_in_button_bounds(double x, double y, vector_t box, int width) {
    return x > box.x && x < box.x+width &&
           y > box.y-TEXT_HEIGHT && y < box.y;
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
    int width2 = sdl_text_width("Instructions", DEFAULT_FONT, TEXT_HEIGHT);
    vector_t box3 = {sdl_text_center(MIN, MAX, "High Scores", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*4};
    int width3 = sdl_text_width("High Scores", DEFAULT_FONT, TEXT_HEIGHT);
    vector_t box4 = {sdl_text_center(MIN, MAX, "Quit Game", DEFAULT_FONT, TEXT_HEIGHT),
                     MAX.y - TEXT_SPACING*5};
    int width4 = sdl_text_width("Quit Game", DEFAULT_FONT, TEXT_HEIGHT);

    if (type == BUTTON_PRESSED) {
        switch (key) {
            case LEFT_CLICK: {
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
                    SDL_HideWindow(window);
                    menu_highscores();
                    show_window(window);
                }
                else if (is_in_button_bounds(x, y, box4, width4)) {
                    SDL_Event *event = malloc(sizeof(event));
                    event->type = SDL_QUIT;
                    SDL_PushEvent(event);
                }
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 );
    Mix_Music *soundtrack = loadMedia(SOUNDTRACK_ADD);
    Mix_PlayMusic(soundtrack, -1);

    time_t t;
    srand((unsigned) time(&t));

    SDL_Window *window = sdl_init(MIN,MAX);
    display_main_menu(window);
    sdl_on_click((event_handler_t) menu_mouse_handler);

    while (!sdl_is_done(window)) {
    }
    SDL_DestroyWindow(window);
    Mix_HaltMusic();
    exit(0);
    return 0;
}