#include "calc.h"
#include <math.h>

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
