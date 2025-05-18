// José Miguel Rodríguez Marchena (josemirm)
#include "utils.h"

//
// -- General helping functions
//

int skipSpaces(TOML* t) {
	while (isspace(t->str[t->pos]) && (t->pos < t->len)) {
		t->pos++;
	}

	if (t->pos == t->len) return 1;

	return 0;
}


int skipComments(TOML * t) {
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


int checkValidTOMLStructure(TOML* t) {
	// Everytime a public TOML function get called, do this check
	assert(t != NULL && t->str != NULL);
	if ((t != NULL) && (t->str != NULL) && (strlen(t->str) > 0)) return 0;
	return -1;
}
