#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <stdbool.h>

/**
 * A number of tags that a body may have if it is an entity in a game.
 */
typedef struct entity entity_t;

/**
 * Child of entity: additional tags that a player may have.
 */
typedef struct player_entity player_entity_t;

/**
 * Initializes an entity.
 * 
 * @param entity_type type of entity: PLAYER, BULLET, ENEMY, TERRAIN, POWERUP
 * @param scrollable whether or not the entity should scroll with the screen
 * @param fallable whether or not the entity experiences gravity
 */
entity_t *entity_init(char *entity_type, bool scrollable, bool fallable);

/**
 * Initializes an entity.
 * 
 * @param entity_type type of entity: PLAYER, BULLET, ENEMY, TERRAIN, POWERUP
 * @param scrollable whether or not the entity should scroll with the screen
 * @param fallable whether or not the entity experiences gravity
 */
player_entity_t *player_entity_init(char *entity_type, bool scrollable, bool fallable);

/**
 * Frees an entity.
 */
void entity_free(entity_t *entity);

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

/**
 * Gets is_colliding of a player entity.
 * 
 * @param entity a pointer to a player entity
 * @return the boolean value of is_colliding of the entity
 */
bool entity_get_colliding(player_entity_t *entity);

/**
 * Sets is_colliding of a player entity.
 * 
 * @param entity a pointer to an entity
 * @param value the new value of is_colliding
 */
void entity_set_colliding(player_entity_t *entity, bool value);

/**
 * Gets the active powerup of a player entity.
 * 
 * @param entity a pointer to a player entity
 * @return the char * value of the player entity
 */
char *entity_get_powerup(player_entity_t *entity);

/**
 * Sets the active powerup of a player entity.
 * 
 * @param entity a pointer to a player entity
 * @param new_powerup the new value of active_powerup
 */
void entity_set_powerup(player_entity_t *entity, char *new_powerup);

#endif // #ifndef __ENTITY_H__
