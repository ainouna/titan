#####
#echo "### translation" > source.titan/po/en/LC_MESSAGES/titan.po
#cat source.titan/skin/default/skin.xml source.titan/titan/plugins/mc/skin.xml source.titan/titan/plugins/aafpanel/skin.xml source.titan/titan/plugins/networkbrowser/skin.xml | sed 's/title="/title=/' | sed 's/title=/\ntitle=!/' | grep title= | sed 's/bordercol=/\n/' | sed 's/valign=/\n/'  |sed 's/posy=/\n/' | sed 's/parent=/\n/' | sed 's/halign=/\n/' | sed 's/type=/\n/' | sed 's/name=/\n/' | sed 's/func=/\n/'| sed 's/progresscol=/\n/'| grep ^title= | sed "s#\".*##" | sort -u |  sed 's/title=!/msgid "/' | tr '\n' '#' | sed 's/#\+/\"\nmsgstr \"translation\"\n\ /g' | sed 's/ "/"/' | sed 's/ "/"/' | sed 's/ msgid/msgid/' | sed 's/msgid\"/msgid "/'  >> source.titan/po/en/LC_MESSAGES/titan.po
#cat source.titan/skin/default/skin.xml source.titan/titan/plugins/mc/skin.xml source.titan/titan/plugins/aafpanel/skin.xml source.titan/titan/plugins/networkbrowser/skin.xml | sed 's/text="/text=/' | sed 's/text=/\ntext=!/' | grep text= | sed 's/bordercol=/\n/' | sed 's/valign=/\n/'  |sed 's/posy=/\n/' | sed 's/parent=/\n/' | sed 's/halign=/\n/' | sed 's/type=/\n/' | sed 's/name=/\n/' | sed 's/func=/\n/'| sed 's/progresscol=/\n/'| grep ^text= | sed "s#\".*##" | sort -u | sed 's/text=!/msgid "/' | tr '\n' '#' | sed 's/#\+/\"\nmsgstr \"translation\"\n\ /g' | sed 's/ "/"/' | sed 's/ "/"/' | sed 's/ msgid/msgid/' >> source.titan/po/en/LC_MESSAGES/titan.po
#####
echo msgfmt -v $HOME/flashimg/source.titan/po/en/LC_MESSAGES/titan.po_auto.po -o $HOME/flashimg/source.titan/po/en/LC_MESSAGES/titan.mo
msgfmt -v $HOME/flashimg/source.titan/po/en/LC_MESSAGES/titan.po_auto.po -o $HOME/flashimg/source.titan/po/en/LC_MESSAGES/titan.mo
