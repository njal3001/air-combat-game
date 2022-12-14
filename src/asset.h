#pragma once
#include "shader.h"
#include "texture.h"
#include "mesh.h"
#include "font.h"

enum asset_type
{
    ASSET_TYPE_SHADER,
    ASSET_TYPE_TEXTURE,
    ASSET_TYPE_MESH,
    ASSET_TYPE_FONT,
    ASSET_TYPE_AUDIO,
};

enum asset_texture
{
    ASSET_TEXTURE_METAL,
    ASSET_TEXTURE_WALL,
    ASSET_TEXTURE_END,
};

enum asset_mesh
{
    ASSET_MESH_PLAYER,
    ASSET_MESH_ORB,
    ASSET_MESH_WALL,
    ASSET_MESH_END,
};

enum asset_shader
{
    ASSET_SHADER_MESH,
    ASSET_SHADER_UI,
    ASSET_SHADER_UNTEXTURED,
    ASSET_SHADER_END,
};

enum asset_font
{
    ASSET_FONT_VCR,
    ASSET_FONT_END,
};

enum asset_audio
{
    ASSET_AUDIO_SONG,
    ASSET_AUDIO_END,
};

void assets_init();
void assets_free();

struct texture *get_texture(enum asset_texture handle);
struct mesh *get_mesh(enum asset_mesh handle);
struct shader *get_shader(enum asset_shader handle);
struct font *get_font(enum asset_font handle);
const char *get_audio_path(enum asset_audio handle);
