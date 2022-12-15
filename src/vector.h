#pragma once
#include <stdbool.h>

struct vec2
{
    float x, y;
};

struct vec3
{
    float x, y, z;
};

struct vec4
{
    float x, y, z, w;
};

struct mat4
{
    union
    {
        struct
        {
            float m11, m12, m13, m14,
                  m21, m22, m23, m24,
                  m31, m32, m33, m34,
                  m41, m42, m43, m44;
        };
        struct
        {
            float vals[16];
        };
        struct
        {
            struct
            {
                struct vec3 xyz;
                float w;
            } rows[4];
        };
    };
};

struct ivec3
{
    int x, y, z;
};

struct vec2 vec2_create(float x, float y);
bool vec2_eq(struct vec2 v1, struct vec2 v2);
struct vec2 vec2_neg(struct vec2 v);
struct vec2 vec2_add(struct vec2 lhs, struct vec2 rhs);
struct vec2 vec2_sub(struct vec2 lhs, struct vec2 rhs);
struct vec2 vec2_mul(struct vec2 v, float rhs);
struct vec2 vec2_div(struct vec2 v, float rhs);
void vec2_add_eq(struct vec2 *lhs, struct vec2 rhs);
void vec2_sub_eq(struct vec2 *lhs, struct vec2 rhs);
void vec2_mul_eq(struct vec2 *v, float rhs);
void vec2_div_eq(struct vec2 *v, float rhs);
float vec2_dot(struct vec2 v1, struct vec2 v2);
float vec2_length(struct vec2 v);
float vec2_length2(struct vec2 v);
struct vec2 vec2_normalize(struct vec2 v);
float vec2_angle(struct vec2 v1, struct vec2 v2);
struct vec2 vec2_rand();
struct vec2 vec2_randrange(float min, float max);
struct vec2 vec2_approach(struct vec2 val, struct vec2 target, float amount);

struct vec3 vec3_create(float x, float y, float z);
bool vec3_eq(struct vec3 v1, struct vec3 v2);
struct vec3 vec3_neg(struct vec3 v);
struct vec3 vec3_add(struct vec3 lhs, struct vec3 rhs);
struct vec3 vec3_sub(struct vec3 lhs, struct vec3 rhs);
struct vec3 vec3_mul(struct vec3 v, float rhs);
struct vec3 vec3_div(struct vec3 v, float rhs);
void vec3_add_eq(struct vec3 *lhs, struct vec3 rhs);
void vec3_sub_eq(struct vec3 *lhs, struct vec3 rhs);
void vec3_mul_eq(struct vec3 *v, float rhs);
void vec3_div_eq(struct vec3 *v, float rhs);
float vec3_dot(struct vec3 v1, struct vec3 v2);
struct vec3 vec3_cross(struct vec3 v1, struct vec3 v2);
float vec3_length(struct vec3 v);
float vec3_length2(struct vec3 v);
struct vec3 vec3_normalize(struct vec3 v);
struct vec3 vec3_vmul(struct vec3 lhs, struct vec3 rhs);
struct vec3 vec3_rand();
struct vec3 vec3_randrange(float min, float max);
struct vec3 vec3_approach(struct vec3 val, struct vec3 target, float amount);
struct vec3 vec3_reflect(struct vec3 dir, struct vec3 norm);

struct vec4 vec4_create(float x, float y, float z, float w);
struct vec4 vec4_div(struct vec4 v, float rhs);

struct ivec3 ivec3_create(int x, int y, int z);
struct ivec3 ivec3_add(struct ivec3 lhs, struct ivec3 rhs);
bool ivec3_equal(struct ivec3 a, struct ivec3 b);

struct mat4 mat4_create(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44);
struct mat4 mat4_zero();
struct mat4 mat4_identity();
// TODO: Should probably pass const pointers instead
struct mat4 mat4_add(struct mat4 lhs, struct mat4 rhs);
struct mat4 mat4_sub(struct mat4 lhs, struct mat4 rhs);
struct mat4 mat4_mul(struct mat4 lhs, struct mat4 rhs);
struct vec3 mat4_v3mul(struct mat4 m, struct vec3 v);
struct vec4 mat4_v4mul(struct mat4 m, struct vec4 v);
struct mat4 mat4_fmul(struct mat4 m, float f);
struct mat4 mat4_scale(struct vec3 s);
struct mat4 mat4_translate(struct vec3 t);
struct mat4 mat4_rotx(float rad);
struct mat4 mat4_roty(float rad);
struct mat4 mat4_rotz(float rad);
struct mat4 mat4_rot(float rad, struct vec3 axis);
struct mat4 mat4_ortho(float left, float right, float bottom, float top,
        float near, float far);
struct mat4 mat4_perspective(float fov, float ratio, float near, float far);
struct mat4 mat4_lookat(struct vec3 at, struct vec3 target, struct vec3 up);
struct mat4 mat4_transpose(struct mat4 m);
struct mat4 mat4_remove_translation(struct mat4 m);

void vec2_print(struct vec2 v);
void vec3_print(struct vec3 v);
void vec4_print(struct vec4 v);
void mat4_print(struct mat4 m);

extern const struct vec2 VEC2_ZERO;
extern const struct vec2 VEC2_UP;
extern const struct vec2 VEC2_DOWN;
extern const struct vec2 VEC2_RIGHT;
extern const struct vec2 VEC2_LEFT;
extern const struct vec2 VEC2_ONE;

extern const struct vec3 VEC3_ZERO;
extern const struct vec3 VEC3_UP;
extern const struct vec3 VEC3_DOWN;
extern const struct vec3 VEC3_RIGHT;
extern const struct vec3 VEC3_LEFT;
extern const struct vec3 VEC3_FORWARD;
extern const struct vec3 VEC3_BACK;
extern const struct vec3 VEC3_ONE;
