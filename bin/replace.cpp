/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

/*
 * replace.cpp
 *
 * Replace a string from a binary file in other same-length string.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/data/string.h"
#include "xStl/data/datastream.h"
#include "xStl/stream/iostream.h"
#include "xStl/stream/file.h"
#include "xStl/utils/algorithm.h"

void printUsage()
{
    cout << "Usage: REPLACE <file> <source> <destination> [UNICODE]" << endl;
}

int main(const uint argc, const char** argv)
{
    XSTL_TRY
    {
        // Test the arguments
        if (argc < 4)
        {
            cout << "Error! Wrong number of arguments" << endl;
            printUsage();
            return -1;
        }

        bool isUnicode = false;
        cString sourceString(argv[2]);
        cString destinationString(argv[3]);

        // Test strings size
        if (sourceString.length() != destinationString.length())
        {
            cout << "Error! Strings must have similar length" << endl;
            printUsage();
            return -1;
        }

        // Read the UNICODE flag
        if (argc > 4)
        {
            if ((strcmp(argv[4], "UNICODE") == 0) && (argc == 5))
            {
                isUnicode = true;
            } else
            {
                cout << "Error! Unknown argument" << endl;
                printUsage();
                return -1;
            }
        }

        // Read the source file.
        cFile source(argv[1]);
        cStream fileContext;
        source >> fileContext;
        source.close();

        // Count the number of character to scan
        uint sCount = sourceString.length();
        uint size = sCount + 1; // Add NULL treminate character
        if (isUnicode)
        {
            sCount = sCount * 2;
            size = size * 2;
        }

        // Replace all apperences
        cStream sourceBuffer(size);
        cStream destinationBuffer(size);

        if (isUnicode)
        {
            memcpy(sourceBuffer.getBuffer(), sourceString.getBuffer(), size);
            memcpy(destinationBuffer.getBuffer(), destinationString.getBuffer(), size);
        } else
        {
            // ASCII
            cChar::covert2Ascii((char*)sourceBuffer.getBuffer(), size, sourceString.getBuffer());
            cChar::covert2Ascii((char*)destinationBuffer.getBuffer(), size, destinationString.getBuffer());
        }

        // Find and replace streams
        for (uint i = 0; i < fileContext.getSize() - sCount; i++)
        {
            // Match source string
            uint j = 0;
            for (; (j < sCount) && (sourceBuffer[j] == fileContext[j + i]); j++);
            if (j == sCount)
            {
                // Match! Replace!
                cout << "Replace " << sourceString << "  <==>  "
                                   << destinationString << "   at: " << HEXDWORD(i) << endl;
                memcpy(fileContext.getBuffer() + i,
                       destinationBuffer.getBuffer(),
                       sCount);
            }
        }

        // Write the content of the file
        cFile dest(argv[1], cFile::WRITE | cFile::CREATE);
        dest.pipeWrite(fileContext, fileContext.getSize());
        dest.close();

        // Done!
        return 0;
    }
    XSTL_CATCH (...)
    {
        cout << "Unknown exception throwed!" << endl;
        return -1;
    }
}
