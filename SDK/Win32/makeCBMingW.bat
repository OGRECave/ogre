bash copysamples.sh CBMINGW
pushd ..\..\Docs\src
doxygen html.cfg
cd ..\api\html
hhc index.hhp
popd
makensis ogresdk_CBMingW.nsi
pause
