@echo off

:xsi5
candle commandlinetools.wxs
light -out OgreCommandLineTools.msi commandlinetools.wixobj %WIX_ROOT%\ui\wixui_featuretree.wixlib


:end

