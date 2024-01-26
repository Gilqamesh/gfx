#ifndef MATRIX_H
# define MATRIX_H

# include <math.h>
# include <string.h>
# include <assert.h>
# include <stdbool.h>

struct         vec2;
struct         vec3;
struct         vec4;
struct         mat4;
struct         quat;
typedef struct vec2   vec2_t;
typedef struct vec3   vec3_t;
typedef struct vec4   vec4_t;
typedef struct mat4   mat4_t; // column major 4x4 matrix
typedef struct quat   quat_t;

struct vec2   { float _[2]; };
struct vec3   { float _[3]; };
struct vec4   { float _[4]; };
struct mat4   { float _[16]; };
struct quat   { float _[4]; };

static inline vec2_t vec2(float a, float b) { return (vec2_t) { { a, b } }; }
static inline vec2_t vec2__null() { return vec2(0.0f, 0.0f); }
static inline vec2_t vec2__dup_self(vec2_t* self) { return vec2(self->_[0], self->_[1]); }
static inline vec2_t vec2__dup(vec2_t a) { return vec2__dup_self(&a); }
static inline bool   vec2__eq_self(vec2_t* self, vec2_t a) { return self->_[0] == a._[0] && self->_[1] == a._[1]; }
static inline bool   vec2__eq(vec2_t a, vec2_t b) { return vec2__eq_self(&a, b); }
static inline void   vec2__add_self(vec2_t* self, vec2_t a) { self->_[0] += a._[0]; self->_[1] += a._[1]; }
static inline vec2_t vec2__add(vec2_t a, vec2_t b) { vec2__add_self(&a, b); return a; }
static inline void   vec2__sub_self(vec2_t* self, vec2_t a) { self->_[0] -= a._[0]; self->_[1] -= a._[1]; }
static inline vec2_t vec2__sub(vec2_t a, vec2_t b) { vec2__sub_self(&a, b); return a; }
static inline void   vec2__mul_self(vec2_t* self, float f) { self->_[0] *= f; self->_[1] *= f; }
static inline vec2_t vec2__mul(vec2_t a, float f) { vec2__mul_self(&a, f); return a; }
static inline void   vec2__div_self(vec2_t* self, float f) { assert(f != 0.0f); self->_[0] /= f; self->_[1] /= f; }
static inline vec2_t vec2__div(vec2_t a, float f) { vec2__div_self(&a, f); return a; }
static inline void   vec2__neg_self(vec2_t* self) { self->_[0] = -self->_[0]; self->_[1] = -self->_[1]; }
static inline vec2_t vec2__neg(vec2_t a) { vec2__neg_self(&a); return a; }
static inline float  vec2__inner(vec2_t a, vec2_t b) { return a._[0] * b._[0] + a._[1] * b._[1]; }
static inline float  vec2__len_self(vec2_t* self) { return sqrt(self->_[0] * self->_[0] + self->_[1] * self->_[1]); }
static inline float  vec2__len(vec2_t a) { return vec2__len_self(&a); }
static inline float  vec2__dist(vec2_t a, vec2_t b) { return vec2__len(vec2__sub(b, a)); }
static inline float  vec2__angl(vec2_t a, vec2_t b) { const float denom = vec2__len(a) * vec2__len(b); assert(denom != 0.0f); return acos(vec2__inner(a, b) / denom); }
static inline void   vec2__norm_self(vec2_t* self) { vec2__div_self(self, vec2__len_self(self)); }
static inline vec2_t vec2__norm(vec2_t a) { vec2__norm_self(&a); return a; }
static inline vec2_t vec2__clamp(vec2_t a, vec2_t min, vec2_t max) {
    a._[0] = a._[0] < min._[0] ? min._[0] : a._[0];
    a._[1] = a._[1] < min._[1] ? min._[1] : a._[1];
    a._[0] = a._[0] < max._[0] ? a._[0] : max._[0];
    a._[1] = a._[1] < max._[1] ? a._[1] : max._[1];
    return a;
}
static inline void   vec2__reflect_self(vec2_t* self, vec2_t normal) {
    // v' = v - 2(nv)n
    vec2__norm_self(&normal);
    vec2__sub_self(self, vec2__mul(normal, 2.0f * vec2__inner(vec2__dup_self(self), normal)));
}
static inline vec2_t vec2__reflect(vec2_t a, vec2_t normal) { vec2__reflect_self(&a, normal); return a; }
/**
 * @note https://en.wikipedia.org/wiki/List_of_refractive_indices
 * @param refractive_index_of_material_1 [1.0f, )
 * @param refractive_index_of_material_2 [1.0f, )
*/
static inline void   vec2__refract_self(vec2_t* self, vec2_t normal, float refractive_index_of_material_1, float refractive_index_of_material_2) {
    assert(refractive_index_of_material_1 >= 1.0f);
    assert(refractive_index_of_material_2 >= 1.0f);
    /**
     * k = 1 - n^2 * (1 - (vN)^2)
     * if (k >= 0)
     *  R = n * v - (n * (vN) + sqrt(k))N
     * else
     *  R = 0.0
    */
    vec2__norm_self(&normal);
    const float n = refractive_index_of_material_1 / refractive_index_of_material_2;
    const float inner = vec2__inner(normal, vec2__dup_self(self));
    float k = 1.0f - n * n * (1.0f - inner * inner);
    if (k >= 0) {
        vec2__mul_self(
            self,
            n
        );
        vec2__sub_self(
            self,
            vec2__mul(
                normal,
                n * inner + sqrt(k)
            )
        );
    } else {
        self->_[0] = 0.0f;
        self->_[1] = 0.0f;
    }
}
static inline vec2_t vec2__refract(vec2_t a, vec2_t normal, float refractive_index_of_material_1, float refractive_index_of_material_2) { vec2__refract_self(&a, normal, refractive_index_of_material_1, refractive_index_of_material_2); return a; }
static inline vec2_t vec2__lerp(vec2_t a, vec2_t b, float t) { vec2__mul_self(&a, 1.0f - t); vec2__mul_self(&b, t); vec2__add_self(&a, b); return a; }

