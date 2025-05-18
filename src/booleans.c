// José Miguel Rodríguez Marchena (josemirm)

#include "booleans.h"

//
// -- Boolean processing function
//

int compareStrBool(char const* str) {
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
} // int compareStrBool()
