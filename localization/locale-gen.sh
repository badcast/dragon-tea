#!/bin/sh

find ../ ../src -type f -name "*.c" -o -name "*.h" | xargs xgettext -j --msgid-bugs-address="lmecomposer@gmail.com" --copyright-holder="badcast" --keyword=gettext --keyword=_ -o messages.pot --from-code=UTF-8 -d dragon-tea

if [ ! -e "translates/ru_RU.po" ]; then
    msginit --locale=ru_RU.UTF-8 --input=messages.pot --output=translates/ru_RU.po
else
    msgmerge --update translates/ru_RU.po messages.pot
fi


sh ./locale-gen-machine.sh
