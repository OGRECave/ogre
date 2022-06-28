# Build instructions

1. copy the contents of `lib/cli/ogre-sharp-x.y.z/` to the `bin/` directory
2. copy `example.cs` to the `bin/` directory
3. compile the sample as
    ```
    C:/Windows/Microsoft.NET/Framework64/x.y.z/csc.exe example.cs -r:Ogre.dll
    ```
    On Linux, you can the `mcs` mono compiler instead.
4. Run the sample by double clicking `example.exe` or on linux, running:
    ```
    mono example.exe
    ```