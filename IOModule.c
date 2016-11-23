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

// App includes.
#include "IOModule.h"
#include "boardConfig.h"

// Drivers includes.
#include "adc128d818.h"
#include "pca9500.h"

// Common includes.
#include "USP10973.h"
#include "Conversion.h"
#include "utils.h"
#include "error.h"

//#define mIOModuleValidateArg(expression) ((expression) ? (void)0 : IOModuleSetMasterStatusFlag(kMasterStatusFlags_InvalidArg))
#define mIOModuleValidateArg(arg) if (arg) { (void)0; } else { IOModuleSetMasterStatusFlag(kMasterStatusFlags_InvalidArg); return -1; }
#define mIOModuleValidateStatus(returnStatus) if (returnStatus != kSuccess) { return returnStatus; }

// --------------------------------------------------------------------------------------------------------------
