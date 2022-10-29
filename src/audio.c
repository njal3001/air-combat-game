#include "audio.h"
#include "assets.h"
#include "log.h"

#define MINIAUDIO_IMPLEMENTATION
#include "third_party/miniaudio.h"

ma_engine engine;

bool audio_init()
{
    ma_result res = ma_engine_init(NULL, &engine);
    if (res != MA_SUCCESS)
    {
        return false;
    }

    return true;
}

void audio_shutdown()
{
    ma_engine_uninit(&engine);
}

void audio_play(const char *name)
{
    const char *path = get_asset_path(ASSET_OTHER, name);
    ma_result res = ma_engine_play_sound(&engine, path, NULL);
    if (res != MA_SUCCESS)
    {
        log_warn("Could not play audio %s", path);
    }
}

void audio_set_volume(float val)
{
    ma_engine_set_volume(&engine, val);
}

void audio_mute()
{
    audio_set_volume(0.0f);
}
