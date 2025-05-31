// José Miguel Rodríguez Marchena (josemirm)

//#include <stdio.h>

#include <stdbool.h>
#include <stdio.h>
#include "../src/toml.h"

#define getTestResults(n, name, funcPtr) \
	{\
		fprintf(stderr, "Test %d: %s\n", n, name);\
\
		int (*__funcToExecute)() = funcPtr;\
		int res = __funcToExecute();\
\
		if (res == 0) {\
			fprintf(stderr, "Result: OK\n\n");\
		} else {\
			fprintf(stderr, "Result: FAIL. Error code: %d\n\n", res);\
		}\
	}\

int bufSize = 256;
char buffer[256];


int booleanExtractingTest() {
	typedef struct {
		char ip[64];
		char role[32];
		bool active;
	} Server_t;

	const char* simpleStr = "\
[servers.alpha]\n\
ip = \"10.0.0.1\"\n\
role = \"frontend\"\n\
active = true";

	Server_t values;

	TOML toml = initTOML(simpleStr);
	// if (getTOMLstr(&toml, "server.alpha.ip", values.ip, 64)) return 1;
	// if (getTOMLstr(&toml, "server.alpha.role", values.role, 32)) return 1;
	int ret = getTOMLbool(&toml, "servers.alpha.active", &values.active);

	printf("ret: %d\n", ret);
	if (ret < 0) {
		printf("Error found: %d\n", toml.lastError);
		return 1;
	}

	return 0;
}


int escapedStringExtractionTest () {
	int ret = 0;

	const char* someStr = "\
str1 = \"I\'m a string. \\\"You can quote me\\\".\"\n\
\n\
str2 = \"\"\"\\\n\
       The quick brown \\\n\
       fox jumps over \\\n\
       the lazy dog.\\\n\
       \"\"\"\n\
";

	TOML toml = initTOML(someStr);
	int ret1 = getTOMLstr(&toml, "str1", buffer, bufSize);
	if (ret1 < 0) {
		printf("Error getting str1: %d\n", toml.lastError);
		ret = ret1;
	}

	int ret2 = getTOMLstr(&toml, "str2", buffer, bufSize);
	if (ret2 < 0) {
		printf("Error getting str2: %d\n", toml.lastError);
		ret = ret2;
	}

	return ret;
}


int literalStringExtractionTest() {
	int ret = 0;

	const char* tomlStr = "path = \'\\\\ServerX\\admin$\\system32\\\'\
\n\
text = \'\'\'\n\
The first newline is\n\
trimmed in raw strings.\n\
    All other whitespace\n\
    is preserved.\n\
\'\'\'";

	TOML toml = initTOML(tomlStr);
	int ret1 = getTOMLstr(&toml, "path", buffer, bufSize);
	if (ret1 < 0) {
		printf("Error getting str1: %d\n", toml.lastError);
		ret = ret1;
	}
	//  else {
	// 	printf("path -> %s\n", buffer);
	// }

	int ret2 = getTOMLstr(&toml, "text", buffer, bufSize);
	if (ret2 < 0) {
		printf("Error getting str2: %d\n", toml.lastError);
		ret = ret2;
	}
	// else {
	// 	printf("text -> %s\n", buffer);
	// }

	return ret;
}



void checkIntResult(TOML* toml, char const* key, TOMLInt_t value, int* ret) {
	TOMLInt_t number;
	int tomlRet = getTOMLint(toml, key, &number);
	if (tomlRet < 0 || number != value) {
		printf("Error getting %s number. Error %d. Number readed: %lld\n", key, toml->lastError, number);
		*ret = -1;
	}
}

void checkDoubleResult(TOML* toml, char const* key, double value, int* ret) {
	double number;
	int tomlRet = getTOMLdouble(toml, key, &number);
	if (tomlRet < 0 || number != value) {
		printf("Error getting %s number. Error %d. Number readed: %E\n", key, toml->lastError, number);
		*ret = -1;
	}
}

int numberExtractionTest() {
	int ret = 0;

	const char* tomlText = "normal = 123\nespaced = 1_234_567\noctal=0o777_111_421\nbin = 0b1100_1010\nhex = 0xdead_beef\n";
	TOML toml = initTOML(tomlText);

	checkIntResult(&toml, "normal", 123, &ret);
	checkIntResult(&toml, "espaced", 1234567, &ret);
	checkIntResult(&toml, "octal", 0x7FC9311, &ret);
	checkIntResult(&toml, "bin", 0b11001010, &ret);
	checkIntResult(&toml, "hex", 0xDEADBEEF, &ret);


	const char* tomlTextDouble = "dbl1 = +1.0\ndbl2 = 3.14_15\ndbl3 = -0.01\ndbl4 = 5e+22\ndbl5 = 1e06\ndbl6 = -2E-2\ndbl7 = 6.626e-34";
	TOML tomlDbl = initTOML(tomlTextDouble);

	checkDoubleResult(&tomlDbl, "dbl1", 1.0, &ret);
	checkDoubleResult(&tomlDbl, "dbl2", 3.1415, &ret);
	checkDoubleResult(&tomlDbl, "dbl3", -0.01, &ret);
	checkDoubleResult(&tomlDbl, "dbl4", 5e+22, &ret);
	checkDoubleResult(&tomlDbl, "dbl5", 1e06, &ret);
	checkDoubleResult(&tomlDbl, "dbl6", -2E-2, &ret);
	checkDoubleResult(&tomlDbl, "dbl7", 6.626e-34, &ret);	

	return ret;
} // int numberExtractionTest()


int main(void) {
	fprintf(stderr, "Start of CTOML testing:\n\n");

	//getTestResults(1, "Extracting a boolean", booleanExtractingTest());
	getTestResults(2, "Extracting a single-line and a multi-line escaped string", escapedStringExtractionTest);
	getTestResults(3, "Extracting a single-line and a multi-line literal string", literalStringExtractionTest);
	getTestResults(4, "Extracting integer and floating-point numbers", numberExtractionTest);

	return 0;
}