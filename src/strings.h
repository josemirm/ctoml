/**
 * @file strings.h
 * @brief Strings extraction
 *
 */

// José Miguel Rodríguez Marchena (josemirm)


#ifndef __JOSEMIRM_CTOML_STRINGS__
#define __JOSEMIRM_CTOML_STRINGS__

#include "toml.h"
#include "utils.h"

int writeUnicodeCharsFromHex(char const* input, char* outBuffer);
int processEscapedChars(char const* input, char* outBuffer, int* copiedPtr, const int maxLen);
int processEscapedString(char const* input, char const* strEnd, const int inputLen, char* output,
	const unsigned int maxOutLen, bool longLine);
char* getEndEscpdSimpleString(char const* str);
int extractStrFromValue(TOML* t, char* returnValue, const int maxLen);

#endif
