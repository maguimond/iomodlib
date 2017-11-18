// ----------------------------------------------------------------------------
// usp10973.c
//
// Copyright (C) 2016 GRR Systems <marc-andre.guimond@grr-systems.com>.
// All rights reserved.
//
// This file is encoded in UTF-8.
// ---------------------------------------------------------------------------
 
// Standard includes.
#include "math.h"

// Common includes.
#include "usp10973.h"
#include "usp10973config.h"

#ifndef kRREF
#error "RREF not defined (ex: #define kRREF 10000)."
#endif

#ifndef kADCResolution
#error "ADC resolution not defined (ex: #define kADCResolution 12)."
#endif

// kADCMaxValue *= RREF to simplify computation.
#define kADCMaxValue ((1 << kADCResolution) - 1) * kRREF

// ----------------------------------------------------------------------------
int USP10973BetaComputeTemperature(uint16_t inRawADCValue, int32_t* outTemperature)
{
    int status = 0;

    // Get resistance value of the thermistor (valueR2 = ((RREF * VREF) / VADC) - RREF).
    // FIXME: Linux platform (uint32_t)kADCMaxValue does not work?
    uint32_t valueR2 = (((double)kADCMaxValue / (uint32_t)inRawADCValue) - (uint32_t)kRREF);

    double beta = 0;
    double valueR1 = 0;
    double valueT1 = 0;

    // Apply coefficients according to the resistance range.
    if ((valueR2 <= kR1M40_0) && (valueR2 > kR10_50))
    {
        // Beta for [-40 0]C.
        beta = (double)kBetaM40_0;
        valueT1 = (double)kT1M40_0;
        valueR1 = (double)kR1M40_0;
    }
    else if ((valueR2 <= kR10_50) && (valueR2 > kR150_100))
    {
        // Beta for [0 50]C.
        beta = (double)kBeta0_50;
        valueT1 = (double)kT10_50;
        valueR1 = (double)kR10_50;
    }
    else if ((valueR2 <= kR150_100) && (valueR2 >= kR1Min))
    {
        // Beta for [50 100]C.
        beta = (double)kBeta50_100;
        valueT1 = (double)kT150_100;
        valueR1 = (double)kR150_100;
    }
    else
    {
        // Resistance out of range, invalid for conversion.
        return status = -1;
    }

    double lnResult = log((double)valueR2 / valueR1);
    // Simplified Steinhart-Hart equation (for Beta parameter) - result on 3 decimal points.
    *outTemperature = (int32_t)(((beta * valueT1) / (beta + lnResult * valueT1) - (double)kKelvinConstant) * (double)1000);

    // Value is in range, success.
    return status;
}
