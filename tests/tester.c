// José Miguel Rodríguez Marchena (josemirm)

//#include <stdio.h>

#include <stdbool.h>
#include <stdio.h>
#include "../src/toml.h"
#include "../tests/test_util.h"


#define getTestResults(n, name, res) \
	if (res == 0) {\
		fprintf(stderr, "[ OK ] Test %d (%s)\n\n", n, name);\
	} else {\
		fprintf(stderr, "[FAIL] Test %d (%s) Failed with code %d\n\n", n, name, res);\
	}\

int bufSize = 256;
char buffer[256];

int test1() {
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


int test2 () {
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

int test3() {
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
	} else {
		printf("path -> %s\n", buffer);
	}

	int ret2 = getTOMLstr(&toml, "text", buffer, bufSize);
	if (ret2 < 0) {
		printf("Error getting str2: %d\n", toml.lastError);
		ret = ret2;
	} else {
		printf("text -> %s\n", buffer);
	}

	return ret;
}

int main(void) {
	fprintf(stderr, "Start of CTOML testing:\n\n");
	//getTestResults(1, "Extracting a boolean", test1());
	getTestResults(2, "Extracting a single-line and a multi-line escaped string", test2());
	getTestResults(3, "Extracting a single-line and a multi-line literal string", test3());

	return 0;
}