static inline vec3_t vec3(float a, float b, float c) { return (vec3_t) { { a, b, c } }; }
static inline vec3_t vec3__null() { return vec3(0.0f, 0.0f, 0.0f); }
static inline vec3_t vec3__dup_self(vec3_t* self) { return vec3(self->_[0], self->_[1], self->_[2]); }
static inline vec3_t vec3__dup(vec3_t a) { return vec3__dup_self(&a); }
static inline bool   vec3__eq_self(vec3_t* self, vec3_t a) { return self->_[0] == a._[0] && self->_[1] == a._[1] && self->_[2] == a._[2]; }
static inline bool   vec3__eq(vec3_t a, vec3_t b) { return vec3__eq_self(&a, b); }
static inline void   vec3__add_self(vec3_t* self, vec3_t a) { self->_[0] += a._[0]; self->_[1] += a._[1]; self->_[2] += a._[2]; }
static inline vec3_t vec3__add(vec3_t a, vec3_t b) { vec3__add_self(&a, b); return a; }
static inline void   vec3__sub_self(vec3_t* self, vec3_t a) { self->_[0] -= a._[0]; self->_[1] -= a._[1]; self->_[2] -= a._[2]; }
static inline vec3_t vec3__sub(vec3_t a, vec3_t b) { vec3__sub_self(&a, b); return a; }
static inline void   vec3__mul_self(vec3_t* self, float f) { self->_[0] *= f; self->_[1] *= f; self->_[2] *= f; }
static inline vec3_t vec3__mul(vec3_t a, float f) { vec3__mul_self(&a, f); return a; }
static inline void   vec3__div_self(vec3_t* self, float f) { assert(f != 0.0f); self->_[0] /= f; self->_[1] /= f; self->_[2] /= f; }
static inline vec3_t vec3__div(vec3_t a, float f) { vec3__div_self(&a, f); return a; }
static inline void   vec3__neg_self(vec3_t* self) { self->_[0] = -self->_[0]; self->_[1] = -self->_[1]; self->_[2] = -self->_[2]; }
static inline vec3_t vec3__neg(vec3_t a) { vec3__neg_self(&a); return a; }
static inline float  vec3__inner(vec3_t a, vec3_t b) { return a._[0] * b._[0] + a._[1] * b._[1] + a._[2] * b._[2]; }
static inline vec3_t vec3__outer(vec3_t a, vec3_t b) { return vec3( a._[1] * b._[2] - a._[2] * b._[1], a._[2] * b._[0] - a._[0] * b._[2], a._[0] * b._[1] - a._[1] * b._[0] ); }
static inline float  vec3__len_self(vec3_t* self) { return sqrt(self->_[0] * self->_[0] + self->_[1] * self->_[1] + self->_[2] * self->_[2]); }
static inline float  vec3__len(vec3_t a) { return vec3__len_self(&a); }
static inline float  vec3__dist(vec3_t a, vec3_t b) { return vec3__len(vec3__sub(b, a)); }
static inline float  vec3__angl(vec3_t a, vec3_t b) { const float denom = vec3__len(a) * vec3__len(b); assert(denom != 0.0f); return acos(vec3__inner(a, b) / denom); }
static inline void   vec3__norm_self(vec3_t* self) { vec3__div_self(self, vec3__len_self(self)); }
static inline vec3_t vec3__norm(vec3_t a) { vec3__norm_self(&a); return a; }
static inline vec3_t vec3__clamp(vec3_t a, vec3_t min, vec3_t max) {
    a._[0] = a._[0] < min._[0] ? min._[0] : a._[0];
    a._[1] = a._[1] < min._[1] ? min._[1] : a._[1];
    a._[2] = a._[2] < min._[2] ? min._[2] : a._[2];
    a._[0] = a._[0] < max._[0] ? a._[0] : max._[0];
    a._[1] = a._[1] < max._[1] ? a._[1] : max._[1];
    a._[2] = a._[2] < max._[2] ? a._[2] : max._[2];
    return a;
}
static inline void   vec3__reflect_self(vec3_t* self, vec3_t normal) {
    // v' = v - 2(vn)n
    vec3__norm_self(&normal);
    vec3__sub_self(self, vec3__mul(normal, 2.0f * vec3__inner(vec3__dup_self(self), normal)));
}
static inline vec3_t vec3__reflect(vec3_t a, vec3_t normal) { vec3__reflect_self(&a, normal); return a; }
/**
 * @note https://en.wikipedia.org/wiki/List_of_refractive_indices
 * @param refractive_index_of_material_1 [1.0f, )
 * @param refractive_index_of_material_2 [1.0f, )
*/
static inline void   vec3__refract_self(vec3_t* self, vec3_t normal, float refractive_index_of_material_1, float refractive_index_of_material_2) {
    assert(refractive_index_of_material_1 >= 1.0f);
    assert(refractive_index_of_material_2 >= 1.0f);
    /**
     * k = 1 - n^2 * (1 - (vN)^2)
     * if (k >= 0)
     *  R = n * v - (n * (vN) + sqrt(k))N
     * else
     *  R = 0.0
    */
    vec3__norm_self(&normal);
    const float n = refractive_index_of_material_1 / refractive_index_of_material_2;
    const float inner = vec3__inner(normal, vec3__dup_self(self));
    float k = 1.0f - n * n * (1.0f - inner * inner);
    if (k >= 0) {
        vec3__mul_self(
            self,
            n
        );
        vec3__sub_self(
            self,
            vec3__mul(
                normal,
                n * inner + sqrt(k)
            )
        );
    } else {
        self->_[0] = 0.0f;
        self->_[1] = 0.0f;
        self->_[2] = 0.0f;
    }
}
static inline vec3_t vec3__refract(vec3_t a, vec3_t normal, float refractive_index_of_material_1, float refractive_index_of_material_2) { vec3__refract_self(&a, normal, refractive_index_of_material_1, refractive_index_of_material_2); return a; }
static inline vec3_t vec3__lerp(vec3_t a, vec3_t b, float t) { vec3__mul_self(&a, 1.0f - t); vec3__mul_self(&b, t); vec3__add_self(&a, b); return a; }

