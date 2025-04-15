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

	// Using this variable as unsigned to just to control more easily negative
	// from '-1' or really big values as size_t from strlen()
	unsigned int prevBufferLen = (unsigned int)strlen(t->buffer);

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


static int writeUnicodeCharsFromHex(char const* input, char* outBuffer) {
	// TODO. This is not trivial
	return TOMLOtherErr;
}


static int processEscapedChars(char const* input, char* outBuffer, int* copiedPtr, const int maxLen) {
	// Except with the Unicode escaped chars, it only skips two chars everytime
	const int amountSkipped = 2;

	// Check if the output can have the Unicode(s) char(s) or the escaped char
	//  and a '\0'.
	if ( ((input[1] == 'U') && ((*copiedPtr + 3) > maxLen)) ||
		 ((*copiedPtr + 2) > maxLen) ) {
		return TOMLOutputBufferTooSmall;
	}

	switch (input[1]) {
	case 'b':
		outBuffer[0] = '\b';
		break;

	case 't':
		outBuffer[0] = '\t';
		break;

	case 'n':
		outBuffer[0] = '\n';
		break;

	case 'f':
		outBuffer[0] = '\f';
		break;

	case 'r':
		outBuffer[0] = '\r';
		break;

	case '"':
		outBuffer[0] = '\"';
		break;

	case '\\':
		outBuffer[0] = '\\';
		break;

	case 'u':
		return writeUnicodeCharsFromHex(&(input[1]), outBuffer);

	case 'U':
		return writeUnicodeCharsFromHex(&(input[1]), outBuffer);

	default:
		return TOMLFormatErr;
	}

	// Add one to the count of characters copied to the output buffer
	++(*copiedPtr);

	return amountSkipped;
}


static int processEscapedString(char const* input,
	char const* strEnd, const int inputLen, char* output, const unsigned int maxOutLen, bool longLine) {
	// If it is a normal string, it will need to convert the escape
	// characters to what they represent.
	
	int copied = 0;
	char* str = (char*) input;

	// This will copy a limited amount of characters
	while (copied < inputLen && str < strEnd) {
		unsigned int amountToCopy;
		char* nextEscapedChar = strchr(str, '\\');
		if (nextEscapedChar == NULL || nextEscapedChar > strEnd) {
			// No escaped character what's remaining to copy in this
			// string, so it can simply copy it directly to the outbut
			// buffer.
			amountToCopy = (int)(strEnd - str);
			nextEscapedChar = NULL;
		} else {
			// There are escaped characters in the middle of in the
			// way, so it only copy the portion before them.
			amountToCopy = (int)(nextEscapedChar - str);
		}

		if ((amountToCopy + 1) > maxOutLen) return TOMLOutputBufferTooSmall;
		if (amountToCopy > 0) {
			memcpy(&(output[copied]), str, amountToCopy);
			copied += amountToCopy;
			str = &(str[amountToCopy]);
		}

		// If not more escaped chars are present the whole string was copied in the
		// previous statement.
		if (nextEscapedChar == NULL) break;

		// Oherwise, it process the remaining escaped chars present
		int countToSkip;

		// When processing a long line with escaped chars, it has to process
		// when a line ends with a '\' and a some whitespace to skip it.
		if (longLine && isspace(str[1])) {
			str = &(str[2]);
			// In that case just skip those unwanted characters
			while (isspace(str[0])) {
				str = &(str[1]);
			}

			continue;

		} else {
			countToSkip = processEscapedChars(nextEscapedChar,
				&(output[copied]), &copied, maxOutLen);
		}

		if (countToSkip < 2) return countToSkip;
		str = &(str[countToSkip]);
	} // while ()

	output[copied] = '\0';
	return copied;
} // static int processEscapedString()


static char* getEndEscpdSimpleString(char const* str) {
	// Get the next end of the line, and if it does not exist, get to the end of
	// the string (the null terminator).
	char* lastStrChar = strchr(str, '\n');
	
	if (!lastStrChar) {
		lastStrChar = (char*) &(str[strlen(str) - 1]);
	} else {
		// Skip carriage return if it exists
		if (lastStrChar[-1] == '\r') {
			lastStrChar = &(lastStrChar[-2]);
		} else {
			lastStrChar = &(lastStrChar[-1]);
		}
	}

	if (lastStrChar[0] == '"') {
		return lastStrChar;
	}

	return NULL;
}

