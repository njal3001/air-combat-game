#include "spatial.h"
#include "string.h"
#include <math.h>

struct vec3 create_vec3(float x, float y, float z)
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
    if (length == 0) return vec3_zero();

    return vec3_div(v, length);
}

struct mat4 create_mat4(
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
