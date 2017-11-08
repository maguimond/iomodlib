/* Copyright (C) 2016, Marc-Andre Guimond <guimond.marcandre@gmail.com>.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file is encoded in UTF-8.
 */

#ifndef ADC128D818_H_
#define ADC128D818_H_

// Standard includes.
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ----------------------------------------------------------------------------
// Constants
// ----------------------------------------------------------------------------
// Slave addresses.
typedef enum
{
    kADC128D818_MaxAddresses = 9,
    kADC128D818_SlaveAddress1 = 0x1D,
    kADC128D818_SlaveAddress2,
    kADC128D818_SlaveAddress3,
    kADC128D818_SlaveAddress4 = 0x2D,
    kADC128D818_SlaveAddress5,
    kADC128D818_SlaveAddress6,
    kADC128D818_SlaveAddress7 = 0x35,
    kADC128D818_SlaveAddress8,
    kADC128D818_SlaveAddress9,
} ADC128D818_SlaveAddress_t;

typedef enum
{
    kADC128D818_IN0,
    kADC128D818_IN1,
    kADC128D818_IN2,
    kADC128D818_IN3,
    kADC128D818_IN4,
    kADC128D818_IN5,
    kADC128D818_IN6,
    kADC128D818_IN7,
    kADC128D818_MaxChannels,
} ADC128D818_Channel_t;

// ------------------
// Registers mapping.
// ------------------
// Configuration register. Provides control and configuration 8-bit, R/W, 8-bit.
#define kADC128D818_RegisterConfiguration 0x00
typedef enum
{
    // 0: ADC128D818 in shutdown mode, 1: Enable startup of monitoring operations.
    kADC128D818_RegisterConfiguration_Start = (1 << 0),
    // 1: Enable the interrupt output pin, INT.
    kADC128D818_RegisterConfiguration_INT_Enable = (1 << 1),
    // 1: Clear the interrupt output pin, INT, without affecting the contents of Interrupt Status Registers. When this bit is set high, the device stops the round-robin monitoring loop.
    kADC128D818_RegisterConfiguration_INT_Clear = (1 << 3),
    // 1: Restore default values to the following registers: Configuration, Interrupt Status, Interrupt Mask, Conversion Rate, Channel Disable, One-Shot, Deep Shutdown, Advanced Configuration, Busy Status, Channel Readings, Limit, Manufacturer ID, Revision ID. This bit clears itself.
    kADC128D818_RegisterConfiguration_Init = (1 << 7),
} ADC128D818_RegisterConfiguration_t;

// Interrupt status register. Provides status of each WATCHDOG limit or interrupt event, R, 8-bit.
#define kADC128D818_RegisterInterruptStatus 0x01

// Interrupt mask register. Masks the interrupt status from propagating to INT, R/W, 8-bit.
#define kADC128D818_RegisterInterruptMask 0x03
typedef enum
{
    // 1: Mask the corresponding interrupt status from propagating to the interrupt output pin, INT.
    kADC128D818_RegisterInterruptMask_None = 0x00,
    kADC128D818_RegisterInterruptMask_IN0 = (1 << 0),
    kADC128D818_RegisterInterruptMask_IN1 = (1 << 1),
    kADC128D818_RegisterInterruptMask_IN2 = (1 << 2),
    kADC128D818_RegisterInterruptMask_IN3 = (1 << 3),
    kADC128D818_RegisterInterruptMask_IN4 = (1 << 4),
    kADC128D818_RegisterInterruptMask_IN5 = (1 << 5),
    kADC128D818_RegisterInterruptMask_IN6 = (1 << 6),
    kADC128D818_RegisterInterruptMask_IN7 = (1 << 7),
    kADC128D818_RegisterInterruptMask_All = 0xFF,
} ADC128D818_RegisterInterruptMask_t;

// Conversion rate register. Controls the conversion rate, R/W, 8-bit.
#define kADC128D818_RegisterConversionRate 0x07
typedef enum
{
    // Controls the conversion rate: 0: Low Power Conversion Mode 1: Continuous Conversion Mode.
    // Note: This register must only be programmed when the device is in shutdown mode, that is, when the 'START' bit of the 'Configuration Register' (address 00h) = 0.
    kADC128D818_ConversionRate_LowPower,
    kADC128D818_ConversionRate_Continuous,
} ADC128D818_ConversionRate_t;

