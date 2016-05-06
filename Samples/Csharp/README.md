# Build instructions for Mono on Ubuntu

1. inside `build/csharp/`
    ```
    mcs -target:library -out:Ogre.dll *.cs
    ```
2. copy `Ogre.dll` and `libOgre.so` to current directory
3. compile the sample
    ```
    mcs example.cs -r:Ogre.dll
    ```
4. copy `build/bin/resources.cfg` to current directory
5. to start the sample run
    ```
    mono example.exe
    ```