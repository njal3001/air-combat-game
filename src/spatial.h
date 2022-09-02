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
    };
};

struct vec3 create_vec3(float x, float y, float z);
struct vec3 vec3_zero();

struct vec3 vec3_add(struct vec3 lhs, struct vec3 rhs);
struct vec3 vec3_sub(struct vec3 lhs, struct vec3 rhs);
struct vec3 vec3_mul(struct vec3 v, float rhs);
struct vec3 vec3_div(struct vec3 v, float rhs);
float vec3_dot(struct vec3 v1, struct vec3 v2);
float vec3_length(struct vec3 v);
float vec3_length2(struct vec3 v);
struct vec3 vec3_normalize(struct vec3 v);

struct mat4 create_mat4(
        float m11, float m12, float m13, float m14,
        float m21, float m22, float m23, float m24,
        float m31, float m32, float m33, float m34,
        float m41, float m42, float m43, float m44);
struct mat4 mat4_zero();
struct mat4 mat4_identity();
