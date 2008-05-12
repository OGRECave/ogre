@echo off

candle milkshapeinstall.wxs
light -out OgreMilkshapeExporter.msi milkshapeinstall.wixobj %WIX_ROOT%\ui\wixui_featuretree.wixlib
goto end

:end
pause