static inline vec4_t vec4(float a, float b, float c, float d) { return (vec4_t) { { a, b, c, d } }; }
static inline vec4_t vec4__null() { return vec4(0.0f, 0.0f, 0.0f, 0.0f); }
static inline vec4_t vec4__dup_self(vec4_t* self) { return vec4(self->_[0], self->_[1], self->_[2], self->_[3]); }
static inline vec4_t vec4__dup(vec4_t a) { return vec4(a._[0], a._[1], a._[2], a._[3]); }
static inline bool   vec4__eq_self(vec4_t* self, vec4_t a) { return self->_[0] == a._[0] && self->_[1] == a._[1] && self->_[2] == a._[2] && self->_[3] == a._[3]; }
static inline bool   vec4__eq(vec4_t a, vec4_t b) { return vec4__eq_self(&a, b); }
static inline void   vec4__add_self(vec4_t* self, vec4_t a) { self->_[0] += a._[0]; self->_[1] += a._[1]; self->_[2] += a._[2]; self->_[3] += a._[3]; }
static inline vec4_t vec4__add(vec4_t a, vec4_t b) { vec4__add_self(&a, b); return a; }
static inline void   vec4__sub_self(vec4_t* self, vec4_t a) { self->_[0] -= a._[0]; self->_[1] -= a._[1]; self->_[2] -= a._[2]; self->_[3] -= a._[3]; }
static inline vec4_t vec4__sub(vec4_t a, vec4_t b) { vec4__sub_self(&a, b); return a; }
static inline void   vec4__mul_self(vec4_t* self, float f) { self->_[0] *= f; self->_[1] *= f; self->_[2] *= f; self->_[3] *= f; }
static inline vec4_t vec4__mul(vec4_t a, float f) { vec4__mul_self(&a, f); return a; }
static inline void   vec4__div_self(vec4_t* self, float f) { assert(f != 0.0f); self->_[0] /= f; self->_[1] /= f; self->_[2] /= f; self->_[3] /= f; }
static inline vec4_t vec4__div(vec4_t a, float f) { vec4__div_self(&a, f); return a; }
static inline void   vec4__neg_self(vec4_t* self) { self->_[0] = -self->_[0]; self->_[1] = -self->_[1]; self->_[2] = -self->_[2]; self->_[3] = -self->_[3]; }
static inline vec4_t vec4__neg(vec4_t a) { vec4__neg_self(&a); return a; }
static inline float  vec4__inner(vec4_t a, vec4_t b) { return a._[0] * b._[0] + a._[1] * b._[1] + a._[2] * b._[2] + a._[3] * a._[3]; }
static inline float  vec4__len_self(vec4_t* self) { return sqrt(self->_[0] * self->_[0] + self->_[1] * self->_[1] + self->_[2] * self->_[2] + self->_[3] * self->_[3]); }
static inline float  vec4__len(vec4_t a) { return vec4__len_self(&a); }
static inline void   vec4__norm_self(vec4_t* self) { vec4__div_self(self, vec4__len_self(self)); }
static inline vec4_t vec4__norm(vec4_t a) { vec4__norm_self(&a); return a; }
static inline vec4_t vec4__clamp(vec4_t a, vec4_t min, vec4_t max) {
    a._[0] = a._[0] < min._[0] ? min._[0] : a._[0];
    a._[1] = a._[1] < min._[1] ? min._[1] : a._[1];
    a._[2] = a._[2] < min._[2] ? min._[2] : a._[2];
    a._[3] = a._[3] < min._[3] ? min._[3] : a._[3];
    a._[0] = a._[0] < max._[0] ? a._[0] : max._[0];
    a._[1] = a._[1] < max._[1] ? a._[1] : max._[1];
    a._[2] = a._[2] < max._[2] ? a._[2] : max._[2];
    a._[3] = a._[3] < max._[3] ? a._[3] : max._[3];
    return a;
}
static inline void   vec4__reflect_self(vec4_t* self, vec4_t normal) {
    // v' = v - 2(vn)n
    vec4__norm_self(&normal);
    vec4__sub_self(self, vec4__mul(normal, 2.0f * vec4__inner(vec4__dup_self(self), normal)));
}
static inline vec4_t vec4__reflect(vec4_t a, vec4_t normal) { vec4__reflect_self(&a, normal); return a; }
static inline vec4_t vec4__lerp(vec4_t a, vec4_t b, float t) { vec4__mul_self(&a, 1.0f - t); vec4__mul_self(&b, t); vec4__add_self(&a, b); return a; }

