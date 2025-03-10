#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <optional>
#include "common.hpp"
#include <fstream>
#include <regex>

std::string* read_file_to_string(const char* file_path) {
   
    FILE* file = fopen(file_path, "r");

    // get length
    fseek(file, 0, SEEK_END);
    int infile_len = ftell(file);
    rewind(file);

    // write to string
    auto file_string = new std::string(infile_len, '\0');
    auto file_data = file_string->data();
    fread(file_data, 1, infile_len, file);

    return file_string;
}

Texture2D genTexture2d(int width, int height) {
    auto image = GenImageColor(width, height, WHITE);
    auto texture = LoadTextureFromImage(image);
    UnloadImage(image);

    return texture;
}

int main(int argc, char* argv[]) {
    std::vector<const char*> args = collectArgs(argc, argv);

    if (args.size() != 2) {
        exitWithUsage();
    }
    const char* infile_path = args.at(1);
    std::string* file_contents = read_file_to_string(infile_path);

    // fetch all integers
    auto regex = new std::regex ("\\d+", std::regex_constants::ECMAScript | std::regex_constants::icase);
    auto iter_begin = new std::sregex_iterator(file_contents->begin(), file_contents->end(), *regex);
    auto iter_end = std::sregex_iterator();
    std::cout << "Found " << std::distance(*iter_begin, iter_end) << " words:\n";
    
    // recover the image bytes from the red blue and green values
    std::vector<unsigned char> data;
    int i = 0;
    for (auto iter = *iter_begin; iter != iter_end; iter ++) {
        auto match = *iter;
        auto match_str = match.str();

        auto c = (unsigned char) atoi(match_str.c_str());
        data.push_back(c);
        
        if (i % 3 == 0) {
            // add back in alpha
            char empty = 255;
            data.push_back(empty);
        }
        i++;
    }
    
    // swap the blue and green channels
    // red, blue green -> red green blue
    for (int index = 0; index < data.size(); index+=4) {
        Color* color = (Color*)&data.at(index);
        Color tmp = *color;
        color->g = tmp.b;
        color->b = tmp.g;
    }
    

    
    // count the rows
    auto regex_rows = new std::regex("begin Row", std::regex_constants::ECMAScript | std::regex_constants::icase);
    auto rows_begin = new std::sregex_iterator(file_contents->begin(), file_contents->end(), *regex_rows);
    auto rows_end = std::sregex_iterator();

    int rows = 0;
    for (auto iter = *rows_begin; iter != rows_end; iter ++) {
        rows ++;
    }

    // cleanup memory
    delete file_contents;
    delete iter_begin;
    delete regex;
    delete rows_begin;
    delete regex_rows;
    
    // calculate width and height
    int width = (data.size()/4) / rows;
    int height = rows;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(std::max(width, 100), std::max(height, 100), "silly image format");
    
    // create a texture
    Texture2D texture = genTexture2d(width, height);
    UpdateTexture(texture, data.data());



    std::cout << "meow\n";
    bool first_iter = true;
    while (!WindowShouldClose())
    {
        if (first_iter == true){
            // draw
            BeginDrawing();
            {
            

                ClearBackground(GRAY);
                DrawTexture(texture, 0, 0, WHITE);

                DrawFPS(10, 10);

    
            }
            EndDrawing();
        }
        
        first_iter = false;
    }

    CloseWindow();
    


    return 0;
}