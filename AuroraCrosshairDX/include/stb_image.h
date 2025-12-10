// This is a simplified version - for production, use the actual stb_image.h from:
// https://github.com/nothings/stb/blob/master/stb_image.h

#pragma once

#include <cstring>

/*
PLACE THE ACTUAL stb_image.h HERE

To use the real stb_image:
1. Download from: https://github.com/nothings/stb/blob/master/stb_image.h
2. Replace this placeholder file
3. In one source file, add:
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"
*/

// Placeholder function signatures
extern "C"
{
    unsigned char* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp);
    void stbi_image_free(void* retval_from_stbi_load);
}
