bash copysamples.sh CBMINGW_STLP
pushd ..\..\Docs\src
doxygen html.cfg
cd ..\api\html
hhc index.hhp
popd
makensis ogresdk_CBMingW_STLP.nsi
pause
