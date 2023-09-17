#!/bin/sh

find ../ ../src -type f -name "*.c" -o -name "*.h" | xargs xgettext -j --msgid-bugs-address="lmecomposer@gmail.com" --copyright-holder="badcast" --keyword=gettext --keyword=_ -o messages.pot --from-code=UTF-8 -d dragon-tea

msginit --locale=ru_RU.UTF-8 --input=messages.pot --output=translates/ru_RU.po

sh ./locale-fmt.sh
