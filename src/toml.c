// José Miguel Rodríguez Marchena (josemirm)

#include "toml.h"

//
// -- Helper functions
//


static inline int skipSpaces(TOML* t) {
	while (isspace(t->str[t->lastPos] && t->lastPos < t->len)) {
		t->lastPos++;
	}

	if (t->lastPos == t->len) return 1;

	return 0;
}


static inline int skipComments(TOML * t) {
	if (t->str[t->lastPos] == '#') {
		char* endComment = strchr(&(t->str[t->lastPos]), '\n');
		if (!endComment || *endComment == '\0') {
			t->lastPos = t->len;
			return 1;
		}

		int newPos = (int)(endComment - t->str);
		t->lastPos = newPos;
	}

	return 0;
}


static int processPossibleTable(TOML* t) {
	// If a table is found
	if (t->str[t->lastPos] == '[') {
		// If an array of tables is found
		// TODO: Not currently supported
		if (t->str[t->lastPos + 1] == '[') {
			t->lastError = TOMLUnsuportedErr;
			return -1;
		}

		// Otherwise, if finds the end of the declaration of the table
		char* tableEnd = strchr(t->str, ']');
		if (!tableEnd) {
			t->lastError = TOMLFormatErr;
			return -1;
		}

		// Get the length of the table name
		int tableLen = (int)(tableEnd - t->str);
		if (tableLen == 1) {
			t->lastError = TOMLFormatErr;
			return -1;
		}

		if (tableLen >= TOML_BUFFER_SIZE) {
			t->lastError = TOMLBufferErr;
			return -1;
		}

		// Copy the table name to the buffer
		memcpy(t->buffer, t->str, tableLen);
		t->buffer[tableLen] = '\0';
		t->lastPos += tableLen + 1;
		return 1;
	}

	return 0;
} // static int processPossibleTable()


static int skipLongString(TOML* t, char const* nextNewline) {
	if (t->len > (t->lastPos + 5)){ // 5 quotation marks + 1 quotation mark in 'lastPos' position
		char* endLongStr;

		if (t->str[t->lastPos] == '\"' &&
			t->str[t->lastPos+1] == '\"' &&
			t->str[t->lastPos+2] == '\"') {

			// Does lastPos + 3 to skip the three first quotes
			endLongStr = strstr(&(t->str[t->lastPos + 3]), "\"\"\"");
			if (!endLongStr) {
				t->lastError = TOMLFormatErr;
				return TOMLFormatErr;
			}

			// Get the pos 3 of the string to skip the three last quotes
			t->lastPos = (int)(&(endLongStr[3]) - t->str);
		} else if (t->str[t->lastPos] == '\'' &&
			t->str[t->lastPos+1] == '\'' &&
			t->str[t->lastPos+2] == '\'') {

			// Does lastPos + 3 to skip the three first quotes
			endLongStr = strstr(&(t->str[t->lastPos + 3]), "\'\'\'");
			if (!endLongStr) {
				t->lastError = TOMLFormatErr;
				return TOMLFormatErr;
			}

			// Get the pos 3 of the string to skip the three last quotes
			t->lastPos = (int)(&(endLongStr[3]) - t->str);
		} else {
			// If no long string found, go to the next line (or the end of
			// the string if no newline exists)
			if (nextNewline) {
				t->lastPos = (int)(&(nextNewline[1]) - t->str);
				if (t->lastPos > t->len) {
					t->lastPos = t->len;
				}
			} else {
				t->lastPos = t->len;
			}
		}
	} else {
		// This is the case where the string we're working on isn't long enough
		// to have a long TOML string, so we get to the next newline, or to the
		// end of the document if it isn't no newline.
		if (nextNewline) {
			t->lastPos = (int)(&(nextNewline[1]) - t->str);
			if (t->lastPos > t->len) {
				t->lastPos = t->len;
			}
		} else {
			t->lastPos = t->len;
		}
	}

	return 0;
} // static int skipLongString()


static int processKeyStmt(TOML* t, char const* key) {
	// <-> Process the entry after removing any previous space

	// The equal sign to assign the value should be in the same line than the
	// name declaration, so no '\n' should exist before any equal sign.

	char* nextEqualSign = strchr(&(t->str[t->lastPos]), '=');
	char* nextNewline = strchr(&(t->str[t->lastPos]), '\n');

	if (!nextEqualSign) {
		return -1;
	}

	if (nextNewline < nextEqualSign) {
		t->lastError = TOMLFormatErr;
		return TOMLFormatErr;
	}

	// We need to consider the table this entry is on to compare to the given key
	int tableLen = (int)strlen(t->buffer);

	// Skipping backwards both the equal sign and the the spaces before the
	// found key
	char* entryKeyEnd = nextEqualSign;
	do {
		entryKeyEnd = &(entryKeyEnd[-1]);
	} while (entryKeyEnd > &(t->str[t->lastPos]) && isspace(entryKeyEnd[0]));

	// With all the previous checks, this asserted situation shouldn't happen
	assert(entryKeyEnd > &(t->str[t->lastPos]));

	// Copy the found entry key to the buffer to compare it
	int foundEntryLen = (int)(entryKeyEnd - &(t->str[t->lastPos]) + 1); // Does not include '\0'
	// The addition of '2' includes the '.' that separates table to key and the '\0'
	if ((tableLen + foundEntryLen + 2) > TOML_BUFFER_SIZE) {
		t->lastError = TOMLBufferErr;
		return TOMLBufferErr;
	}

	printDebug("Buffer with only table");
	printDebug(t->buffer);

	t->buffer[tableLen] = '.';
	memcpy(&(t->buffer[tableLen + 1]), &(t->str[t->lastPos]), foundEntryLen);
	t->buffer[tableLen + 1 + foundEntryLen] = '\0';

	printDebug("Buffer after entry added");
	printDebug(t->buffer);

	// If the found key is equal to the given one, then its found, otherwise
	// it will need to continue
	int returnPos = -1;
	if (strcmp(key, t->buffer) == 0) {
		// Return the value after the equal sign
		returnPos = (int)(&(nextEqualSign[1]) - t->str);
	}

	// Either way, found or not, we have to reach to the end of the value
	// declaration to being able to process the next ones if needed, and
	// bring the 'lastPost' variable to a correct position.
	skipSpaces(t);

	// Skip the special case of multi-line strings
	if (skipLongString(t, nextNewline) < 0) return t->lastError;

	return returnPos;
} // processKeyStmt()


