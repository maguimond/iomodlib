// ----------------------------------------------------------------------------
// usp10973.h
//
// Copyright (C) 2016 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------

#ifndef USP10973_H_
#define USP10973_H_

// Standard includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// Kelvin constant for conversion in Celsius.
#define kKelvinConstant 273.15

// Beta coefficients
// ---------------------------
// Coefficients for temperature range [-40, 0]C.
#define kBeta_m40_0 3713.9
#define kT1_m40_0 (-40 + kKelvinConstant)
#define kR1_m40_0 336473.5
// Coefficients for temperature range [0, 50]C.
#define kBeta_0_50 3891.5
#define kT1_0_50 (0 + kKelvinConstant)
#define kR1_0_50 32650
// Coefficients coefficient for temperature range [50, 100]C.
#define kBeta_50_100 4025.59
#define kT1_50_100 (50 + kKelvinConstant)
#define kR1_50_100 3602
// Coefficients coefficient for temperature range [50, 100]C.
// #define kBeta_100_150
#define kT1_100_150 (100 + kKelvinConstant)
#define kR1_100_150 678.60
// R min / max.
#define kR1_150_200 183.486


// Steinhart-Hart coefficients
// ---------------------------

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int USP10973BetaComputeTemperature(uint16_t inRawADCValue, int32_t* outTemperature);

#ifdef __cplusplus
}
#endif

#endif // USP10973_H_
