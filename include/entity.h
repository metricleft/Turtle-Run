#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <stdbool.h>
#include "body.h"

/**
 * A number of tags that a body may have if it is an entity in a game.
 */
typedef struct entity entity_t;

/**
 * Initializes an entity without any info.
 * Acts like body_init_with_info() where info and info_freer are NULL.
 */
entity_t *entity_init(char *entity_type, bool scrollable, bool fallable);

/**
 * Frees an entity.
 */
void entity_free(entity_t *entity);

//TODO: consider changing to "returns a copy of the entity type of an entity" to encapsulate
/**
 * Returns the entity type of an entity.
 * 
 * @param entity a pointer to an entity
 * @return entity_type of the entity
 */
char *entity_get_type(entity_t *entity);

/**
 * Returns the state of scrollable of an entity.
 * 
 * @param entity a pointer to an entity
 * @return a boolean representing whether scrollable is true or false.
 */
bool entity_get_scrollable(entity_t *entity);

/**
 * Returns the state of fallable of an entity.
 * 
 * @param entity a pointer to an entity
 * @return a boolean representing whether fallable is true or false.
 */
bool entity_get_fallable(entity_t *entity);

/**
 * Returns the state of is_scrolling of an entity.
 * 
 * @param entity a pointer to an entity
 * @return a boolean representing whether is_scrolling is true or false.
 */
bool entity_is_scrolling(entity_t *entity);

/**
 * Sets is_scrolling of an entity to true.
 * 
 * @param entity a pointer to an entity
 */
void entity_set_scrolling(entity_t *entity);

#endif // #ifndef __ENTITY_H__
