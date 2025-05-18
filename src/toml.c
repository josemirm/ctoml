// José Miguel Rodríguez Marchena (josemirm)

#include "toml.h"
#include "proc_stments.h"
#include "strings.h"
#include "booleans.h"

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


int getTOMLbool(TOML* t, char const* key, bool* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOMLStructure(t)) return -1;

	int keyPos = findKeyValPos(t, key);
	if (keyPos < 0) return keyPos;

	int comparison = compareStrBool(&(t->str[keyPos]));

	if (comparison == 0) {
		if (value != NULL) *value = false;
		return 0;
	} else if (comparison == 1) {
		if (value != NULL) *value = true;
		return 1;
	}

	return -1;
}


int getTOMLstr(TOML* t, char const* key, char* value, const int maxLen) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOMLStructure(t)) return -1;

	int keyPos = findKeyValPos(t, key);
	if (keyPos < 0) return keyPos;

	int ret = extractStrFromValue(t, value, maxLen);
	if (ret < 0) t->lastError = ret;

	return ret;
}


int getTOMLint(TOML* t, char const* key, int* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOMLStructure(t)) return -1;

	return -1;
}


int getTOMLdouble(TOML* t, char const* key, double* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOMLStructure(t)) return -1;

	return -1;
}
