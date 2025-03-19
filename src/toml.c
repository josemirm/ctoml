// José Miguel Rodríguez Marchena (josemirm)

#include "toml.h"

//
// -- Helper functions
//


static inline int skipSpaces(TOML* t) {
	while (isspace(t->str[t->pos]) && (t->pos < t->len)) {
		t->pos++;
	}

	if (t->pos == t->len) return 1;

	return 0;
}


static inline int skipComments(TOML * t) {
	if (t->str[t->pos] == '#') {
		char* endComment = strchr(&(t->str[t->pos]), '\n');
		if (!endComment || *endComment == '\0') {
			t->pos = t->len;
			return 1;
		}

		int newPos = (int)(endComment - t->str);
		t->pos = newPos;
	}

	return 0;
}


static int processPossibleTable(TOML* t) {
	// If a table is found
	if (t->str[t->pos] == '[') {
		// If an array of tables is found
		// TODO: Not currently supported
		if (t->str[t->pos + 1] == '[') {
			t->lastError = TOMLUnsuportedErr;
			return -1;
		}

		// Skips the '[' character
		t->pos++;

		// Otherwise, if finds the end of the declaration of the table
		char* tableEnd = strchr(&(t->str[t->pos]), ']');
		if (!tableEnd) {
			t->lastError = TOMLFormatErr;
			return -1;
		}

		// Get the length of the table name
		int tableLen = (int)(tableEnd - &(t->str[t->pos]));
		if (tableLen == 1) {
			t->lastError = TOMLFormatErr;
			return -1;
		}

		if (tableLen >= TOML_BUFFER_SIZE) {
			t->lastError = TOMLBufferErr;
			return -1;
		}

		// Copy the table name to the buffer
		memcpy(t->buffer, &(t->str[t->pos]), tableLen);
		t->buffer[tableLen] = '\0';

		// Skip the closing sign of the table and the spaces up next
		t->pos += tableLen + 1;
		skipSpaces(t);

		return 1;
	}

	return 0;
} // static int processPossibleTable()


static int skipEntryValue(TOML* t, char const* nextNewline) {
	// First try to skip a multi-line (long) string
	if (t->len > (t->pos + 5)){ // 5 quotation marks + 1 quotation mark in 'pos' position
		char* endLongStr;

		if (t->str[t->pos] == '\"' &&
			t->str[t->pos+1] == '\"' &&
			t->str[t->pos+2] == '\"') {

			// Does pos + 3 to skip the three first quotes
			endLongStr = strstr(&(t->str[t->pos + 3]), "\"\"\"");
			if (!endLongStr) {
				t->lastError = TOMLFormatErr;
				return TOMLFormatErr;
			}

			// Get the pos 3 of the string to skip the three last quotes
			t->pos = (int)(&(endLongStr[3]) - t->str);
		} else if (t->str[t->pos] == '\'' &&
			t->str[t->pos+1] == '\'' &&
			t->str[t->pos+2] == '\'') {

			// Does pos + 3 to skip the three first quotes
			endLongStr = strstr(&(t->str[t->pos + 3]), "\'\'\'");
			if (!endLongStr) {
				t->lastError = TOMLFormatErr;
				return TOMLFormatErr;
			}

			// Get the pos 3 of the string to skip the three last quotes
			t->pos = (int)(&(endLongStr[3]) - t->str);
		} else {
			// If no multi-line string found, go to the next line (or the end of
			// the string if no newline exists)
			if (nextNewline) {
				t->pos = (int)(&(nextNewline[1]) - t->str);
				if (t->pos > t->len) {
					t->pos = t->len;
				}
			} else {
				t->pos = t->len;
			}
		}
	} else {
		// This is the case where the string we're working on isn't long enough
		// to have a multi-line TOML string, so we get to the next newline, or to the
		// end of the document if it isn't no newline.
		if (nextNewline) {
			t->pos = (int)(&(nextNewline[1]) - t->str);
			if (t->pos > t->len) {
				t->pos = t->len;
			}
		} else {
			t->pos = t->len;
		}
	}

	return 0;
} // static int skipEntryValue()


