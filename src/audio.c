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

void audio_play(const char *name)
{
    const char *path = get_asset_path(ASSET_OTHER, name);
    ma_result res = ma_engine_play_sound(&engine, path, NULL);
    if (res != MA_SUCCESS)
    {
        log_warn("Could not play audio %s", path);
    }
}
