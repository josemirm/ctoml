/**
 * @file strings.h
 * @brief Integer and double numbers extraction
 *
 */

// José Miguel Rodríguez Marchena (josemirm)

#ifndef __JOSEMIRM_CTOML_NUMERICALS__
#define __JOSEMIRM_CTOML_NUMERICALS__

#include "toml.h"
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

int extractIntFromValue(TOML* t, TOMLInt_t* returnValue);
int extractDoubleFromValue(TOML* t, double* returnValue);

#endif