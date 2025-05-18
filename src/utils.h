/**
 * @file utils.h
 * @brief Utility functions used across multiple source files
 * 
 */

// José Miguel Rodríguez Marchena (josemirm)

#ifndef __JOSEMIRM_CTOML_UTILITY__
#define __JOSEMIRM_CTOML_UTILITY__ 

#include "toml.h"

int skipSpaces(TOML* t);
int skipComments(TOML* t);
int checkValidTOMLStructure(TOML* t);

#endif
