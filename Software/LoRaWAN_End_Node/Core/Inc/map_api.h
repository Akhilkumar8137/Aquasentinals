/*
 * custom_map.h
 *
 *  Created on: Mar 3, 2025
 *      Author: zero
 */

#ifndef APPLICATION_AQUA_GUAGE_INC_MAP_API_H_
#define APPLICATION_AQUA_GUAGE_INC_MAP_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @file map_api.h
 * @brief API for mapping a value from one range to another supporting multiple data types.
 *
 * This module provides generic mapping functions to convert a value from an input range
 * [in_min, in_max] to an output range [out_min, out_max]. It supports various data types such as
 * int, int8_t, uint8_t, int32_t, uint32_t, float, and double. For signed integers, proper rounding is applied,
 * while the unsigned version uses simpler arithmetic.
 *
 */

int32_t map_int32(int32_t x, int32_t in_min, int32_t in_max,
                  int32_t out_min, int32_t out_max);

uint32_t map_uint32(uint32_t x, uint32_t in_min, uint32_t in_max,
                    uint32_t out_min, uint32_t out_max);

float map_float(float x, float in_min, float in_max,
                float out_min, float out_max);

double map_double(double x, double in_min, double in_max,
                  double out_min, double out_max);

/**
 * @brief Generic mapping macro that selects the appropriate function based on the type of the input value.
 *
 * This macro uses C11's _Generic to dispatch the call to the correct mapping function.
 *
 * Supported types include:
 * - float: Uses map_float
 * - double: Uses map_double
 * - unsigned char (e.g., uint8_t), unsigned short (e.g., uint16_t),
 *   unsigned int, unsigned long: Uses map_uint32
 * - signed char (e.g., int8_t), char, short (e.g., int16_t), int, long: Uses map_int32
 *
 * @param x       The input value.
 * @param in_min  The minimum of the input range.
 * @param in_max  The maximum of the input range.
 * @param out_min The minimum of the output range.
 * @param out_max The maximum of the output range.
 *
 * @return The value mapped to the output range.
 */
#define map(x, in_min, in_max, out_min, out_max)                        \
    _Generic((x),                    									\
        float: map_float,            									\
        double: map_double,          									\
        unsigned char: map_uint32,   									\
        unsigned short: map_uint32,  									\
        unsigned int: map_uint32,    									\
        unsigned long: map_uint32,   									\
        signed char: map_int32,      									\
        char: map_int32,             									\
        short: map_int32,            									\
        int: map_int32,              									\
        long: map_int32,             									\
        default: map_int32           									\
    )(x, in_min, in_max, out_min, out_max)

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_AQUA_GUAGE_INC_MAP_API_H_ */