static int extractStrFromValue(TOML* t, char* returnValue, const int maxLen) {
	char* str = (char*) & (t->str[t->pos]);
	char* strEnd = NULL;
	bool isMultiline = false;
	bool isLiteralString = false;

	// First check if it's a multiline string or not
	if (strlen(str) > 5) {
		if (str[0] == '\"' && str[1] == '\"' && str[2] == '\"') {
			strEnd = strstr(&(str[3]), "\"\"\"");
			if (strEnd == NULL) return TOMLFormatErr;
			str = &(str[3]);
			strEnd = &(strEnd[-1]);
			isMultiline = true;

		} else if (str[0] == '\'' && str[1] == '\'' && str[2] == '\'') {
			strEnd = strstr(&(str[3]), "\'\'\'");
			if (strEnd == NULL) return TOMLFormatErr;
			str = &(str[3]);
			strEnd = &(strEnd[-1]);
			isLiteralString = true;
			isMultiline = true;
		}
	}

	if (isMultiline) {
		// Skip the first newline if exists. Could be CRLF or LF ended.
		if (str[0] == '\n') str = (char*)&(str[1]);
		if (str[0] == '\r' && str[1] == '\n') str = (char*)&str[2];

		int strLen = (int)(strEnd - str);
		if (strLen == 0) {
			returnValue[0] = '\0';

			// Update the latest string position in the structure. The '+3' is
			// to skip the ending terminators.
			t->pos += (int) (strEnd - &(t->str[t->pos]) + 3);

			return 0;
		}

		if (isLiteralString) {
			// Check if the buffer is big enough to copy the string content
			if ((strLen + 1) > maxLen) return TOMLOutputBufferTooSmall;

			// Copy the string to the output buffer and add a null terminator
			memcpy(returnValue, str, strLen);
			returnValue[strLen] = '\0';

			t->pos += (int)(strEnd - &(t->str[t->pos]) + 3);

			return strLen;
		} else {
			// It process that multi-line string like a normal one. The main
			// difference in both is just skipping spaces after an escaped
			// newline.

			t->pos += (int)(strEnd - &(t->str[t->pos]) + 3);

			return processEscapedString(str, strEnd, strLen, returnValue,
				maxLen, true);
		}
	} // if (isMultiline)

	// Checks if there is a literal or formatted single line string
	if (str[0] == '\'') {
		// Found a literal single line string!

		str = &(str[1]); // Skips ' delimiter
		strEnd = (char*)strchr(str, '\'');
		
		if (!strEnd) return TOMLFormatErr;

		int strLen = (int)(strEnd - str);

		// Update the latest string position for the next usage of the TOML structure
		t->pos += (int)(strEnd - &(t->str[t->pos]) + 1);

		if ((strLen + 1) > maxLen) return TOMLOutputBufferTooSmall;

		// If it's just an empty string, it will only have to add the '\0' and
		// return.
		if (strLen == 0) {
			returnValue[0] = '\0';
			return 0;
		}

		// Copy the string and add a null terminator.
		memcpy(returnValue, str, strLen);
		returnValue[strLen] = '\0';
		return strLen;

	} else if (str[0] == '\"') {
		// Found an escaped single line string!

		str = &(str[1]); // Skips " delimiter
		strEnd = getEndEscpdSimpleString(str);
		if (!strEnd) return TOMLFormatErr;

		int strLen = (int)(strEnd - str);

		// Update the latest string position for the next usage of the TOML structure
		t->pos += (int)(strEnd - &(t->str[t->pos]) + 1);

		// In the case of not being a literal single line string
		return processEscapedString(str, strEnd, strLen, returnValue, maxLen, false);
	} else {
		// If a string isn't found this key could have a VALID value of another type
		
		// TODO: A good idea would be to go back to thre PREVIOUS newline, to the
		// start of the key name instead of 
		return TOMLInvalidValue;
	}

} // static int extractStrFromValue()


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
