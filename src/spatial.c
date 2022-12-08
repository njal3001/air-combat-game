#include "spatial.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "calc.h"

struct vec2 vec2_create(float x, float y)
{
    struct vec2 res;
    res.x = x;
    res.y = y;
    return res;
}

bool vec2_eq(struct vec2 v1, struct vec2 b)
{
    return v1.x == b.x && v1.y == b.y;
}

struct vec2 vec2_neg(struct vec2 v)
{
    struct vec2 res;
    res.x = -v.x;
    res.y = -v.y;

    return res;
}

struct vec2 vec2_add(struct vec2 lhs, struct vec2 rhs)
{
    struct vec2 res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;

    return res;
}

struct vec2 vec2_sub(struct vec2 lhs, struct vec2 rhs)
{
    struct vec2 res;
    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;

    return res;
}

struct vec2 vec2_mul(struct vec2 v, float rhs)
{
    struct vec2 res;
    res.x = v.x * rhs;
    res.y = v.y * rhs;

    return res;
}

struct vec2 vec2_div(struct vec2 v, float rhs)
{
    struct vec2 res;
    res.x = v.x / rhs;
    res.y = v.y / rhs;

    return res;
}

void vec2_add_eq(struct vec2 *lhs, struct vec2 rhs)
{
    lhs->x += rhs.x;
    lhs->y += rhs.y;
}

void vec2_sub_eq(struct vec2 *lhs, struct vec2 rhs)
{
    lhs->x -= rhs.x;
    lhs->y -= rhs.y;
}

void vec2_mul_eq(struct vec2 *v, float rhs)
{
    v->x *= rhs;
    v->y *= rhs;
}

void vec2_div_eq(struct vec2 *v, float rhs)
{
    v->x /= rhs;
    v->y /= rhs;
}

