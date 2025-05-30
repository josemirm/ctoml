// José Miguel Rodríguez Marchena (josemirm)
#include "numericals.h"

//
//-- Integer number extraction
//

static int processOctalNumber(TOML* t, TOMLInt_t* returnValue) {
	TOMLInt_t ret = 0;
	int bitCount = 0;


	while (t->pos < t->len) {
		// First digit has to be a number to be valid.

		// If it already reached the maximum amount of bits to save in the
		// integer, then the number is too big to save it and an error must
		// be returned.
		if (('0' <= t->str[t->pos]) && (t->str[t->pos] <= '7')) {
			ret *= 8;
			ret += (TOMLInt_t)(t->str[t->pos] - '0');
			bitCount += 3;
			if (bitCount >= TOML_MAX_INT_LEN) return TOMLBufferErr;

		} else if (t->str[t->pos] == '_') {
			// It can't start with an underscore or have one after another
			if ((bitCount == 0) || t->str[t->pos - 1] == '_') return TOMLFormatErr;

		} else if (isspace(t->str[t->pos])) {
			// If some blank space or newline found, the whole number is inserted
			t->pos++;
			break;

		} else {
			// If some other character is found, then that's not a valid octal
			// number format.
			return TOMLFormatErr;
		}

		t->pos++;
	}

	*returnValue = ret;
	return 0;
}


static int processHexNumber(TOML* t, TOMLInt_t* returnValue) {
	TOMLInt_t ret = 0;
	int bitCount = 0;

	while (t->pos < t->len) {
		// First digit has to be a number to be valid.

		// If it already reached the maximum amount of bits to save in the
		// integer, then the number is too big to save it and an error must
		// be returned.
		if (isdigit(t->str[t->pos])) {
			ret *= 16;
			ret += (TOMLInt_t)(t->str[t->pos] - '0');
			bitCount += 4;
			if (bitCount >= TOML_MAX_INT_LEN) return TOMLBufferErr;

		} else if (isxdigit(t->str[t->pos])) {
			ret *= 16;
			ret += 10 + (TOMLInt_t)(tolower(t->str[t->pos]) - 'a');
			bitCount += 4;
			if (bitCount >= TOML_MAX_INT_LEN) return TOMLBufferErr;

		} else if (t->str[t->pos] == '_') {
			// It can't start with an underscore or have one after another
			if ((bitCount == 0) || t->str[t->pos - 1] == '_') return TOMLFormatErr;

		} else if (isspace((t->str[t->pos]))) {
			// If some blank space or newline found, the whole number is inserted
			t->pos++;
			break;
		} else {
			// If some other character is found, then that's not a valid hex
			// number format.
			return TOMLFormatErr;
		}

		t->pos++;
	}

	*returnValue = ret;
	return 0;
}


static int processBinNumber(TOML* t, TOMLInt_t* returnValue) {
	TOMLInt_t ret = 0;
	unsigned int bitCount = 0;

	while (t->pos < t->len) {
		// First digit has to be a number to be valid

		// If it already reached the maximum amount of bits to save in the
		// integer, then the number is too big to save it and an error must
		// be returned.

		if ((t->str[t->pos] == '0') || (t->str[t->pos] == '1')) {
			ret <<= 1;
			ret |= (TOMLInt_t)(t->str[t->pos] - '0');
			bitCount++;
			if (bitCount >= TOML_MAX_INT_LEN) return TOMLBufferErr;

		} else if (t->str[t->pos] == '_') {
			// It can't start with an underscore or have one after another
			if ((bitCount == 0) || t->str[t->pos - 1] == '_') return TOMLFormatErr;

		} else if (isspace(t->str[t->pos])) {
			// If some blank space or newline found, the whole number is inserted
			t->pos++;
			break;

		} else {
			// If some other character is found, then that's not a valid binary
			// number format.
			return TOMLFormatErr;
		}

		t->pos++;
	}

	*returnValue = ret;
	return 0;
}


static inline int safeDecimalIncrement(TOMLInt_t* ret, const TOMLInt_t newDigit, bool isNegative) {
	TOMLInt_t before = *ret;
	*ret *= 10;

	if (isNegative) {
		*ret -= newDigit;
		if (*ret > before) return -1;
	} else {
		*ret += newDigit;
		if (*ret < before) return -1;
	}

	return 0;
}


static int processDecNumber(TOML* t, TOMLInt_t* returnValue, bool isNegative) {
	TOMLInt_t ret = 0;
	bool firstDigit = true;

	while (t->pos < t->len) {
		// First digit has to be a number to be valid
		if (isdigit(t->str[t->pos])) {
			if (safeDecimalIncrement(&ret, (TOMLInt_t)(t->str[t->pos] - '0'), isNegative)) {
				return TOMLBufferErr;
			}

			firstDigit = false;

		} else if (t->str[t->pos] == '_') {
			// It can't start with an underscore or have one after another
			if (firstDigit || t->str[t->pos - 1] == '_') return TOMLFormatErr;

		} else if (isspace(t->str[t->pos])) {
			// If some blank space or newline found, the whole number is inserted
			t->pos++;
			break;
		}

		t->pos++;
	}

	*returnValue = ret;
	return 0;
}


