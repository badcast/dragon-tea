/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef _TEA_LOCALIZATION_H_
#define _TEA_LOCALIZATION_H_ 1

#include <locale.h>
#include <libintl.h>

#define _(String) gettext(String)

#define DRAGON_TEA_TEXTDOMAIN "dragon-tea"

#define LANGMAX 3

static const char *LANGUAGES[LANGMAX] = {"en_US", "ru_RU", "kk_KZ"};

#endif
