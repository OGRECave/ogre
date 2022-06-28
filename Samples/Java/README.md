# Build instructions

1. copy `lib/OgreJNI.dll` and the contents of `share/java/` to the `bin/` directory
2. copy `Example.java` to the `bin/` directory
3. compile the sample as
    ```
    javac Example.java -classpath Ogre-*.jar
    ```
4. Run the sample as
    ```
    java -classpath .:Ogre-1.12.12.jar -Djava.library.path=. Example
    ```