// Channel disable register. Disables conversion for each voltage or temperature channel, R/W, 8-bit.
#define kADC128D818_RegisterChannelDisable 0x08
typedef enum
{
    // 1: Conversions are skipped and disabled, value register reading will be 0, and error events will be suppressed.
    kADC128D818_RegisterChannelDisable_None = 0x00,
    kADC128D818_RegisterChannelDisable_IN0 = (1 << 0),
    kADC128D818_RegisterChannelDisable_IN1 = (1 << 1),
    kADC128D818_RegisterChannelDisable_IN2 = (1 << 2),
    kADC128D818_RegisterChannelDisable_IN3 = (1 << 3),
    kADC128D818_RegisterChannelDisable_IN4 = (1 << 4),
    kADC128D818_RegisterChannelDisable_IN5 = (1 << 5),
    kADC128D818_RegisterChannelDisable_IN6 = (1 << 6),
    kADC128D818_RegisterChannelDisable_IN7 = (1 << 7),
    kADC128D818_RegisterChannelDisable_All = 0xFF,
} ADC128D818_RegisterChannelDisable_t;

// One shot register. Initiates a single conversion of all enabled channels, W, 8-bit.
#define kADC128D818_RegisterOneShot 0x09
typedef enum
{
    // 1: Initiate a single conversion and comparison cycle when the device is in shutdown mode or deep shutdown mode, after which the device returns to the respective mode that it was in.
    kADC128D818_RegisterOneShot_OneShot = (1 << 0),
} ADC128D818_RegisterOneShot_t;

// Deep shutdown register. Enables deep shutdown mode, R/W, 8-bit.
#define kADC128D818_RegisterDeepShutdown 0x0A
typedef enum
{
    // 1: When 'START' = 0 (address 00h, bit 0), setting this bit high will place the device in deep shutdown mode.
    kADC128D818_RegisterDeepShutdown_DeepShutdownDisable = 0x00,
    kADC128D818_RegisterDeepShutdown_DeepShutdownEnable = 0x01,
} ADC128D818_RegisterDeepShutdown_t;

// Advanced configuration register. Selects internal or external VREF and modes of operation, R/W, 8-bit.
#define kADC128D818_RegisterAdvancedConfiguration 0x0B
typedef enum
{
    // 0: Selects the 2.56V internal VREF 1: Selects the variable external VREF.
    kADC128D818_RegisterAdvancedConfiguration_ExternalReferenceEnable = (1 << 0),
    // Mode = ModeSelect[1..0].
    kADC128D818_RegisterAdvancedConfiguration_ModeSelect0 = (1 << 1),
    // Mode 0 = IN7 is temperature, mode 1 = single-ended, mode 2 = pseudo-differential, mode 3 = half single-ended, half pseudo-differential.
    kADC128D818_RegisterAdvancedConfiguration_ModeSelect1 = (1 << 2),
} ADC128D818_RegisterAdvancedConfiguration_t;
typedef enum
{
    // Modes constants.
    kADC128D818_Mode_Temp,
    kADC128D818_Mode_SingleEnded,
    kADC128D818_Mode_PseudoDiff,
    kADC128D818_Mode_HalfPseudiDiff,
} ADC128D818_Mode_t;

// Busy status register. Reflects ADC128D818 'Busy' and 'Not Ready' statuses, R, 8-bit.
#define kADC128D818_RegisterBusyStatus 0x0C
typedef enum
{
    // 1: ADC128D818 is converting.
    kADC128D818_RegisterBusyStatus_Busy = (1 << 0),
    // 1: Waiting for the power-up sequence to end.
    kADC128D818_RegisterBusyStatus_NotReady = (1 << 1),
} ADC128D818_RegisterBusyStatus_t;

// Channel Readings Registers. Report the channels (voltage or temperature) readings, R, 16-bit.
#define kADC128D818_RegisterChannel0Read 0x20
#define kADC128D818_RegisterChannel1Read 0x21
#define kADC128D818_RegisterChannel2Read 0x22
#define kADC128D818_RegisterChannel3Read 0x23
#define kADC128D818_RegisterChannel4Read 0x24
#define kADC128D818_RegisterChannel5Read 0x25
#define kADC128D818_RegisterChannel6Read 0x26
#define kADC128D818_RegisterChannel7Read 0x27

