#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "body.h"
#include "polygon.h"

typedef struct body { 
    list_t *shape;
    draw_func_t drawer;
    void* draw_info;
    double mass;
    vector_t centroid;
    vector_t velocity;
    double orientation;
    vector_t force;
    vector_t impulse;
    bool remove;
    void *info;
    free_func_t info_freer;
    free_func_t draw_freer;
} body_t;

body_t *body_init(list_t *shape, double mass){
    return(body_init_with_info(shape, mass, NULL, NULL));
    
}
body_t *body_init_with_info(list_t *shape,
                            double mass,
                            void *info,
                            free_func_t info_freer){
    body_t *body = malloc(sizeof(body_t));
    assert(body != NULL);
    body->shape = shape;
    body->drawer = NULL;
    body->draw_info = NULL;
    body->mass = mass;
    body->centroid = polygon_centroid(shape);
    body->velocity = VEC_ZERO;
    body->orientation = 0;
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
    body->remove = false;
    body->info = info;
    body->info_freer = info_freer;
    body->draw_freer = NULL;
    return body;
}


void body_free(body_t *body){
    list_free(body->shape);
    if (body->info_freer != NULL){
        body->info_freer(body->info);
    }
    if (body->draw_freer != NULL){
        body->draw_freer(body->draw_info);
    }
    free(body);
}

list_t *body_get_shape(body_t *body){
    list_t *ret_body = list_init(list_size(body->shape), free);
    for (int i = 0; i < list_size(body->shape); i++) {
        vector_t *to_add = malloc(sizeof(vector_t));
        memcpy(to_add, list_get(body->shape,i), sizeof(vector_t));
        list_add(ret_body, to_add);
    }
    return ret_body;
}

vector_t body_get_centroid(body_t *body){
    return body->centroid;
}

vector_t body_get_velocity(body_t *body){
    return body->velocity;
}

double body_get_mass(body_t *body){
    return body->mass;
}

void *body_get_info(body_t *body) {
    return body->info;
}

void *body_get_draw_info(body_t *body) {
    return body->draw_info;
}

double body_get_orientation(body_t *body){
    return body->orientation;
}

void body_set_draw(body_t *body, draw_func_t drawer, void* draw_info,
                   free_func_t draw_freer){
    body->drawer = drawer;
    body->draw_info = draw_info;
    body->draw_freer = draw_freer;
}

void body_set_centroid(body_t *body, vector_t x){
    polygon_translate(body->shape, vec_subtract(x, body->centroid));
    body->centroid = x;
}

void body_translate(body_t *body, vector_t v) {
    polygon_translate(body->shape, v);
    body->centroid = vec_add(v,body->centroid);
}

void body_set_velocity(body_t *body, vector_t v){
    body->velocity = v;
}

void body_set_rotation(body_t *body, double angle){
    polygon_rotate(body->shape, angle-body->orientation, body->centroid);
    body->orientation = angle;
}

void body_add_force(body_t *body, vector_t force){
    body->force = vec_add(body->force, force); 
}

void body_add_impulse(body_t *body, vector_t impulse){
    body->impulse = vec_add(body->impulse, impulse); 
}

void body_tick(body_t *body, double dt){
    vector_t dv_impulse = vec_multiply(1 / body->mass, body->impulse);
    vector_t dv_force = vec_multiply(dt / body->mass, body->force);
    vector_t old = body_get_velocity(body);
    body_set_velocity(body, vec_add(old, vec_add(dv_impulse, dv_force)));
    body_set_centroid(body, vec_add(body->centroid, 
                                    vec_multiply(dt/2, vec_add(old, body->velocity))));
    body->force = VEC_ZERO;
    body->impulse = VEC_ZERO;
}

void body_remove(body_t *body){
    body->remove = true;
}

bool body_is_removed(body_t *body){
    return body->remove;
}

void body_draw(body_t *body){
    if (body->drawer!= NULL){
       body->drawer(body, body->draw_info); 
    }
}