static int findKeyValPos(TOML* t, char const* key) {
	assert(t != NULL);
	assert(key != NULL);

	// Not needed here to call checkValidTOML() as it was called previously by
	// the parent functions calling this.

	// If this start to read from the middle, this will make it read the
	// rest until the end, and then, from the start to that middle position.
	int initialRunPos = t->lastPos;
	bool wrappedAround = false;

	for (;;) {
		// Checks if it can continue with a possible wrap around
		if (wrappedAround && (t->lastPos > initialRunPos)) {
			return -1;
		}

		// Checks if it has arrived to the end and needs to read from the start
		if (t->lastPos >= t->len) {
			if (initialRunPos == 0) return -1;

			wrappedAround = true;
			t->lastPos = 0;
		}

		// Skip any spaces before processing. If it reaches the end, restarts
		// the loop to process the situation
		if (skipSpaces(t)) continue;

		// Skip comments. If it reaches the end, restarts
		// the loop to process the situation
		if (skipComments(t)) continue;

		int tableFound = processPossibleTable(t);
		if (tableFound == -1) return -1;

		// If a table is found, it need to check if the key belongs to that table
		if (tableFound == 1) {
			if (strstr(t->str, t->buffer) == t->str) {
				// The value belongs to the table, so it search if any of the
				// entries of the table is what it's looking for
				continue;
			} else {
				// The value doesn't belongs to the table, so it will need to
				// find a new table. The buffer is reset to 'remove' that non
				// valid table.
				t->buffer[0] = '\0';

				// Now, all the entries up next to the table belongs to it, so
				// we need to skip all of those entries and find the next table
				// (or the end of the document).

				// If it got to the end of the string, it won't be possible to find another table
				if (t->str[t->lastPos] == '\0') continue;

				char* foundTable = strchr(t->str, '[');
				if (foundTable) {
					// If it found another table, process it
					t->lastPos = (int)(foundTable - t->str);
					continue;
				} else {
					// If it doesn't found any, maybe it need to wrap around
					t->lastPos = t->len;
					continue;
				}
			}
		}

		int retProcessKeyStmt = processKeyStmt(t, key);
		if (retProcessKeyStmt < -1) {
			return retProcessKeyStmt;
		} else if (retProcessKeyStmt == -1) {
			continue;
		} else {
			return retProcessKeyStmt;
		}
	} // for (;;)
} // static int findKeyValPos()


static int compareStrBool(char const* str) {
	assert(str != NULL);

	// We need to have in mind that the only possible values are 'true' and
	// 'false'. Always lowercase, and without quatation marks (otherwise it
	// would be strings).

	// I do this in this way instead of using strcmp() or similar 'string.h'
	// functions to avoid copying 'str' to a buffer to being able to use it,
	// and also because this is easy and fast.

	int len = (int)strlen(str);

	// If len < 4 cannot be 'true' or 'false'
	if (len < 4) {
		return -1;
	}

	// Checks for 'true'
	if (len >= 4 &&
		str[0] == 't' &&
		str[1] == 'r' &&
		str[2] == 'u' &&
		str[3] == 'e' ) {
			return 1;
	}

	// Checks for 'false'
	if (len >= 5 &&
		str[0] == 'f' &&
		str[1] == 'a' &&
		str[2] == 'l' &&
		str[3] == 's' &&
		str[3] == 'e' ) {
			return 0;
	}

	return -1;
} // static int compareStrBool()


// Everytime a public TOML function get called, do this check
static inline int checkValidTOML(TOML* t) {
	assert(t != NULL && t->str != NULL);
	if ((t != NULL) && (t->str != NULL) && (strlen(t->str) > 0)) return 0;
	return -1;
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
		ret.lastPos = -1;
		ret.len = -1;
	};

	ret.str = str;
	ret.len = (int)strlen(str);
	ret.lastPos = 0;
	ret.lastError = TOMLNoError;

	return ret;
}


int getTOMLbool(TOML* t, char const* key, bool* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOML(t)) return -1;

	int keyPos = findKeyValPos(t, key);
	if (keyPos < 0) return keyPos;

	int comparison = compareStrBool(&(t->str[keyPos]));

	if (comparison == 0) {
		*value = false;
		return 0;
	} else if (comparison == 1) {
		*value = true;
		return 1;
	}

	return -1;
}


int getTOMLstr(TOML* t, char const* key, char* value, const int maxLen) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOML(t)) return -1;

	return -1;
}


int getTOMLint(TOML* t, char const* key, int* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOML(t)) return -1;

	return -1;
}


int getTOMLdouble(TOML* t, char const* key, double* value) {
	assert(t != NULL);
	assert(key != NULL);

	if (key == NULL) return -1;
	if (checkValidTOML(t)) return -1;

	return -1;
}