// Limit registers. Set the limits for the voltage and temperature channels, R/W, 8-bit.
#define kADC128D818_RegisterChannel0LimitHigh 0x2A
#define kADC128D818_RegisterChannel0LimitLow 0x2B
#define kADC128D818_RegisterChannel1LimitHigh 0x2C
#define kADC128D818_RegisterChannel1LimitLow 0x2D
#define kADC128D818_RegisterChannel2LimitHigh 0x2E
#define kADC128D818_RegisterChannel2LimitLow 0x2F
#define kADC128D818_RegisterChannel3LimitHigh 0x30
#define kADC128D818_RegisterChannel3LimitLow 0x31
#define kADC128D818_RegisterChannel4LimitHigh 0x32
#define kADC128D818_RegisterChannel4LimitLow 0x33
#define kADC128D818_RegisterChannel5LimitHigh 0x34
#define kADC128D818_RegisterChannel5LimitLow 0x35
#define kADC128D818_RegisterChannel6LimitHigh 0x36
#define kADC128D818_RegisterChannel6LimitLow 0x37
#define kADC128D818_RegisterChannel7LimitHigh 0x38
#define kADC128D818_RegisterChannel7LimitLow 0x39

// Manufacturer ID register. Reports the manufacturer's ID, R, 8-bit.
#define kADC128D818_RegisterManufacturerID 0x3E

// Revision ID register. Reports the revision's ID, R, 8-bit.
#define kADC128D818_RegisterRevisionID 0x3F

// ----------------------------------------------------------------------------
// Macros
// ----------------------------------------------------------------------------
// For asserts.
#define mADC128D818IsMode(modeSelect) ((modeSelect == kADC128D818_Mode_Temp) || (modeSelect == kADC128D818_Mode_SingleEnded) || (modeSelect == kADC128D818_Mode_PseudoDiff) || (modeSelect == kADC128D818_Mode_HalfPseudiDiff))
#define mADC128D818IsDeepShutdown(deepShutdown) ((deepShutdown == kADC128D818_RegisterDeepShutdown_DeepShutdownDisable) || (deepShutdown == kADC128D818_RegisterDeepShutdown_DeepShutdownEnable))
#define mADC128D818IsChannelReadings(channel) ((channel == kADC128D818_RegisterChannel0Read) || (channel == kADC128D818_RegisterChannel1Read) || (channel == kADC128D818_RegisterChannel2Read) || (channel == kADC128D818_RegisterChannel3Read) || (channel == kADC128D818_RegisterChannel4Read) || (channel == kADC128D818_RegisterChannel5Read) || (channel == kADC128D818_RegisterChannel6Read) || (channel == kADC128D818_RegisterChannel7Read))
#define mADC128D818IsConversionRate(mode) ((mode == kADC128D818_ConversionRate_Continuous) || (mode == kADC128D818_ConversionRate_LowPower))

// ----------------------------------------------------------------------------
// Data types
// ----------------------------------------------------------------------------
typedef struct
{
    uint8_t manufacturerID;
    uint8_t revisionID;
    uint16_t channel0data;
    uint16_t channel1data;
    uint16_t channel2data;
    uint16_t channel3data;
    uint16_t channel4data;
    uint16_t channel5data;
    uint16_t channel6data;
    uint16_t channel7data;
} ADC128D818_t;

// ----------------------------------------------------------------------------
// Function prototypes
// ----------------------------------------------------------------------------
///
int ADC128D818Init(uint8_t inADCAddress);
///
int ADC128D818StartConversion(uint8_t inADCAddress, uint8_t inMode);
///
int ADC128D818StopConversion(uint8_t inADCAddress);
///
int ADC128D818SingleConversion(uint8_t inADCAddress);
///
int ADC128D818DeepShutdown(uint8_t inADCAddress, uint8_t inShutdownMode);
///
int ADC128D818ReadChannel(uint8_t inADCAddress, uint8_t inChannel, uint16_t* outADCData);

#ifdef __cplusplus
}
#endif

#endif // ADC128D818_H_
