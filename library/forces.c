#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "forces.h"
#include "test_util.h"
#include "test_util.h"
#include "collision.h"

#include <stdio.h>

const double SMALL_DISTANCE = 5;

typedef struct param {
    void *constant;
    body_t *body1;
    body_t *body2;
} param_t;

double calculate_reduced_mass(body_t *body1, body_t *body2) {
    double reduced_mass;
    if (body_get_mass(body1) == INFINITY) {
        reduced_mass = body_get_mass(body2);
    } else if (body_get_mass(body2) == INFINITY) {
        reduced_mass = body_get_mass(body1);
    } else {
        reduced_mass = body_get_mass(body1)*body_get_mass(body2) /
            (body_get_mass(body1)+body_get_mass(body2));
    }
    return reduced_mass;
}
 
void gravity_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1), body_get_centroid(aux->body2));
    double mass_product = body_get_mass(aux->body1)* body_get_mass(aux->body2);
    vector_t force = VEC_ZERO;
    if (sqrt(vec_dot(r, r)) > SMALL_DISTANCE){
        force = vec_multiply(-(double) aux->constant * mass_product / pow(sqrt(vec_dot(r,r)), 3.0) , r);
    }
    body_add_force(aux->body1, force);
    body_add_force(aux->body2, vec_negate(force));
}

void create_newtonian_gravity(scene_t *scene, double G, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){(void *) G, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, gravity_creator, force_param, bodies, free);
}

void const_force_creator(param_t *aux){
    vector_t force = vec_multiply(body_get_mass(aux->body1), *(vector_t *) aux->constant);
    body_add_force(aux->body1, force);
}

void create_constant_force(scene_t *scene, vector_t *a, body_t *body){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){a, body, NULL};
    list_t *bodies = list_init(1, body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, const_force_creator, force_param, bodies, free);

}

void spring_creator(param_t *aux){
    vector_t r = vec_subtract(body_get_centroid(aux->body1), body_get_centroid(aux->body2));
    vector_t force = vec_multiply(-1 * (double) aux->constant, r);
    body_add_force(aux->body1, force);
    body_add_force(aux->body2, vec_negate(force));
} 

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){(void *)k, body1, body2};
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    scene_add_bodies_force_creator(scene, spring_creator, force_param, bodies, free);
}

void drag_creator(param_t *aux){
    vector_t force = vec_multiply(- (double) aux->constant, body_get_velocity(aux->body1));
    body_add_force(aux->body1, force);
}

void create_drag(scene_t *scene, double gamma, body_t *body){
    param_t *force_param = malloc(sizeof(param_t));
    *force_param = (param_t){(void *) gamma, body, NULL};
    list_t *bodies = list_init(1, body_free);
    list_add(bodies, body);
    scene_add_bodies_force_creator(scene, drag_creator, force_param, bodies, free);
}

typedef struct {
    collision_handler_t handler;
    body_t *body1;
    body_t *body2;
    void *aux;
    bool collided;
} collision_param_t;

void collision_force_creator(collision_param_t *param) {
    list_t *shape1 = body_get_shape(param->body1);
    list_t *shape2 = body_get_shape(param->body2);
    collision_info_t collision = find_collision(shape1, shape2);
    if (collision.collided && !(param->collided)) {
        param->handler(param->body1, param->body2, collision.axis, param->aux);
        param->collided = true;
    } else if (!collision.collided) {
        param->collided = false;
    }
    free(shape1);
    free(shape2);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
            collision_handler_t handler, void *aux, free_func_t freer) {
    list_t *bodies = list_init(2, body_free);
    list_add(bodies, body1);
    list_add(bodies, body2);
    collision_param_t *force_param = malloc(sizeof(collision_param_t));
    *force_param = (collision_param_t) {handler, body1, body2, aux, false};
    scene_add_bodies_force_creator(scene, collision_force_creator, force_param,
                                        bodies, freer);
}


void physics_collision_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    double reduced_mass = calculate_reduced_mass(body1, body2);
    vector_t impulse = 
                vec_multiply(
                    reduced_mass * 
                    (1.0 + *(double *) aux) *
                    (vec_dot(body_get_velocity(body2),axis)-
                    vec_dot(body_get_velocity(body1),axis)),
                    axis);
    if (body_get_mass(body1) != INFINITY) {
        body_add_impulse(body1, impulse);
    }
    if (body_get_mass(body2) != INFINITY) {
        body_add_impulse(body2, vec_negate(impulse));
    }
}

void one_way_destroy_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    physics_collision_handler(body1, body2, axis, aux);
    body_remove(body2);
}

void two_way_destroy_handler(body_t *body1, body_t *body2,
                                vector_t axis, void *aux) {
    body_remove(body1);
    body_remove(body2);
}

void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2){
    create_collision(scene, body1, body2, two_way_destroy_handler,
                        NULL, free);
}

void create_oneway_destructive_collision(scene_t *scene, double elasticity, 
                                            body_t *body1, body_t *body2) {
    double *elasticity_param = malloc(sizeof(double));
    *elasticity_param = elasticity;
    create_collision(scene, body1, body2, one_way_destroy_handler,
                        elasticity_param, free);
}

void create_physics_collision(scene_t *scene, double elasticity,
                                    body_t *body1, body_t *body2) {
    double *elasticity_param = malloc(sizeof(double));
    *elasticity_param = elasticity;
    create_collision(scene, body1, body2, physics_collision_handler,
                        elasticity_param, free);
}
