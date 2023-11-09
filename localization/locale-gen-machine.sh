#!/bin/sh

mkdir -p ./locale/ru_RU/LC_MESSAGES

msgfmt translates/ru_RU.po -o ./locale/ru_RU/LC_MESSAGES/dragon-tea.mo