int extractIntFromValue(TOML* t, TOMLInt_t* returnValue) {
	// Valid integers could be: +12, 3, 0, -23, 1234_5_67, 0xDeA_dBeEf, 0o755, 0b1001_1011
	bool isNegative = false;

	// Get the sign at start if it is present
	bool hasSign = false;
	if (t->str[t->pos] == '-') {
		isNegative = true;
		t->pos++;
		hasSign = true;

	} else if (t->str[t->pos] == '+') {
		t->pos++;
		hasSign = true;
	}

	// If it's formated in hex or octal get it correctly
	if (t->str[t->pos] == '0') {
		if (hasSign) return TOMLFormatErr;

		switch (t->str[t->pos + 1]) {
			case 'o':
				t->pos += 2;
				return processOctalNumber(t, returnValue);
			case 'x':
				t->pos += 2;
				return processHexNumber(t, returnValue);
			case 'b':
				t->pos += 2;
				return processBinNumber(t, returnValue);
			default:
				return TOMLFormatErr;
		}
	} else {
		return processDecNumber(t, returnValue, isNegative);
	}
}



//
//-- Double number extraction
//


/**
 * Gets all those special values apart, like infinite, negative zero, etc.
 * @param  t           TOML structure
 * @param  returnValue Return of the value extracted
 * @return             0 if not found, 1 if found
 */
static int checkSpecialDoubleValues(TOML* t, double* returnValue, bool isNegative) {
	if ( (t->pos + 2) > t->len) return 0;

	// I prefer checking those small string this way to not deal to strcmp or
	// other functions idiosyncrasies
	if (t->str[t->pos] == 'n' &&
		t->str[t->pos + 1 == 'a' &&
		t->str[t->pos + 2] == 'n']) {
		*returnValue = nan((isNegative? "-1": "0"));
		return 1;

	} else if (t->str[t->pos] == 'i' &&
		t->str[t->pos + 1 == 'n' &&
		t->str[t->pos + 2] == 'f']) {
		*returnValue = (isNegative? -INFINITY: INFINITY);
		return 1;
	}

	return 0;
}


int extractDoubleFromValue(TOML* t, double* returnValue) {
	bool isNegative = false;

	// Get sign if it's available
	if (t->str[t->pos] == '-') {
		isNegative = true;
		t->pos++;
	} else if (t->str[t->pos] == '+') {
		t->pos++;
	}

	// Check special cases
	if (checkSpecialDoubleValues(t, returnValue, isNegative)){
		// In case of finding a special case, it returns its value and there's
		// no need to continue processing the string.
		return 0;
	}

	// Extract mantissa
	TOMLInt_t mantissa = 0;

	// It needs to start with a digit
	if (!isdigit(t->str[t->pos])) return TOMLFormatErr;

	while (t->pos < t->len) {
		if (isdigit(t->str[t->pos])) {
			mantissa *= 10;
			mantissa += t->str[t->pos] - '0';

		} else if ( (t->str[t->pos] == '_') &&
					(t->str[t->pos + 1] == '_')) {
			// If double underscore inserted is an error
			return TOMLFormatErr;
		} else {
			// If some other character found, go to the next check
			break;
		}

		t->pos++;
		continue;
	}

	// Now get the decimals and exponents
	TOMLInt_t decs = 0;
	TOMLInt_t exp = 0;
	bool negativeExp = false;

	if (t->str[t->pos] == '.') {
		if (!isdigit(t->str[t->pos + 1])) return TOMLFormatErr;

		// Get the decimals
		t->pos++;
		while ((t->pos < t->len) && isdigit(t->str[t->pos])) {
			decs *= 10;
			decs += t->str[t->pos] - '0';
			t->pos++;
		}
	}

	if (t->str[t->pos] == 'e' || t->str[t->pos] == 'E') {
		t->pos++;

		if (t->str[t->pos] == '-') {
			negativeExp = true;
			t->pos++;
		} else if (t->str[t->pos] == '+') {
			t->pos++;
		}

		if (!isdigit(t->str[t->pos])) return TOMLFormatErr;
		while ( (t->pos < t->len) && isdigit(t->str[t->pos])) {
			exp *= 10;
			exp += t->str[t->pos] - '0';
			t->pos++;
		}
	}

	// There should be a blank space after entering the number, and not any
	// other kind of character
	if (!isspace(t->str[t->pos])) return TOMLFormatErr;

	return 0;
} // int extractDoubleFromValue()
