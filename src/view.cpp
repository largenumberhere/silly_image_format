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
#include <format>

// returns null on failure
std::string* read_file_to_string(const char* file_path) {
   
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        return nullptr;
    }

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


static float WINDOW_RELATIVE_SIZE = 1;
static float CAMERA_ZOOM_RATIO = 1;

float screen_ratio(int current_size, int default_size) {
    return (float)current_size / (float)default_size;
}


int main(int argc, char* argv[]) {
    const int MIN_WIDTH = 800;
    const int MIN_HEIGHT = 600;

    // create a raylib context
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(MIN_WIDTH, MIN_HEIGHT, "silly image format");    
    BeginDrawing();
    {
        DrawText("Loading image...", 10, 10, 20, RAYWHITE);
    }
    EndDrawing();

    std::vector<const char*> args = collectArgs(argc, argv);
    if (args.size() != 2) {
        exitWithUsage();
    }

    const char* infile_path = args.at(1);
    std::string* file_contents = read_file_to_string(infile_path);
    if (file_contents == nullptr) {
        std::cerr << "Failed to open the file '" << infile_path << "' ";
        perror("");
        exit(1);
    }

    // fetch all integers from file
    auto regex = new std::regex ("\\d+", std::regex_constants::ECMAScript | std::regex_constants::icase);
    auto iter_begin = new std::sregex_iterator(file_contents->begin(), file_contents->end(), *regex);
    auto iter_end = std::sregex_iterator();
    
    // recover the image bytes from the red blue and green values
    std::vector<unsigned char> data;
    int i = 0;
    for (auto iter = *iter_begin; iter != iter_end; iter ++) {
        auto match = *iter;
        auto match_str = match.str();

        auto c = (unsigned char) atoi(match_str.c_str());
        data.push_back(c);
        
        i++;
        if (i == 3) {
            // add back in alpha
            char empty = 255;
            data.push_back(empty);
            i = 0;
        }
    }

    // free the regex
    delete iter_begin;
    delete regex;
    
    // swap the blue and green channels due to the format having a different order
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

    // file conversion is complete
    Color* image_colours = (Color*)data.data();

    // cleanup memory
    delete rows_begin;
    delete regex_rows;
    delete file_contents;
    
    // calculate width and height
    int width = (data.size()/4) / rows;
    int height = rows;

    // update window size
    if ((width > MIN_WIDTH) || (height > MIN_HEIGHT)) {
        SetWindowSize(std::max(width, MIN_WIDTH), std::max(height, MIN_HEIGHT));
    }

    // create a texture
    Texture2D texture = genTexture2d(width, height);
    UpdateTexture(texture, image_colours);

    // create a camera


    Camera2D camera = {0};
    camera.offset = Vector2{0, 0};
    camera.rotation = 0;
    camera.target = Vector2{0,0};

    camera.zoom = WINDOW_RELATIVE_SIZE * CAMERA_ZOOM_RATIO;

    Color faint_gray = GRAY;
    faint_gray.a = 50;

    std::string zoom_format_buffer;
    double prev_key_time = GetTime();
    bool first_run = true;
    while (!WindowShouldClose())
    {
        // update camera if window resized
        if (IsWindowResized() || first_run) { 
            int new_h = GetScreenHeight();
            int new_w = GetScreenWidth();
            
            float ratio_h = screen_ratio(new_h, height); // (float)new_h / (float)HEIGHT;
            float ratio_w = screen_ratio(new_w, width); // (float)new_w / (float)WIDTH;
            
            // smallest ratio means window is smallest along that dimension
            
            float newRelatveSize = 0;
            if (ratio_w < ratio_h) {
                newRelatveSize = ratio_w;
            } else {
                newRelatveSize = ratio_h;
            }

            WINDOW_RELATIVE_SIZE = newRelatveSize;
            camera.zoom = WINDOW_RELATIVE_SIZE * CAMERA_ZOOM_RATIO;
        }
        
        double time = GetTime();
        if (prev_key_time + 0.1 < time) {
            prev_key_time = time;
            if (IsKeyDown('=')) {
                CAMERA_ZOOM_RATIO = CAMERA_ZOOM_RATIO * 1.1;
                camera.zoom = WINDOW_RELATIVE_SIZE * CAMERA_ZOOM_RATIO;
            } else if (IsKeyDown('-')) {
                CAMERA_ZOOM_RATIO = CAMERA_ZOOM_RATIO * 0.9;
                camera.zoom = WINDOW_RELATIVE_SIZE * CAMERA_ZOOM_RATIO;
            }
        }

        BeginDrawing();
        {   
            ClearBackground(GRAY);
            BeginMode2D(camera);
            {
                DrawTexture(texture, 0, 0, WHITE);
                
            }
            EndMode2D();

            zoom_format_buffer.clear();
            zoom_format_buffer.append(std::format("Zoom: {0}. ", CAMERA_ZOOM_RATIO));
            const char* comment = "(Use +/- to zoom)";
            int width =  std::max(MeasureText(zoom_format_buffer.c_str(), 10), MeasureText(comment, 10));

            DrawRectangle(0, 0, width+10, 30, faint_gray);
            DrawText(zoom_format_buffer.c_str(), 10, 10, 10, WHITE);
            DrawText("(Use +/- to zoom)", 10, 20, 10, WHITE);
        }
        EndDrawing();

        first_run = false;
    }

    UnloadTexture(texture);
    CloseWindow();
    
    return 0;
}
