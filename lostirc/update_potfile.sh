#!/bin/sh
xgettext -d lostirc --keyword="_" -f po/POTFILES.in
mv lostirc.po po/lostirc.pot
