#! /bin/sh
$EXTRACTRC `find . -name \*.ui | grep -v '/tests/'` >> rc.cpp
RCFILES=`find . -name \*.rc                                                   \
	| grep -v plugins/extensions/metadataeditor/editors/dublincore.rc     \
	| grep -v plugins/extensions/metadataeditor/editors/exif.rc           \
	| grep -v sketch/KritaSketchWin.rc                                    \
	| grep -v gemini/KritaGeminiWin.rc
         `
$EXTRACTRC $RCFILES >> rc.cpp
perl extracti18n.pl > i18ndata
# ignore sdk/templates since it contains templates for use a future plugins, none of the strings there will ever be seen by any user
$XGETTEXT -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 -kkundo2_i18nc:1c,2 -kkundo2_i18ncp:1c,2,3 i18ndata rc.cpp `find . -name \*.cc -o -name \*.h  -o -name \*.cpp | grep -v '/tests/' | grep -v './sdk/templates'` ui/kis_aboutdata.h -o $podir/krita.pot
rm -f i18ndata
