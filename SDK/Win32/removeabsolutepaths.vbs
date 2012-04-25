FileName = WScript.Arguments(0)

' We want to replace absolute paths with ones that are relative to the current folder
set WshShell = WScript.CreateObject("WScript.Shell")
CurDir = WshShell.CurrentDirectory
OrigText = Replace(CurDir, "\", "/", 1, -1, 1)
'Read source text file
FileContents = GetFile(FileName)


'replace all string In the source file
cFileContents = Replace(FileContents, OrigText, "../..", 1, -1, 1)
dFileContents = Replace(cFileContents, CurDir, "..\..", 1, -1, 1)

' Update if different
if dFileContents <> FileContents Then WriteFile FileName, dFileContents

'Read text file
function GetFile(FileName)
	If FileName<>"" Then
		Dim FS, FileStream
		Set FS = CreateObject("Scripting.FileSystemObject")
		on error resume Next
		Set FileStream = FS.OpenTextFile(FileName)
		GetFile = FileStream.ReadAll
	End If
End Function

'Write string As a text file.
function WriteFile(FileName, Contents)
	Dim OutStream, FS

	on error resume Next
	Set FS = CreateObject("Scripting.FileSystemObject")
	Set OutStream = FS.OpenTextFile(FileName, 2, True)
	OutStream.Write Contents
End Function