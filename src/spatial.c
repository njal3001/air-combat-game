#include "spatial.h"
#include "string.h"
#include <math.h>
#include <stdio.h>

struct vec3 vec3_create(float x, float y, float z)
{
    struct vec3 res;
    res.x = x;
    res.y = y;
    res.z = z;
    return res;
}

struct vec3 vec3_zero()
{
    struct vec3 res;
    memset(&res, 0, sizeof(struct vec3));
    return res;
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

float vec3_dot(struct vec3 v1, struct vec3 v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

struct vec3 vec3_cross(struct vec3 v1, struct vec3 v2)
{
    struct vec3 res;
    res.x = v1.y * v2.z - v1.z * v2.y;
    res.y = v1.z * v2.z - v1.x * v2.z;
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

struct vec3 mat4_vmul(struct mat4 m, struct vec3 v)
{
    struct vec3 res;
    res.x = m.m11 * v.x + m.m12 * v.y + m.m13 * v.z + m.m14;
    res.y = m.m21 * v.x + m.m22 * v.y + m.m23 * v.z + m.m24;
    res.z = m.m31 * v.x + m.m32 * v.y + m.m33 * v.z + m.m34;

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

void vec3_print(struct vec3 v)
{
    printf("(%f, %f, %f)\n", v.x, v.y, v.z);
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
