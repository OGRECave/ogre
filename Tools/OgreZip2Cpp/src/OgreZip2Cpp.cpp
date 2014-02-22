/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

// this tool helps to embed zip files in programs.
// if convert the zip file to a cpp file that should be added to the program build.
// read more about it on this forum thread: http://www.ogre3d.org/forums/viewtopic.php?f=4&t=62181



// _CRT_SECURE_NO_WARNINGS is defined to disable this compiler warning:
// "
//  warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. 
// To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
// "
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sys/stat.h>
#include <string>


void help(void)
{
    // Print help message
    printf("OgreZip2Cpp: Convert a zip file into a cpp file.\n");
    printf("Usage: OgreFile2Cpp [zip file name without extension (without .zip)]\n");
    printf("The working folder needs to be the same as the one of the zip file.\n");
}

// the main function
int main(int numargs, char** args)
{
    // check that we have only one parameter - the zip file name without extension 
    if (numargs != 2)
    {
        printf("Error: Wrong number of parameters (the program uses one parameter)\n");
        help();
        return -1;
    }
    std::string fileNameWithoutExt = args[1];
    std::string zipFileName = fileNameWithoutExt + ".zip";
    std::string cppFileName = zipFileName + ".cpp";
    std::string structName = fileNameWithoutExt;


    // get the zip file size
    std::streamsize fileSize = 0;
    struct stat results;
    if (stat(zipFileName.c_str(), &results) == 0)
        // The size of the file in bytes is in
        // results.st_size
    {
        fileSize = results.st_size;
    }

    // check that the file is valid
    if (fileSize == 0)
    {
        printf("Error: Zip file was not found or its size is zero (make sure that you didn't add .zip at the end of the parameter)\n");
        help();
        return -2;
    }
    
    // alloc memory for the in(zip) file - loading it all to memory is much fater then
    // one char at a time
    unsigned char * inFileData = new unsigned char[fileSize];

    // open, read the data to memory and close the in file
    std::ifstream roStream;
    roStream.open(zipFileName.c_str(),  std::ifstream::in | std::ifstream::binary);
    roStream.read((char *)inFileData, fileSize);
    roStream.close();

    // alloc the out file, the out file is a text file, meaning it is much bigger then the input file
    // allocating a buffer ten time bigger then the size of the in file - just to be on the safe side.
    // waste of memory, yes, but the most easy way to be safe without writing lots of code 
    char * outFileData = new char[fileSize * 10];
    char * outFileDataPos = outFileData;
    

    // start writing out the out cpp file 
    // ----------------------------------

    // add the include
    outFileDataPos+= sprintf(outFileDataPos, "#include \"OgreZip.h\"\n\n");
    
    // start the names place
    outFileDataPos+= sprintf(outFileDataPos, "namespace Ogre {\n");
    
    // declare the struct 
    outFileDataPos+= sprintf(outFileDataPos, "struct %s{\n", structName.c_str());
    
    // add a data member for the file name
    outFileDataPos+= sprintf(outFileDataPos, "\tconst char * fileName;\n");

    //  the struct contractor
    outFileDataPos+= sprintf(outFileDataPos, "\t%s() : fileName(\"%s\")\n", structName.c_str(), zipFileName.c_str());
    outFileDataPos+= sprintf(outFileDataPos, "\t{\n");

    // declare and init the content of the zip file to a static buffer
    outFileDataPos+= sprintf(outFileDataPos, "\t\tstatic uint8 fileData[] = \n");
    outFileDataPos+= sprintf(outFileDataPos, "\t\t{\n\t\t\t");    
    int posCurOutInLine = 0;
    for( std::streamsize i = 0 ;  i < fileSize ; i++ ) 
    {                
        // get the current char        
        unsigned char outChar = inFileData[i];
        
        // if you want to encrypt the data - encrypt the char here
        // ....

        // play with the formatting so the data will look nice
        // add spaces before small number
        if(outChar < 10) 
            outFileDataPos+= sprintf(outFileDataPos, " ");
        if(outChar < 100)
            outFileDataPos+= sprintf(outFileDataPos, " ");

        // output the char to the out cpp file
        outFileDataPos+= sprintf(outFileDataPos, "%d", outChar);
        posCurOutInLine+=3;

        // add a comman after every char but the last
        if(i + 1 != fileSize) {
            outFileDataPos+= sprintf(outFileDataPos, ", ");
            posCurOutInLine+=2;
        }

        // cut the line every 100 chars
        if (posCurOutInLine > 100) {
            outFileDataPos+= sprintf(outFileDataPos, "\n\t\t\t");
            posCurOutInLine = 0;
        }
    }

    // close the static buffer that holds the file data
    outFileDataPos+= sprintf(outFileDataPos, "\n\t\t};\n");

    // register the file buffer to tEmbeddedZipArchiveFactory
    outFileDataPos+= sprintf(outFileDataPos, "\t\tEmbeddedZipArchiveFactory::addEmbbeddedFile(fileName, fileData, sizeof(fileData), NULL);\n");

    // close the contractor 
    outFileDataPos+= sprintf(outFileDataPos, "}\n");

// This code may be risky in some case and is not relevant in most case for this tool so I am leaving it out. 
//     // the distractor
//  outFileDataPos+= sprintf(outFileDataPos, "\t~%s()\n", structName.c_str());
//  outFileDataPos+= sprintf(outFileDataPos, "\t{\n");
//     // remove the file from tEmbeddedZipArchiveFactory in the distractor (for the case of dynamic plugin unload?)
//  outFileDataPos+= sprintf(outFileDataPos, "\t\tEmbeddedZipArchiveFactory::removeEmbbeddedFile(fileName);\n");
//  outFileDataPos+= sprintf(outFileDataPos, "\t}\n");

    // close the class
    outFileDataPos+= sprintf(outFileDataPos, "};\n");
    outFileDataPos+= sprintf(outFileDataPos, "\n");

    // declare a global (static) variable so the struct contractor will be called when the program starts
    // or the plugin loads.
    outFileDataPos+= sprintf(outFileDataPos, "%s g_%s;\n", structName.c_str(), structName.c_str());
    outFileDataPos+= sprintf(outFileDataPos, "\n");

    // close the Ogre name space
    outFileDataPos+= sprintf(outFileDataPos, "}\n");

    // write out the cpp file
    std::ofstream writeStream;
    writeStream.open(cppFileName.c_str(),  std::ifstream::out | std::ofstream::binary);
    writeStream.write(outFileData, ((std::streamsize)(outFileDataPos - outFileData)));
    writeStream.close();

    // done, let go home and drink some beers
    return 0;
}

