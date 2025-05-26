// José Miguel Rodríguez Marchena (josemirm)

#include "strings.h"

//
// -- String processing functions
//


int writeUnicodeCharsFromHex(char const* input, char* outBuffer) {
	// TODO. This is not trivial
	return TOMLOtherErr;
}


int processEscapedChars(char const* input, char* outBuffer, int* copiedPtr, const int maxLen) {
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


int processEscapedString(char const* input,
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
} // int processEscapedString()


char* getEndEscpdSimpleString(char const* str) {
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

int extractStrFromValue(TOML* t, char* returnValue, const int maxLen) {
	char* str = (char*) & (t->str[t->pos]);
	char* strEnd = NULL;
	bool isMultiline = false;
	bool isLiteralString = false;

	// First check if it's a multiline string or not
	if (strlen(str) > 5) {
		if (str[0] == '\"' && str[1] == '\"' && str[2] == '\"') {
			// If it starts with """ and ends in the same way is a multiline string
			strEnd = strstr(&(str[3]), "\"\"\"");
			if (strEnd == NULL) return TOMLFormatErr;
			str = &(str[3]);
			strEnd = &(strEnd[-1]);
			isMultiline = true;

		} else if (str[0] == '\'' && str[1] == '\'' && str[2] == '\'') {
			// If it starts with ''' and ends in the same way is a multiline string
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

} // int extractStrFromValue()
