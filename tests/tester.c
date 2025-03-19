// José Miguel Rodríguez Marchena (josemirm)

//#include <stdio.h>

#include <stdbool.h>
#include <stdio.h>
#include "../src/toml.h"

typedef struct {
	char ip[64];
	char role[32];
	bool active;
} Server_t;

int main(void) {

	const char* simpleStr = "\
[servers.alpha]\
ip = \"10.0.0.1\"\
role = \"frontend\"\
active = true";

	Server_t values;

	TOML toml = initTOML(simpleStr);
	// if (getTOMLstr(&toml, "server.alpha.ip", values.ip, 64)) return 1;
	// if (getTOMLstr(&toml, "server.alpha.role", values.role, 32)) return 1;
	int ret = getTOMLbool(&toml, "servers.alpha.active", &values.active);

	printf("ret: %d\n", ret);
	if (ret < 0) {
		printf("Error found: %d\n", toml.lastError);
	}


	return 0;
}
