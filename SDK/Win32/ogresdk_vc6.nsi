!define TARGET_COMPILER_DESCRIPTION "Visual C++ 6"
!define TARGET_COMPILER "VC6"
!include ogresdk.nsh

Section -Samples
  ; We assume copysamples.sh has been run recently enough for these files to be available
  SetOutPath "$INSTDIR\samples\scripts"
  SetOverwrite try
  File ".\Samples\scripts\*.dsp"
  SetOutPath "$INSTDIR\samples\src"
  SetOverwrite try
  File ".\Samples\src\*.cpp"
  SetOutPath "$INSTDIR\samples\include"
  SetOverwrite try
  File ".\Samples\include\*.h"

  ; Refapp
  SetOutPath "$INSTDIR\samples\refapp\scripts"
  SetOverwrite try
  File ".\samples\refapp\scripts\*.dsp"
  SetOutPath "$INSTDIR\samples\refapp\src"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\src\*.cpp"
  SetOutPath "$INSTDIR\samples\refapp\include"
  SetOverwrite try
  File "..\..\ReferenceApplication\ReferenceAppLayer\include\*.h"


  SetOutPath "$INSTDIR\samples"
  SetOverwrite try
  File ".\Samples\Samples.dsw"
  File ".\Samples\Samples.opt"


SectionEnd