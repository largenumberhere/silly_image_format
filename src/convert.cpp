#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <optional>
#include <cassert>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>

#include "common.hpp"

int main(int argc, char* argv[]) {
    std::vector<const char*> args = collectArgs(argc, argv);

    if (args.size() != 3) {
        exitWithUsage();
    }

    
    const char* infile_path = args.at(1);
    Image image = LoadImage(infile_path);
    
    if (image.data == NULL) {
        std::cerr << "Failed to open the input file\n";
        exit(1);
    }

    ImageFormat(&image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); // important, makes the pointer arithmetic bellow correct
    if (image.data == NULL) {
        std::cerr << "Failed change the image encoding\n";
        exit(1);
    }

    const char* outfile_path = args.at(2);
    std::ofstream outfile = std::ofstream(outfile_path);
    
    // set a large buffer for the big file IO coming up
    std::vector<char> output_buffer = std::vector<char> (4096); 
    outfile.rdbuf()->pubsetbuf(output_buffer.data(), output_buffer.size());
    
    outfile << "begin Image\n";
    for (int y = 0; y < image.height; y++) {
        // new row        
        outfile << "\tbegin Row\n";
        
        for (int x = 0; x < image.width; x++) {
            Color pixel = ((Color*)image.data)[(y * image.width) + x];
            outfile << "\t\tbegin Pixel\n";
            outfile << "\t\t\tset Red = " << (int)pixel.r << ";\n" ;
            outfile << "\t\t\tset Blue = " << (int)pixel.b << ";\n";
            outfile << "\t\t\tset Green = " << (int)pixel.g << ";\n";
            outfile << "\t\tend Pixel\n";
        }

        outfile << "\tend Row\n";
        outfile << "\n";
    }
    outfile << "end Image\n";
    
    outfile.close();

    std::cout << "Converted '" << infile_path << "' to '" << outfile_path << "'\n";

    return 0;

}