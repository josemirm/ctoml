/**
 * @file proc_stments.h
 * @brief TOML statements processing
 *
 */

// José Miguel Rodríguez Marchena (josemirm)

#ifndef __JOSEMIRM_CTOML_PROC_STMENTS__
#define __JOSEMIRM_CTOML_PROC_STMENTS__

#include "toml.h"
#include "utils.h"

int processPossibleTable(TOML* t);
int skipEntryValue(TOML* t, char const* nextNewline);
int processKeyStmt(TOML* t, char const* key);
int findKeyValPos(TOML* t, char const* key);

#endif