static int processKeyStmt(TOML* t, char const* key) {
	// <-> Process the entry after removing any previous space

	// The equal sign to assign the value should be in the same line than the
	// name declaration, so no '\n' should exist before any equal sign.

	char* nextEqualSign = strchr(&(t->str[t->pos]), '=');
	char* nextNewline = strchr(&(t->str[t->pos]), '\n');

	if (!nextEqualSign) {
		return -1;
	}

	if ((nextNewline != NULL) && (nextNewline < nextEqualSign)) {
		t->lastError = TOMLFormatErr;
		return TOMLFormatErr;
	}

	int prevBufferLen = (int)strlen(t->buffer);

	// Skipping backwards both the equal sign and the the spaces before the
	// found key
	char* entryKeyEnd = nextEqualSign;
	do {
		entryKeyEnd = &(entryKeyEnd[-1]);
	} while (entryKeyEnd > &(t->str[t->pos]) && isspace(entryKeyEnd[0]));

	// With all the previous checks, this asserted situation shouldn't happen
	assert(entryKeyEnd > &(t->str[t->pos]));

	int foundEntryLen = (int)(entryKeyEnd - &(t->str[t->pos]) + 1); // Does not include '\0'
	
	// If no previous table entry was found, it compares the key with the found
	// entry key directly. Otherwise, it will add the found entry to the table
	// name temporally.
	// At the end, the buffer should be restored as its previous state.
	
	// Check if the buffer is big enough to write inside the entry. The '+2' in
	// the first case is to count the '.' between the table and the entry and the
	// '\0'. The other case only have the '+1' for the '\0'
	if ((prevBufferLen > 0) && ((prevBufferLen + foundEntryLen + 2) > TOML_BUFFER_SIZE) ||
		(prevBufferLen == 0) && ((foundEntryLen + 1) > TOML_BUFFER_SIZE)) {
		t->lastError = TOMLBufferErr;
		return TOMLBufferErr;
	}

	if (prevBufferLen > 0) {
		t->buffer[prevBufferLen] = '.';
		memcpy(&(t->buffer[prevBufferLen + 1]), &(t->str[t->pos]), foundEntryLen);
		t->buffer[foundEntryLen + prevBufferLen + 1] = '\0';
	}
	else {
		memcpy(t->buffer, &(t->str[t->pos]), foundEntryLen);
		t->buffer[foundEntryLen] = '\0';
	}

	// If the found key is equal to the given one, then its found, otherwise
	// it will need to continue
	int returnPos = -1;
	if (strcmp(key, t->buffer) == 0) {
		// Return the key position after the equal sign skipping any spaces
		t->pos = (int)(&(nextEqualSign[1]) - t->str);
		skipSpaces(t);
		returnPos = t->pos;
	}

	// Return the state of the buffer. It's "double-checked" here to remove
	// warnings about buffer overrunning.
	t->buffer[prevBufferLen] = '\0';

	// If this entry isn't what it was looking for, skip its value
	if ((returnPos < 0) && (skipEntryValue(t, nextNewline) < 0)) return t->lastError;

	return returnPos;
} // processKeyStmt()


static int findKeyValPos(TOML* t, char const* key) {
	assert(t != NULL);
	assert(key != NULL);

	// If this start to read from the middle, this will make it read the
	// rest until the end, and then, from the start to that middle position.
	int initialRunPos = t->pos;
	bool wrappedAround = false;
	bool inCurrentTable = true;

	for (;;) {
		// Checks if it can continue with a possible wrap around
		if (wrappedAround && (t->pos > initialRunPos)) {
			return -1;
		}

		// Checks if it has arrived to the end and needs to read from the start
		if (t->pos >= t->len) {
			if (initialRunPos == 0) return -1;

			wrappedAround = true;
			t->pos = 0;

			// Statements at the start won't belong to previously found tables

			t->buffer[0] = '\0';
		}

		// Skip any spaces before processing. If it reaches the end, restarts
		// the loop to process the situation
		if (skipSpaces(t)) continue;

		// Skip comments. If it reaches the end, restarts
		// the loop to process the situation
		if (skipComments(t)) continue;

		int tableFound = processPossibleTable(t);
		if (tableFound == -1) return -1;

		// Check if the table found belongs to the key or not.
		if (tableFound == 1) {
			inCurrentTable = ( strstr(key, t->buffer) == key );
		}

		// If the key is not in the current table, it will have to skip until
		// the next table in found.
		if (!inCurrentTable) {
			char* nextNewline = strchr(&(t->str[t->pos + 1]), '\n');
			if (nextNewline) {
				t->pos = (int)(nextNewline - t->str + 1);
			}

			continue;
		}

		int retProcessKeyStmt = processKeyStmt(t, key);
		if (retProcessKeyStmt < -1) {
			// Some error found
			return retProcessKeyStmt;

		} else if (retProcessKeyStmt == -1) {
			// The key readed isn't what it was looking for
			continue;

		} else {
			// The key found was what it was looking for
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
static inline int checkValidTOMLStructure(TOML* t) {
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
		ret.pos = -1;
		ret.len = -1;
	};

	ret.str = str;
	ret.len = (int)strlen(str);
	ret.pos = 0;
	ret.lastError = TOMLNoError;

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
	if (checkValidTOMLStructure(t)) return -1;

	return -1;
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
