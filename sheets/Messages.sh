#! /bin/sh
$EXTRACTRC --tag-group=koffice functions/*.xml > xml_doc.cpp
$EXTRACTRC chart/*.ui dialogs/*.ui  part/dialogs/*.ui *.kcfg *.rc >> rc.cpp
$EXTRACTATTR --attr=optionWidget,name ui/CellToolOptionWidgets.xml >> rc.cpp
$XGETTEXT -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 *.cpp chart/*.cpp commands/*.cpp database/*.cpp dialogs/*.cpp functions/*.cpp part/AboutData.h part/*.cpp part/commands/*.cpp part/dialogs/*.cpp ui/*.cpp -o $podir/sheets.pot
rm xml_doc.cpp rc.cpp
