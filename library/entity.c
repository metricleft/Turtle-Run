#include <stdbool.h>
#include <stdlib.h>
#include "entity.h"

/*
TODO:
- entity stores a behavior function for enemies
- entity stores some sort of code that corresponds to its sprite image
*/

/*
List of entity_type tags:
PLAYER - the player character. Stored at index 0 in the scene.
ENEMY - a movable enemy with its own unique behavior.
TERRAIN - a piece of terrain. Not fallable.
POWERUP - a powerup with its own unique behavior. Includes coins.
*/

typedef struct entity {
    char *entity_type;
    bool scrollable;
    bool fallable;
    bool is_scrolling;
} entity_t;

entity_t *entity_init(char *entity_type, bool scrollable, bool fallable) {
    entity_t *entity = malloc(sizeof(entity_t));
    entity->entity_type = entity_type;
    entity->scrollable = scrollable;
    entity->fallable = fallable;
    entity->is_scrolling = false;
    return entity;
}

void entity_free(entity_t *entity) {
    free(entity->entity_type);
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
