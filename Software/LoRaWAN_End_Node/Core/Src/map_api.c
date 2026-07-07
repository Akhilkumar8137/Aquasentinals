/*
 * map_api.c
 *
 *  Created on: Mar 3, 2025
 *      Author: zero
 */

#include "map_api.h"
#include <math.h>

#define EPSILON_FLOAT 1e-6f
#define EPSILON_DOUBLE 1e-12

/* Mapping function for signed integers (using int32_t arithmetic). */
int32_t map_int32(int32_t x, int32_t in_min, int32_t in_max,
                  int32_t out_min, int32_t out_max)
{
    int32_t in_range = in_max - in_min;
    int32_t out_range = out_max - out_min;
    if (in_range == 0)
        return (out_min + out_range / 2);
    int32_t num = (x - in_min) * out_range;
    if (out_range >= 0) {
        num += in_range / 2;
    } else {
        num -= in_range / 2;
    }
    int32_t result = num / in_range + out_min;
    if (out_range >= 0) {
        if (in_range * num < 0)
            return (result - 1);
    } else {
        if (in_range * num >= 0)
            return (result + 1);
    }
    return (result);
}

/* Mapping function for unsigned integers (using uint32_t arithmetic). */
uint32_t map_uint32(uint32_t x, uint32_t in_min, uint32_t in_max,
                    uint32_t out_min, uint32_t out_max)
{
    uint32_t in_range = in_max - in_min;
    uint32_t out_range = out_max - out_min;
    if (in_range == 0)
        return (out_min + out_range / 2);
    uint32_t num = (x - in_min) * out_range;
    num += in_range / 2;  // rounding adjustment for unsigned arithmetic
    return (num / in_range + out_min);
}

/* Mapping function for float values. */
float map_float(float x, float in_min, float in_max,
                float out_min, float out_max)
{
    float in_range = in_max - in_min;
    float out_range = out_max - out_min;
    if (fabsf(in_range) < EPSILON_FLOAT)
        return (out_min + out_range / 2.0f);
    return (((x - in_min) * out_range) / in_range + out_min);
}

/* Mapping function for double values. */
double map_double(double x, double in_min, double in_max,
                  double out_min, double out_max)
{
    double in_range = in_max - in_min;
    double out_range = out_max - out_min;
    if (fabs(in_range) < EPSILON_DOUBLE)
        return (out_min + out_range / 2.0);
    return (((x - in_min) * out_range) / in_range + out_min);
}
