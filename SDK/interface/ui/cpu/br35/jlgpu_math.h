#ifndef __GPU_MATH_H__
#define __GPU_MATH_H__

// #include <stdint.h>
#include "asm/cpu.h"
#include "system/includes.h"

#define GPU_MATH_CODE		AT(.math_api.text.cache.L2)

typedef struct {

    float m[3][3];

} gpu_matrix_t;

typedef struct {

    float x;
    float y;

} gpu_point2_t;

typedef struct {

    float x;
    float y;
    float z;
} gpu_point3_t;

typedef struct {
    int minx;
    int maxx;
    int miny;
    int maxy;
} gpu_boundbox_t;


float gpu_sinf(float x);
float gpu_cosf(float x);
gpu_matrix_t *gpu_matrix_create();
void gpu_matrix_free_(gpu_matrix_t *m);
void gpu_matrix_set_identity(gpu_matrix_t *m);
void gpu_matrix_set_translate(gpu_matrix_t *m, float tx, float ty);
void gpu_matrix_set_scale(gpu_matrix_t *m, float sx, float sy);
void gpu_matrix_set_shear(gpu_matrix_t *m, float shx, float shy);
void gpu_matrix_set_rotate(gpu_matrix_t *m, float a);
void gpu_matrix_set_flip(gpu_matrix_t *m, int x_en, int y_en);

void gpu_matrix_multiply(gpu_matrix_t *d, gpu_matrix_t *s0, gpu_matrix_t *s1);
void gpu_matrix_mul_const(gpu_matrix_t *m, float f);
int gpu_matrix_is_affine(gpu_matrix_t *m);
int gpu_matrix_invert(gpu_matrix_t *dm, gpu_matrix_t *m);


void gpu_matrix_translate(gpu_matrix_t *m, float tx, float ty);
void gpu_matrix_scale(gpu_matrix_t *m, float sx, float sy);
void gpu_matrix_shear(gpu_matrix_t *m, float shx, float shy);
void gpu_matrix_rotate(gpu_matrix_t *m, float a);
void gpu_matrix_flip(gpu_matrix_t *m, int x_en, int y_en);

void gpu_transform_point2(gpu_point2_t *dp, gpu_matrix_t *m, gpu_point2_t *sp);

void gpu_point2_add(gpu_point2_t *d, gpu_point2_t *s0, gpu_point2_t *s1);
void gpu_point2_sub(gpu_point2_t *d, gpu_point2_t *s0, gpu_point2_t *s1);
void gpu_point2_mul(gpu_point2_t *d, gpu_point2_t *s0, gpu_point2_t *s1);
float gpu_point2_len(gpu_point2_t *s0, gpu_point2_t *s1);

void gpu_matrix_get_boundbox(gpu_matrix_t *matrix, gpu_boundbox_t *src, gpu_boundbox_t *dst);

int gpu_matrix_calc_by_point(gpu_matrix_t *matrix, int pt_w, int pt_h, float points[8]);

#endif