static inline quat_t quat(float a, float b, float c, float d) { return (quat_t) { { a, b, c, d } }; }
static inline quat_t quat__null() { return quat(0.0f, 0.0f, 0.0f, 0.0f); }
static inline quat_t quat__dup_self(quat_t* self) { return quat(self->_[0], self->_[1], self->_[2], self->_[3]); }
static inline quat_t quat__dup(quat_t a) { return quat__dup_self(&a); }
static inline bool   quat__eq_self(quat_t* self, quat_t a) { return self->_[0] == a._[0] && self->_[1] == a._[1] && self->_[2] == a._[2] && self->_[3] == a._[3]; }
static inline bool   quat__eq(quat_t a, quat_t b) { return quat__eq_self(&a, b); }
static inline void   quat__add_self(quat_t* self, quat_t a) { self->_[0] += a._[0]; self->_[1] += a._[1]; self->_[2] += a._[2]; self->_[3] += a._[3]; }
static inline quat_t quat__add(quat_t a, quat_t b) { quat__add_self(&a, b); return a; }
static inline void   quat__sub_self(quat_t* self, quat_t a) { self->_[0] -= a._[0]; self->_[1] -= a._[1]; self->_[2] -= a._[2]; self->_[3] -= a._[3]; }
static inline quat_t quat__sub(quat_t a, quat_t b) { quat__sub_self(&a, b); return a; }
static inline void   quat__mul_self(quat_t* self, float f) { self->_[0] *= f; self->_[1] *= f; self->_[2] *= f; self->_[3] *= f; }
static inline quat_t quat__mul(quat_t a, float f) { quat__mul_self(&a, f); return a; }
static inline quat_t quat__ham_mul(quat_t a, quat_t b) {
    return quat(
        a._[0] * b._[0] - a._[1] * b._[1] - a._[2] * b._[2] - a._[3] * b._[3],
        a._[0] * b._[1] + a._[1] * b._[0] + a._[2] * b._[3] - a._[3] * b._[2],
        a._[0] * b._[2] - a._[1] * b._[3] + a._[2] * b._[0] + a._[3] * b._[1],
        a._[0] * b._[3] + a._[1] * b._[2] - a._[2] * b._[1] + a._[3] * b._[0]
    );
}
static inline void   quat__div_self(quat_t* self, float f) { assert(f != 0.0f); self->_[0] /= f; self->_[1] /= f; self->_[2] /= f; self->_[3] /= f; }
static inline quat_t quat__div(quat_t a, float f) { quat__div_self(&a, f); return a; }
static inline void   quat__neg_self(quat_t* self) { self->_[0] = -self->_[0]; self->_[1] = -self->_[1]; self->_[2] = -self->_[2]; self->_[3] = -self->_[3]; }
static inline quat_t quat__neg(quat_t a) { quat__neg_self(&a); return a; }
static inline float  quat__inner(quat_t a, quat_t b) { return a._[0] * b._[0] + a._[1] * b._[1] + a._[2] * b._[2] + a._[3] * a._[3]; }
static inline float  quat__len_self(quat_t* self) { return sqrt(self->_[0] * self->_[0] + self->_[1] * self->_[1] + self->_[2] * self->_[2] + self->_[3] * self->_[3]); }
static inline float  quat__len(quat_t a) { return quat__len_self(&a); }
static inline void   quat__norm_self(quat_t* self) { quat__div_self(self, quat__len_self(self)); }
static inline quat_t quat__norm(quat_t a) { quat__norm_self(&a); return a; }
static inline void   quat__conj_self(quat_t* self) { self->_[1] *= -1.0f; self->_[2] *= -1.0f; self->_[3] *= -1.0f; }
static inline quat_t quat__conj(quat_t a) { quat__conj_self(&a); return a; }
static inline quat_t quat__rot(vec3_t axis, float angle) {
    // qpq^(-1)
    const float sin_of_half_angle = sin(angle / 2.0f);
    return quat(cos(angle / 2.0f), axis._[0] * sin_of_half_angle, axis._[1] * sin_of_half_angle, axis._[2] * sin_of_half_angle);
}

