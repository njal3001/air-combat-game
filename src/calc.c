#include "calc.h"
#include <math.h>
#include <stdlib.h>

float fclamp(float min, float val, float max)
{
    return fmin(fmax(min, val), max);
}

float approach(float val, float target, float amount)
{
    if (val > target)
    {
        return fmax(val - amount, target);
    }

    return fmin(val + amount, target);
}

int randrange(int min, int max)
{
    return rand() % (max - min) + min;
}

float frand()
{
    return rand() / (float)RAND_MAX;
}

float frandrange(float min, float max)
{
    return frand() * (max - min) + min;
}

int sign(int val)
{
    return val < 0 ? -1 : 1;
}

int min(int a, int b)
{
    return a < b ? a : b;
}
