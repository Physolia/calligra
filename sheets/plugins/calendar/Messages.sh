#! /bin/sh
source ../../../calligra_xgettext.sh

$EXTRACTRC *.ui >> rc.cpp
calligra_xgettext rc.cpp *.cpp > $podir/CalendarTool.pot