static inline mat4_t mat4__null() { return (mat4_t) { { 0.0f } }; }
static inline mat4_t mat4__id() { return (mat4_t) { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f } }; }
static inline mat4_t mat4__dup_self(mat4_t* self) { mat4_t result; memcpy(&result, self, sizeof(*self)); return result; }
static inline mat4_t mat4__dup(mat4_t a) { return mat4__dup_self(&a); }
static inline bool   mat4__eq_self(mat4_t* self, mat4_t* other) { return memcmp(self, other, sizeof(*self)) == 0; }
static inline bool   mat4__eq(mat4_t a, mat4_t b) { return mat4__eq_self(&a, &b); }
static inline void   mat4__add_self(mat4_t* self, mat4_t a) { for (int i = 0; i < 16; ++i) self->_[i] += a._[i]; }
static inline mat4_t mat4__add(mat4_t a, mat4_t b) { mat4__add_self(&a, b); return a; }
static inline void   mat4__sub_self(mat4_t* self, mat4_t a) { for (int i = 0; i < 16; ++i) self->_[i] -= a._[i]; }
static inline mat4_t mat4__sub(mat4_t a, mat4_t b) { mat4__sub_self(&a, b); return a; }
static inline mat4_t mat4__mul(mat4_t a, mat4_t b) { mat4_t result = mat4__null(); for (int i = 0; i < 4; ++i) for (int j = 0; j < 4; ++j) { for (int k = 0; k < 4; ++k) { result._[i * 4 + j] += a._[i * 4 + k] * b._[k * 4 + j]; } } return result; }

