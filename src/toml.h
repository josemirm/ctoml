// José Miguel Rodríguez Marchena (josemirm)

#ifndef __JOSEMIRM_CTOML__
#define __JOSEMIRM_CTOML__

#include <assert.h>
#include <ctype.h> // for isspace()
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define printDebug(str) (fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, str))


// Default string manipulation buffer
#ifndef TOML_BUFFER_SIZE
#define TOML_BUFFER_SIZE 64
#endif

enum TOMLError {
	TOMLNoError = 0,
	TOMLKeyNotFound = -1,
	TOMLFormatErr = -2,
	TOMLBufferErr = -3,
	TOMLUnsuportedErr = -4,
	TOMLInputError = -5,
	TOMLOutputBufferTooSmall = -6,
	TOMLOtherErr = -100
};

typedef struct TOML_t {
	const char* str;
	int pos;
	int len;
	enum TOMLError lastError;
	char buffer[TOML_BUFFER_SIZE];
} TOML;

TOML initTOML(char const* str);

int getTOMLbool(TOML* t, char const* key, bool* value);
int getTOMLstr(TOML* t, char const* key, char* value, const int maxLen);
int getTOMLint(TOML* t, char const* key, int* value);
int getTOMLdouble(TOML* t, char const* key, double* value);

//int getTOMLDate(TOML* t, char const* key, bool* value);
//int getTOMLTime(TOML* t, char const* key, bool* value);


#endif