float vec2_dot(struct vec2 v1, struct vec2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

float vec2_length(struct vec2 v)
{
    return sqrtf(vec2_length2(v));
}

float vec2_length2(struct vec2 v)
{
    return v.x * v.x + v.y * v.y;
}

struct vec2 vec2_normalize(struct vec2 v)
{
    float length = vec2_length(v);
    if (length == 0) return v;

    return vec2_div(v, length);
}

float vec2_angle(struct vec2 v1, struct vec2 v2)
{
    return atan2(v1.x * v2.y - v1.y * v2.x, v1.x * v2.x + v1.y * v2.y);
}

// FIXME: Not uniform?
// Also should prevent zero vectors
struct vec2 vec2_rand()
{
    float x = frandrange(-1.0f, 1.0f);
    float y = frandrange(-1.0f, 1.0f);

    return vec2_normalize(vec2_create(x, y));
}

struct vec2 vec2_randrange(float min, float max)
{
    struct vec2 dir = vec2_rand();
    float len = frandrange(min, max);

    return vec2_mul(dir, len);
}

struct vec2 vec2_approach(struct vec2 val, struct vec2 target, float amount)
{
    struct vec2 diff = vec2_sub(target, val);
    float l2 = vec2_length2(diff);
    if (l2 <= amount * amount)
    {
        return target;
    }

    struct vec2 dir = vec2_normalize(diff);
    return vec2_add(val, vec2_mul(dir, amount));
}

struct vec3 vec3_create(float x, float y, float z)
{
    struct vec3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

bool vec3_eq(struct vec3 v1, struct vec3 v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

struct vec3 vec3_neg(struct vec3 v)
{
    struct vec3 res;
    res.x = -v.x;
    res.y = -v.y;
    res.z = -v.z;

    return res;
}

struct vec3 vec3_add(struct vec3 lhs, struct vec3 rhs)
{
    struct vec3 res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;

    return res;
}

struct vec3 vec3_sub(struct vec3 lhs, struct vec3 rhs)
{
    struct vec3 res;
    res.x = lhs.x - rhs.x;
    res.y = lhs.y - rhs.y;
    res.z = lhs.z - rhs.z;

    return res;
}

struct vec3 vec3_mul(struct vec3 v, float rhs)
{
    struct vec3 res;
    res.x = v.x * rhs;
    res.y = v.y * rhs;
    res.z = v.z * rhs;

    return res;
}

struct vec3 vec3_div(struct vec3 v, float rhs)
{
    struct vec3 res;
    res.x = v.x / rhs;
    res.y = v.y / rhs;
    res.z = v.z / rhs;

    return res;
}

void vec3_add_eq(struct vec3 *lhs, struct vec3 rhs)
{
    lhs->x += rhs.x;
    lhs->y += rhs.y;
    lhs->z += rhs.z;
}

void vec3_sub_eq(struct vec3 *lhs, struct vec3 rhs)
{
    lhs->x -= rhs.x;
    lhs->y -= rhs.y;
    lhs->z -= rhs.z;
}

void vec3_mul_eq(struct vec3 *v, float rhs)
{
    v->x *= rhs;
    v->y *= rhs;
    v->z *= rhs;
}

void vec3_div_eq(struct vec3 *v, float rhs)
{
    v->x /= rhs;
    v->y /= rhs;
    v->z /= rhs;
}

float vec3_dot(struct vec3 v1, struct vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

struct vec3 vec3_cross(struct vec3 v1, struct vec3 v2)
{
    struct vec3 res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.x - v1.x * v2.z;
    res.z = v1.x * v2.y - v1.y * v2.x;

    return res;
}

float vec3_length(struct vec3 v)
{
    return sqrtf(vec3_length2(v));
}

float vec3_length2(struct vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

struct vec3 vec3_normalize(struct vec3 v)
{
    float length = vec3_length(v);
    if (length == 0) return v;

    return vec3_div(v, length);
}

// FIXME: Not uniform?
// Also should prevent zero vectors
struct vec3 vec3_rand()
{
    float x = frandrange(-1.0f, 1.0f);
    float y = frandrange(-1.0f, 1.0f);
    float z = frandrange(-1.0f, 1.0f);

    return vec3_normalize(vec3_create(x, y, z));
}

struct vec3 vec3_randrange(float min, float max)
{
    struct vec3 dir = vec3_rand();
    float len = frandrange(min, max);

    return vec3_mul(dir, len);
}

struct vec3 vec3_approach(struct vec3 val, struct vec3 target, float amount)
{
    struct vec3 diff = vec3_sub(target, val);
    float l2 = vec3_length2(diff);
    if (l2 >= amount * amount)
    {
        return target;
    }

    struct vec3 dir = vec3_normalize(diff);
    return vec3_add(val, vec3_mul(dir, amount));
}

struct vec4 vec4_create(float x, float y, float z, float w)
{
    struct vec4 res;
    res.x = x;
    res.y = y;
    res.z = z;
    res.w = w;
    return res;
}

struct vec4 vec4_div(struct vec4 v, float rhs)
{
    struct vec4 res;
    res.x = v.x / rhs;
    res.y = v.y / rhs;
    res.z = v.z / rhs;
    res.w = v.w / rhs;

    return res;
}

struct ivec3 ivec3_create(int x, int y, int z)
{
    struct ivec3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

struct ivec3 ivec3_add(struct ivec3 lhs, struct ivec3 rhs)
{
    struct ivec3 res;
    res.x = lhs.x + rhs.x;
    res.y = lhs.y + rhs.y;
    res.z = lhs.z + rhs.z;

    return res;
}

bool ivec3_equal(struct ivec3 a, struct ivec3 b)
{
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct mat4 mat4_create(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44)
{
   struct mat4 res;
   res.m11 = m11;
   res.m12 = m12;
   res.m13 = m13;
   res.m14 = m14;
   res.m21 = m21;
   res.m22 = m22;
   res.m23 = m23;
   res.m24 = m24;
   res.m31 = m31;
   res.m32 = m32;
   res.m33 = m33;
   res.m34 = m34;
   res.m41 = m41;
   res.m42 = m42;
   res.m43 = m43;
   res.m44 = m44;

   return res;
}

struct mat4 mat4_zero()
{
    struct mat4 res;
    memset(&res, 0, sizeof(struct mat4));
    return res;
}
struct mat4 mat4_identity()
{
    struct mat4 res = mat4_zero();
    res.m11 = 1.0f;
    res.m22 = 1.0f;
    res.m33 = 1.0f;
    res.m44 = 1.0f;
    return res;
}

struct mat4 mat4_add(struct mat4 lhs, struct mat4 rhs)
{
    struct mat4 res;
    res.m11 = lhs.m11 + rhs.m11;
    res.m12 = lhs.m12 + rhs.m12;
    res.m13 = lhs.m13 + rhs.m13;
    res.m13 = lhs.m14 + rhs.m14;

    res.m21 = lhs.m21 + rhs.m21;
    res.m22 = lhs.m22 + rhs.m22;
    res.m23 = lhs.m23 + rhs.m23;
    res.m23 = lhs.m23 + rhs.m23;

    res.m31 = lhs.m31 + rhs.m31;
    res.m32 = lhs.m32 + rhs.m32;
    res.m33 = lhs.m33 + rhs.m33;
    res.m33 = lhs.m33 + rhs.m33;

    res.m41 = lhs.m41 + rhs.m41;
    res.m42 = lhs.m42 + rhs.m42;
    res.m43 = lhs.m43 + rhs.m43;
    res.m43 = lhs.m43 + rhs.m43;

    return res;
}

struct mat4 mat4_sub(struct mat4 lhs, struct mat4 rhs)
{
    struct mat4 res;
    res.m11 = lhs.m11 - rhs.m11;
    res.m12 = lhs.m12 - rhs.m12;
    res.m13 = lhs.m13 - rhs.m13;
    res.m13 = lhs.m14 - rhs.m14;

    res.m21 = lhs.m21 - rhs.m21;
    res.m22 = lhs.m22 - rhs.m22;
    res.m23 = lhs.m23 - rhs.m23;
    res.m23 = lhs.m23 - rhs.m23;

    res.m31 = lhs.m31 - rhs.m31;
    res.m32 = lhs.m32 - rhs.m32;
    res.m33 = lhs.m33 - rhs.m33;
    res.m33 = lhs.m33 - rhs.m33;

    res.m41 = lhs.m41 - rhs.m41;
    res.m42 = lhs.m42 - rhs.m42;
    res.m43 = lhs.m43 - rhs.m43;
    res.m43 = lhs.m43 - rhs.m43;

    return res;
}

struct mat4 mat4_mul(struct mat4 lhs, struct mat4 rhs)
{
    struct mat4 res;

    res.m11 = lhs.m11 * rhs.m11 + lhs.m12 * rhs.m21 + lhs.m13 * rhs.m31 + lhs.m14 * rhs.m41;
    res.m12 = lhs.m11 * rhs.m12 + lhs.m12 * rhs.m22 + lhs.m13 * rhs.m32 + lhs.m14 * rhs.m42;
    res.m13 = lhs.m11 * rhs.m13 + lhs.m12 * rhs.m23 + lhs.m13 * rhs.m33 + lhs.m14 * rhs.m43;
    res.m14 = lhs.m11 * rhs.m14 + lhs.m12 * rhs.m24 + lhs.m13 * rhs.m34 + lhs.m14 * rhs.m44;

    res.m21 = lhs.m21 * rhs.m11 + lhs.m22 * rhs.m21 + lhs.m23 * rhs.m31 + lhs.m24 * rhs.m41;
    res.m22 = lhs.m21 * rhs.m12 + lhs.m22 * rhs.m22 + lhs.m23 * rhs.m32 + lhs.m24 * rhs.m42;
    res.m23 = lhs.m21 * rhs.m13 + lhs.m22 * rhs.m23 + lhs.m23 * rhs.m33 + lhs.m24 * rhs.m43;
    res.m24 = lhs.m21 * rhs.m14 + lhs.m22 * rhs.m24 + lhs.m23 * rhs.m34 + lhs.m24 * rhs.m44;

    res.m31 = lhs.m31 * rhs.m11 + lhs.m32 * rhs.m21 + lhs.m33 * rhs.m31 + lhs.m34 * rhs.m41;
    res.m32 = lhs.m31 * rhs.m12 + lhs.m32 * rhs.m22 + lhs.m33 * rhs.m32 + lhs.m34 * rhs.m42;
    res.m33 = lhs.m31 * rhs.m13 + lhs.m32 * rhs.m23 + lhs.m33 * rhs.m33 + lhs.m34 * rhs.m43;
    res.m34 = lhs.m31 * rhs.m14 + lhs.m32 * rhs.m24 + lhs.m33 * rhs.m34 + lhs.m34 * rhs.m44;

    res.m41 = lhs.m41 * rhs.m11 + lhs.m42 * rhs.m21 + lhs.m43 * rhs.m31 + lhs.m44 * rhs.m41;
    res.m42 = lhs.m41 * rhs.m12 + lhs.m42 * rhs.m22 + lhs.m43 * rhs.m32 + lhs.m44 * rhs.m42;
    res.m43 = lhs.m41 * rhs.m13 + lhs.m42 * rhs.m23 + lhs.m43 * rhs.m33 + lhs.m44 * rhs.m43;
    res.m44 = lhs.m41 * rhs.m14 + lhs.m42 * rhs.m24 + lhs.m43 * rhs.m34 + lhs.m44 * rhs.m44;

    return res;
}

struct vec3 mat4_v3mul(struct mat4 m, struct vec3 v)
{
    struct vec3 res;
    res.x = m.m11 * v.x + m.m12 * v.y + m.m13 * v.z + m.m14;
    res.y = m.m21 * v.x + m.m22 * v.y + m.m23 * v.z + m.m24;
    res.z = m.m31 * v.x + m.m32 * v.y + m.m33 * v.z + m.m34;

    return res;
}

struct vec4 mat4_v4mul(struct mat4 m, struct vec4 v)
{
    struct vec4 res;
    res.x = m.m11 * v.x + m.m12 * v.y + m.m13 * v.z + m.m14 * v.w;
    res.y = m.m21 * v.x + m.m22 * v.y + m.m23 * v.z + m.m24 * v.w;
    res.z = m.m31 * v.x + m.m32 * v.y + m.m33 * v.z + m.m34 * v.w;
    res.w = m.m41 * v.x + m.m42 * v.y + m.m43 * v.z + m.m44 * v.w;

    return res;
}

struct mat4 mat4_fmul(struct mat4 m, float f)
{
    struct mat4 res;
    for (size_t i = 0; i < 16; i++)
    {
        res.vals[i] = m.vals[i] / f;
    }

    return res;
}

struct mat4 mat4_scale(struct vec3 s)
{
    struct mat4 res = mat4_zero();
    res.m11 = s.x;
    res.m22 = s.y;
    res.m33 = s.z;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_translate(struct vec3 t)
{
    struct mat4 res = mat4_identity();
    res.m14 = t.x;
    res.m24 = t.y;
    res.m34 = t.z;

    return res;
}

struct mat4 mat4_rotx(float rad)
{
    float c = cosf(rad);
    float s = sinf(rad);

    struct mat4 res = mat4_zero();
    res.m11 = 1.0f;
    res.m22 = c;
    res.m23 = -s;
    res.m32 = s;
    res.m33 = c;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_roty(float rad)
{
    float c = cosf(rad);
    float s = sinf(rad);

    struct mat4 res = mat4_zero();
    res.m11 = c;
    res.m13 = s;
    res.m22 = 1.0f;
    res.m31 = -s;
    res.m33 = c;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_rotz(float rad)
{
    float c = cosf(rad);
    float s = sinf(rad);

    struct mat4 res = mat4_zero();
    res.m11 = c;
    res.m12 = -s;
    res.m21 = s;
    res.m22 = c;
    res.m33 = 1.0f;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_rot(float rad, struct vec3 axis)
{
    float c = cosf(rad);
    float co = 1.0f - c;
    float s = sinf(rad);

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    struct mat4 res = mat4_zero();
    res.m11 = c + x * x * co;
    res.m12 = x * y * co - z * s;
    res.m13 = x * z * co + y * s;

    res.m21 = y * x * co + z * s;
    res.m22 = c + y * y * co;
    res.m23 = y * z * co - x * s;

    res.m31 = z * x * co - y * s;
    res.m32 = z * y * co + x * s;
    res.m33 = c + z * z * co;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far)
{
    struct mat4 res = mat4_zero();
    res.m11 = 2.0f / (right - left);
    res.m14 = -(right + left) / (right - left);
    res.m22 = 2.0f / (top - bottom);
    res.m24 = -(top + bottom) / (top - bottom);
    res.m33 = -2.0f / (far - near);
    res.m34 = -(far + near) / (far - near);
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_perspective(float fov, float aspect, float near, float far)
{
    float t = tanf(fov / 2.0f);

    struct mat4 res = mat4_zero();
    res.m11 = 1.0f / (aspect * t);
    res.m22 = 1.0f / t;
    res.m33 = -(far + near) / (far - near);
    res.m34 = (2.0f * far * near) / (near - far);
    res.m43 = -1.0f;

    return res;
}

struct mat4 mat4_lookat(struct vec3 at, struct vec3 target, struct vec3 up)
{
    struct vec3 zaxis = vec3_normalize(vec3_sub(at, target));
    struct vec3 xaxis = vec3_normalize(vec3_cross(up, zaxis));
    struct vec3 yaxis = vec3_cross(zaxis, xaxis);

    struct mat4 res;
    res.m11 = xaxis.x;
    res.m12 = xaxis.y;
    res.m13 = xaxis.z;
    res.m14 = -vec3_dot(xaxis, at);

    res.m21 = yaxis.x;
    res.m22 = yaxis.y;
    res.m23 = yaxis.z;
    res.m24 = -vec3_dot(yaxis, at);

    res.m31 = zaxis.x;
    res.m32 = zaxis.y;
    res.m33 = zaxis.z;
    res.m34 = -vec3_dot(zaxis, at);

    res.m41 = 0.0f;
    res.m42 = 0.0f;
    res.m43 = 0.0f;
    res.m44 = 1.0f;

    return res;
}

struct mat4 mat4_transpose(struct mat4 m)
{
    struct mat4 res;
    res.m11 = m.m11;
    res.m12 = m.m21;
    res.m13 = m.m31;
    res.m14 = m.m41;
    res.m21 = m.m12;
    res.m22 = m.m22;
    res.m23 = m.m32;
    res.m24 = m.m42;
    res.m31 = m.m13;
    res.m32 = m.m23;
    res.m33 = m.m33;
    res.m34 = m.m43;
    res.m41 = m.m14;
    res.m42 = m.m24;
    res.m43 = m.m34;
    res.m44 = m.m44;

    return res;
}

struct mat4 mat4_remove_translation(struct mat4 m)
{
    m.m14 = 0.0f;
    m.m24 = 0.0f;
    m.m34 = 0.0f;
    m.m41 = 0.0f;
    m.m42 = 0.0f;
    m.m43 = 0.0f;
    m.m44 = 1.0f;

    return m;
}

void vec2_print(struct vec2 v)
{
    printf("(%f, %f)\n", v.x, v.y);
}

void vec3_print(struct vec3 v)
{
    printf("(%f, %f, %f)\n", v.x, v.y, v.z);
}

void vec4_print(struct vec4 v)
{
    printf("(%f, %f, %f, %f)\n", v.x, v.y, v.z, v.w);
}

void mat4_print(struct mat4 m)
{
    printf("[%f, %f, %f, %f\n"
            "%f, %f, %f, %f\n"
            "%f, %f, %f, %f\n"
            "%f, %f, %f, %f]\n",
            m.m11, m.m12, m.m13, m.m14,
            m.m21, m.m22, m.m23, m.m24,
            m.m31, m.m32, m.m33, m.m34,
            m.m41, m.m42, m.m43, m.m44);
}

const struct vec2 VEC2_ZERO =    { 0.0f, 0.0f  };
const struct vec2 VEC2_UP =      { 0.0f, 1.0f  };
const struct vec2 VEC2_DOWN =    { 0.0f, -1.0f };
const struct vec2 VEC2_RIGHT =   { 1.0f, 0.0f  };
const struct vec2 VEC2_LEFT =    { -1.0f, 0.0f };
const struct vec2 VEC2_ONE =     { 1.0f, 1.0f  };

const struct vec3 VEC3_ZERO =    { 0.0f, 0.0f, 0.0f  };
const struct vec3 VEC3_UP =      { 0.0f, 1.0f, 0.0f  };
const struct vec3 VEC3_DOWN =    { 0.0f, -1.0f, 0.0f };
const struct vec3 VEC3_RIGHT =   { 1.0f, 0.0f, 0.0f  };
const struct vec3 VEC3_LEFT =    { -1.0f, 0.0f, 0.0f };
const struct vec3 VEC3_FORWARD = { 0.0f, 0.0f, 1.0f  };
const struct vec3 VEC3_BACK =    { 0.0f, 0.0f, -1.0f };
const struct vec3 VEC3_ONE =     { 1.0f, 1.0f, 1.0f  };
