@echo off
rem copy & tweak resources.cfg
copy ..\bin\release\resources.cfg .\
sed -i -e 's/\.\.\/\.\.\/\.\.\///' resources.cfg

rem create quake3settings.cfg
echo Pak0Location: Media\packs\chiropteraDM.pk3 > quake3settings.cfg
echo Map: maps/chiropteradm.bsp >> quake3settings.cfg

candle demos.wxs
light -out OgreDemos.msi demos.wixobj %WIX_ROOT%\ui\wixui_featuretree.wixlib


:end
pause
