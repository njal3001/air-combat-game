#pragma once

struct vec3
{
    float x, y, z;
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

struct vec3 vec3_create(float x, float y, float z);

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
struct vec3 mat4_vmul(struct mat4 m, struct vec3 v);
struct mat4 mat4_fmul(struct mat4 m, float f);
struct mat4 mat4_scale(struct vec3 s);
struct mat4 mat4_translate(struct vec3 t);
struct mat4 mat4_rotx(float rad);
struct mat4 mat4_roty(float rad);
struct mat4 mat4_rotz(float rad);
struct mat4 mat4_rot(float rad, struct vec3 axis);
struct mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);
struct mat4 mat4_perspective(float fov, float ratio, float near, float far);
struct mat4 mat4_lookat(struct vec3 at, struct vec3 target, struct vec3 up);
struct mat4 mat4_transpose(struct mat4 m);
struct mat4 mat4_remove_translation(struct mat4 m);

void vec3_print(struct vec3 v);
void mat4_print(struct mat4 m);

extern const struct vec3 VEC3_ZERO;
extern const struct vec3 VEC3_UP;
extern const struct vec3 VEC3_DOWN;
extern const struct vec3 VEC3_RIGHT;
extern const struct vec3 VEC3_LEFT;
extern const struct vec3 VEC3_FORWARD;
extern const struct vec3 VEC3_BACK;
extern const struct vec3 VEC3_ONE;