static inline vec4_t mat4__mul_vec4(mat4_t a, vec4_t b) { vec4_t result = vec4__null(); for (int i = 0; i < 4; ++i) { for (int k = 0; k < 4; ++k) { result._[i] += a._[i * 4 + k] * b._[k]; } } return result; }
static inline void   mat4__tranlate_self(mat4_t* self, vec3_t b) { self->_[3] += b._[0]; self->_[7] += b._[1]; self->_[11] += b._[2]; }
static inline mat4_t mat4__tranlate(mat4_t a, vec3_t b) { mat4__tranlate_self(&a, b); return a; }
static inline mat4_t mat4__rot(vec3_t axis, float angle) {
    mat4_t result = mat4__null();
    const float cost = cos(angle);
    const float sint = sin(angle);
    result._[0]  = cost + axis._[0] * axis._[0] * (1.0f - cost);
    result._[1]  = axis._[0] * axis._[1] * (1.0f - cost) - axis._[2] * sint;
    result._[2]  = axis._[0] * axis._[2] * (1.0f - cost) + axis._[1] * sint;
    result._[4]  = axis._[0] * axis._[1] * (1.0f - cost) + axis._[2] * sint;
    result._[5]  = cost + axis._[1] * axis._[1] * (1.0f - cost);
    result._[6]  = axis._[1] * axis._[2] * (1.0f - cost) - axis._[0] * sint;
    result._[8]  = axis._[0] * axis._[2] * (1.0f - cost) - axis._[1] * sint;
    result._[9]  = axis._[1] * axis._[2] * (1.0f - cost) + axis._[0] * sint;
    result._[10] = cost + axis._[2] * axis._[2] * (1.0f - cost);
    result._[15] = 1.0f;
    return result;
}
static inline void   mat4__scale(mat4_t* self, vec3_t b) { self->_[0] *= b._[0]; self->_[5] *= b._[1]; self->_[10] *= b._[2]; }
static inline mat4_t mat4__look_at(vec3_t eye, vec3_t point_of_interest, vec3_t world_up) {
    mat4_t result = mat4__null();
    vec3_t forward = vec3__sub(point_of_interest, eye);
    vec3__norm_self(&forward);
    const vec3_t side = vec3__norm(vec3__outer(forward, world_up));
    world_up = vec3__outer(side, forward);
    result._[0]  = side._[0];
    result._[1]  = world_up._[0];
    result._[2]  = forward._[0];
    result._[3]  = -eye._[0];
    result._[4]  = side._[1];
    result._[5]  = world_up._[1];
    result._[6]  = forward._[1];
    result._[7]  = -eye._[1];
    result._[8]  = side._[2];
    result._[9]  = world_up._[2];
    result._[10] = forward._[2];
    result._[11] = -eye._[2];
    result._[15] = 1.0f;
    // result._[0]  = side._[0];
    // result._[1]  = side._[1];
    // result._[2]  = side._[2];
    // result._[4]  = world_up._[0];
    // result._[5]  = world_up._[1];
    // result._[6]  = world_up._[2];
    // result._[8]  = forward._[0];
    // result._[9]  = forward._[1];
    // result._[10] = forward._[2];
    // result._[12] = -eye._[0];
    // result._[13] = -eye._[1];
    // result._[14] = -eye._[2];
    // result._[15] = 1.0f;
    return result;
}
//! @brief Perspective projection matrix defined by a frustum
static inline mat4_t mat4__perspective_frustum(float left, float right, float bottom, float top, float near, float far) {
    assert(right > left && top > bottom && far > near);
    mat4_t result = mat4__null();
    result._[0]  = 2.0f * near / (right - left);
    result._[2]  = (right + left) / (right - left);
    result._[5]  = 2.0f * near / (top - bottom);
    result._[6]  = (top + bottom) / (top - bottom);
    result._[10] = (near + far) / (near - far);
    result._[11] = 2.0f * near * far / (near - far);
    result._[14] = -1.0f;
    // result._[0]  = 2.0f * near / (right - left);
    // result._[5]  = 2.0f * near / (top - bottom);
    // result._[8]  = (right + left) / (right - left);
    // result._[9]  = (top + bottom) / (top - bottom);
    // result._[10] = (near + far) / (near - far);
    // result._[11] = -1.0f;
    // result._[14] = 2.0f * near * far / (near - far);
    return result;
}
//! @brief Perspective projection matrix defined by an aspect ratio and fov
static inline mat4_t mat4__perspective_aspect(float fov_rad, float aspect, float near, float far) {
    assert(far > near);
    const float tan_half_of_fov = tan(fov_rad / 2.0f);
    mat4_t result = mat4__null();
    result._[0]  = 1.0f / (aspect * tan_half_of_fov);
    result._[5]  = 1.0f / tan_half_of_fov;
    result._[10] = -((far + near) / (far - near));
    result._[11] = -1.0f;
    result._[14] = -((2.0f * far * near) / (far - near));
    return result;
}
//! @brief Orthographic projection defined by a frustum
static inline mat4_t mat4__orthographic(float left, float right, float bottom, float top, float near, float far) {
    assert(right > left && top > bottom && far > near);
    mat4_t result = mat4__null();
    result._[0]  = 2.0f / (right - left);
    result._[3]  = (right + left) / (left - right);
    result._[5]  = 2.0f / (top - bottom);
    result._[7]  = (top + bottom) / (bottom - top);
    result._[10] = 2.0f / (near - far);
    result._[11] = (near + far) / (near - far);
    result._[15] = 1.0f;
    return result;
}

static inline vec3_t vec3__rot(vec3_t a, vec3_t axis, float angle) {
    quat_t q = quat__rot(axis, angle);
    quat_t p = quat(0.0f, a._[0], a._[1], a._[2]);
    quat_t qresult = quat__ham_mul(quat__ham_mul(q, p), quat__conj(q));
    return vec3(qresult._[1], qresult._[2], qresult._[3]);
}

#endif // MATRIX_H
