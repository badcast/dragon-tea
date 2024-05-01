#!/bin/sh

function gen_locales()
{
    # Generate language (example, en_US, ru_RU)

    for l in $@; do
        echo Generating locale for $l
        if [ ! -e "translates/$l.po" ]; then
            msginit --locale=$l.UTF-8 --input=messages.pot --output=translates/$l.po
        else
            msgmerge --update translates/$l.po messages.pot
        fi

        # Gen machine (.mo)

        mkdir -p ./locale/$l/LC_MESSAGES

        msgfmt translates/$l.po -o ./locale/$l/LC_MESSAGES/dragon-tea.mo

    done
}

find ../ ../src -type f -name "*.c" -o -name "*.h" | xargs xgettext -j --msgid-bugs-address="lmecomposer@gmail.com" --copyright-holder="badcast" --keyword=gettext --keyword=_ -o messages.pot --from-code=UTF-8 -d dragon-tea

gen_locales ru_RU kk_KZ
