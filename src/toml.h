/**
 * @file toml.h
 * @brief Main header with the structures and functions to use library
 * 
 */


// José Miguel Rodríguez Marchena (josemirm)

#ifndef __JOSEMIRM_CTOML__
#define __JOSEMIRM_CTOML__

#include <assert.h>
#include <ctype.h> // for isspace()
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define printDebug(str) (fprintf(stderr, "%s:%d - %s\n", __FILE__, __LINE__, str))


/**
* @brief Default string manipulation buffer size.
* 
* There will be a statically allocated internal buffer for all string
* manipulation, basically a char array, and it will be of this size.
* 
* To change that size either change this value or declare it in the code that
* uses this library before including this header.
*/
#ifndef TOML_BUFFER_SIZE
#define TOML_BUFFER_SIZE 64
#endif

/**
* @brief Enumeration of errors that could appears in runtime.
*/
enum TOMLError {
	TOMLNoError = 0,
	TOMLKeyNotFound = -1,
	TOMLInvalidValue = -2,
	TOMLFormatErr = -3,
	TOMLBufferErr = -4,
	TOMLUnsuportedErr = -5,
	TOMLInputError = -6,
	TOMLOutputBufferTooSmall = -7,
	TOMLOtherErr = -100
};

/**
* @struct TOML
* @brief Main structure of the library
* 
* Allocate this structure using the initTOML() function. It isn't needed to use
* or change any of its internal while using the library, only to pass it to the
* functions that requires it.
*
* This contains an existing, previously allocated, string with all the
* information required to process it with the rest of the functions of the
* library.
* 
* @var TOML::str
* String to process, allocated externally.
* 
* @var TOML::pos
* Position of the string where the next executed library function will start to
* process.
* 
* @var TOML::len
* Total length of the string.
* 
* @var TOML::lastError
* Last error found while executing the last function.
* 
* @var TOML::buffer
* Buffer where all the strings manipulations will be done.
*/
typedef struct TOML_t {
	const char* str;
	int pos;
	int len;
	enum TOMLError lastError;
	char buffer[TOML_BUFFER_SIZE];
} TOML;

/**
* @brief Initialize a TOML structure with a given string.
* 
* Fills a TOML structure with all the data from an existing string, previously
* allocated, to being able to process it with the rest of the function and
* returns it. That string will need to exist and not be change in all the
* execution of function on the created TOML function, otherwise erratic and
* undefined behavious will appear.
*
* It doesn't allocate dinamically any memory, so no clean-up or
* memory freeing function is needed after running this.
* 
* The correct way to use this function is this way:
* @code{.c}
* TOML toml = initTOML(someString);
* @endcode
*/
TOML initTOML(char const* str);

int getTOMLbool(TOML* t, char const* key, bool* value);
int getTOMLstr(TOML* t, char const* key, char* value, const int maxLen);
int getTOMLint(TOML* t, char const* key, int* value);
int getTOMLdouble(TOML* t, char const* key, double* value);

//int getTOMLDate(TOML* t, char const* key, bool* value);
//int getTOMLTime(TOML* t, char const* key, bool* value);


#endif
