// José Miguel Rodríguez Marchena (josemirm)

#include "toml.h"
#include "numericals.h"
#include "proc_stments.h"
#include "strings.h"
#include "booleans.h"


//
// -- Auxiliar functions
//

inline int checkValidTOMLStructure(TOML* t) {
	// Everytime a public TOML function get called, do this check
	assert(t != NULL && t->str != NULL);
	if ((t != NULL) && (t->str != NULL) && (strlen(t->str) > 0)) return 0;
	return -1;
}

inline int preliminaryProcess(TOML* t, char const* key, void* value) {
	assert(t != NULL);
	assert(key != NULL);
	assert(value != NULL);

	if ((key == NULL) || (value == NULL) || checkValidTOMLStructure(t)) return -1;

	return findKeyValPos(t, key);
}


//
// -- Public functions
//

TOML initTOML(char const* str) {
	assert(str != NULL);

	TOML ret;
	if (str == NULL) {
		ret.str = NULL;
		ret.lastError = TOMLInputError;
		ret.pos = -1;
		ret.len = -1;
	};

	ret.str = str;
	ret.len = (int)strlen(str);
	ret.pos = 0;
	ret.lastError = TOMLNoError;
	ret.buffer[0] = '\0';

	return ret;
}


int getTOMLbool(TOML* t, char const* key, bool* returnValue) {
	int keyPos = preliminaryProcess(t, key, (void*) returnValue);
	if (keyPos < 0) return keyPos;

	int comparison = compareStrBool(&(t->str[keyPos]));

	if (comparison == 0) {
		*returnValue = false;
		return 0;
	} else if (comparison == 1) {
		*returnValue = true;
		return 1;
	} else {
		t->lastError = comparison;
		return comparison;
	}
}


int getTOMLstr(TOML* t, char const* key, char* returnValue, const int maxLen) {
	int keyPos = preliminaryProcess(t, key, (void*) returnValue);
	if (keyPos < 0) return keyPos;

	int ret = extractStrFromValue(t, returnValue, maxLen);
	if (ret < 0) t->lastError = ret;

	return ret;
}


int getTOMLint(TOML* t, char const* key, TOMLInt_t* returnValue) {
	int keyPos = preliminaryProcess(t, key, (void*) returnValue);
	if (keyPos < 0) return keyPos;

	int ret = extractIntFromValue(t, returnValue);
	if (ret < 0) return ret;

	return 0;
}


int getTOMLdouble(TOML* t, char const* key, double* returnValue) {
	int keyPos = preliminaryProcess(t, key, (void*) returnValue);
	if (keyPos < 0) return keyPos;

	int ret = extractDoubleFromValue(t, returnValue);
	if (ret < 0) return ret;

	return 0;
}
