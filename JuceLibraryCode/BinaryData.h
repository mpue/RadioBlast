/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   Fader_png;
    const int            Fader_pngSize = 19460;

    extern const char*   Knob_64_png;
    const int            Knob_64_pngSize = 495933;

    extern const char*   splash_png;
    const int            splash_pngSize = 394797;

    extern const char*   RadioBlast_ico;
    const int            RadioBlast_icoSize = 38637;

    extern const char*   RadioBlast_png;
    const int            RadioBlast_pngSize = 716976;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 5;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
