!define TARGET_COMPILER_DESCRIPTION "C::B + MingW + STLPort"
!define TARGET_COMPILER "CBMingW_STLP"
!define MINGW
!define STLPORT
!include ogresdk.nsh

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*_stlp.cbp"
  SetOutPath "$INSTDIR\samples\src"
  SetOverwrite try
  File ".\Samples\src\*.cpp"
  SetOutPath "$INSTDIR\samples\include"
  SetOverwrite try
  File ".\Samples\include\*.h"

  ; Refapp
  SetOutPath "$INSTDIR\samples\refapp\scripts"
  SetOverwrite try
  File ".\samples\refapp\scripts\*_stlp.cbp"
  SetOutPath "$INSTDIR\samples\refapp\src"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\src\*.cpp"
  SetOutPath "$INSTDIR\samples\refapp\include"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\include\*.h"
  
  
  SetOutPath "$INSTDIR\samples"
  SetOverwrite try
  File ".\Samples\Samples_stlp.workspace"


SectionEnd

Section -STLPort
  SetOutPath "$INSTDIR\stlport\stlport"
  SetOverwrite try
  File /r ".\stlport\stlport\*.*"
  
  SetOutPath "$INSTDIR\bin\debug"
  SetOverwrite try
  File ".\stlport\bin\libstlportstlg.5.0.dll"

  SetOutPath "$INSTDIR\bin\release"
  SetOverwrite try
  File ".\stlport\bin\libstlport.5.0.dll"
  
SectionEnd