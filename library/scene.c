#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include "scene.h"

const size_t DEFAULT_CAPACITY = 30;

//Stores a force that acts on a list of bodies.
typedef struct force {
    void *info;
    force_creator_t force;
    free_func_t info_freer;
    list_t *force_bodies;
} force_t;

typedef struct scene {
    list_t *bodies;
    list_t *forces;
} scene_t;

void force_free(force_t *force) {
    if (force->info_freer != NULL){
        force->info_freer(force->info);
    }
    free(force->force_bodies);
    free(force);
}

scene_t *scene_init(void){
    scene_t *scene = malloc(sizeof(scene_t));
    assert(scene != NULL);
    scene->bodies = list_init(DEFAULT_CAPACITY, body_free);
    scene->forces = list_init(DEFAULT_CAPACITY, force_free);
    return scene;
}

void scene_free(scene_t *scene){
    list_free(scene -> bodies);
    list_free(scene -> forces);
    free(scene);
}

size_t scene_bodies(scene_t *scene){
    return list_size(scene->bodies);
}

body_t *scene_get_body(scene_t *scene, size_t index){
    return list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body){
    list_add(scene->bodies, body);
}

void scene_remove_body(scene_t *scene, size_t index){
    body_remove(list_get(scene->bodies, index));
}

void scene_remove_force(scene_t *scene, size_t index){
    force_t *removed = list_remove(scene->forces, index);
    force_free(removed);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux, 
                             free_func_t freer){
    scene_add_bodies_force_creator(scene, forcer, aux, NULL, freer);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                                        list_t *bodies, free_func_t freer) {
    force_t *new_force = malloc(sizeof(force_t));
    *new_force = (force_t){aux, forcer, freer, bodies};
    list_add(scene->forces, new_force);
}

void scene_tick(scene_t *scene, double dt){
    for (size_t i = 0; i < list_size(scene->forces); i++) {
        force_t *curr =  list_get(scene->forces, i);
        curr->force(curr->info);
    }
    for (size_t i = 0; i < list_size(scene->bodies); i++) {
        body_tick(list_get(scene->bodies, i), dt);
    }
    for (int i = ((int) list_size(scene->forces))-1; i >= 0; i--) {
        force_t *cur = list_get(scene->forces, i);
        bool removed = false;
        if (cur->force_bodies != NULL) {
            for (int j = 0; j < list_size(cur->force_bodies); j++) {
                if (body_is_removed(list_get(cur->force_bodies,j))) {
                    removed = true;
                    break;
                }
            }
        }
        if (removed) {
            scene_remove_force(scene, i);
        }
    }
    for (int i = ((int) list_size(scene->bodies))-1; i >= 0; i--) {
        if (body_is_removed(list_get(scene->bodies, i))) {
            body_t *removed = list_remove(scene->bodies, i);
            body_free(removed);
        }
    }
}
