#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "entity.h"

typedef struct entity {
    char *entity_type;
    bool scrollable;
    bool fallable;
    bool is_scrolling;
} entity_t;

typedef struct player_entity {
    char *entity_type;
    bool scrollable;
    bool fallable;
    bool is_scrolling;
    bool is_colliding;
    char *active_powerup;
    int num_coins;
} player_entity_t;

entity_t *entity_init(char *entity_type, bool scrollable, bool fallable) {
    entity_t *entity = malloc(sizeof(entity_t));
    entity->entity_type = entity_type;
    entity->scrollable = scrollable;
    entity->fallable = fallable;
    entity->is_scrolling = false;
    return entity;
}

player_entity_t *player_entity_init(char *entity_type, bool scrollable, bool fallable) {
    player_entity_t *player_entity = malloc(sizeof(player_entity_t));
    player_entity->entity_type = entity_type;
    player_entity->scrollable = scrollable;
    player_entity->fallable = fallable;
    player_entity->is_scrolling = false;
    player_entity->is_colliding = false;
    player_entity->active_powerup = "NONE";
    player_entity->num_coins = 0;
    return player_entity;
}

void entity_free(entity_t *entity) {
    free(entity);
}

char *entity_get_type(entity_t *entity) {
    return entity->entity_type;
}

bool entity_get_scrollable(entity_t *entity) {
    return entity->scrollable;
}

bool entity_get_fallable(entity_t *entity) {
    return entity->fallable;
}

bool entity_is_scrolling(entity_t *entity) {
    return entity->is_scrolling;
}

void entity_set_scrolling(entity_t *entity) {
    entity->is_scrolling = true;
}

bool entity_get_colliding(player_entity_t *entity) {
    return entity->is_colliding;
}

void entity_set_colliding(player_entity_t *entity, bool value) {
    entity->is_colliding = value;
}

char *entity_get_powerup(player_entity_t *entity) {
    return entity->active_powerup;
}

void entity_set_powerup(player_entity_t *entity, char *new_powerup) {
    entity->active_powerup = new_powerup;
}
