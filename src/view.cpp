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
#include <fstream>
#include <stdint.h>
// #include <stdio.h>
#include <cstdio>
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

struct ParsedFile {
    std::vector<unsigned char> rgba_bytes;
    size_t rows_count;
};


class BufferedFile {
    private: 
        FILE* _fileHandle;
        std::string _out_tmp;
        int _line_number = -1;
    
        void trim(std::string *string){
            if (string->length() == 0) {
                return;   
            }

            // lhs
            int count = 0;
            for (int i = 0; i < string->size(); i++) {
                char c = string->at(i);
                if (c != '\n' && c != '\r' && c != '\t' && c !=' ') {
                    break;
                }

                count ++;
            }
            string->erase(0, count);

            // rhs
            char end = string->at(string->size() -1);
            if (end == '\n' || end == '\r' || end == 't' || end == ' ') {
                std::cerr << "'" << *string << "'\n";
                TODO();
            }
        }
    public: 
        BufferedFile(FILE* file) {
            _fileHandle = file;
        }

        size_t lineNumber() {
            return _line_number;
        }

        std::string* readStrippedLine() {
            _out_tmp.clear();
            for (int i = 0; true; i++) {
                int c = getc(_fileHandle);

                if (c == EOF) {
                    break;
                }
                
                // break on newline - \n or \r\n
                if (c == '\r') {
                    c = getc(_fileHandle);
                    if(c != '\n') {
                        TODO();
                    }
                }

                if (c == '\n') {
                    break;
                }

                _out_tmp.push_back((char)c);
            }

            trim(&_out_tmp);

            _line_number++;
            return &_out_tmp;
        }
};

bool expectBeginImage(BufferedFile* cursor) {
    std::string* line = cursor->readStrippedLine();
    return (line->compare("begin Image") == 0);
    
}

// bool expectBeginRow(BufferedFile* cursor) {
//     std::string* line = cursor->readStrippedLine();
//     return (line->compare("begin Row") == 0);
    
// }

// bool expectBeginPixel(BufferedFile* cursor) {
//     std::string* line = cursor->readStrippedLine();
//     std::cerr << "'" << *line << "'\n";
//     return (line->compare("begin Pixel") == 0);   
// }



// bool expectEndPixel(BufferedFile* cursor) {
//     std::string* line = cursor->readStrippedLine();
//     return (line->compare("end Pixel") == 0);
    
// }

// bool expectEndRow (BufferedFile* cursor) {
//     std::string* line = cursor->readStrippedLine();
//     return (line->compare("end Row") == 0);
// }

#define STRNCMP_LEN(string1, literal) strncmp(string1, literal, strlen(literal))

std::pair<unsigned char, int> parseByte(std::string* line, int number_start_offset) {
    // get length of number
    int number_len = 0;
    for (auto line_iter = line->begin()+number_start_offset; line_iter != line->end(); line_iter++) {
        if (isdigit(*line_iter) == 0) {
            break;
        }
        number_len ++ ;
    }

    int number_end_offset = number_start_offset + number_len;

    // parse the number
    auto number_str = std::string();
    for (int i = number_start_offset; i < number_end_offset; i++) {
        number_str.push_back(line->at(i));
    }

    unsigned char number = atoi(number_str.c_str());
    
    return std::pair(number, number_len);
}
std::pair<bool, unsigned char> expectSetRed(std::string* line) {
    char* leading = "set Red = ";
    if (STRNCMP_LEN(line->c_str(), leading) !=0) {
        return std::pair(false, 0);
    }
    int number_start_offset = 0 + strlen(leading);

    std::pair byte_result = parseByte(line, number_start_offset);

    int number_end_offset = byte_result.second + number_start_offset;
    unsigned char number = byte_result.first;

    // check for a trailing semicolon
    if (line->at(number_end_offset) != ';') {
        return std::pair(false, 0);
    }

    // check entire string is consumed
    if ((number_end_offset + 1) < (line->size())) {
        return std::pair(false, 0);    
    }

    return std::pair(true, number);
}
std::pair<bool, unsigned char> expectSetBlue(std::string* line) {
    char* leading = "set Blue = ";
    if (STRNCMP_LEN(line->c_str(), leading) !=0) {
        return std::pair(false, 0);
    }
    int number_start_offset = 0 + strlen(leading);

    std::pair byte_result = parseByte(line, number_start_offset);

    int number_end_offset = byte_result.second + number_start_offset;
    unsigned char number = byte_result.first;

    // check for a trailing semicolon
    if (line->at(number_end_offset) != ';') {
        return std::pair(false, 0);
    }

    // check entire string is consumed
    if ((number_end_offset + 1) < (line->size())) {
        return std::pair(false, 0);    
    }

    return std::pair(true, number);
}

std::pair<bool, unsigned char> expectSetGreen(std::string* line) {
    char* leading = "set Green = ";
    if (STRNCMP_LEN(line->c_str(), leading) !=0) {
        return std::pair(false, 0);
    }
    int number_start_offset = 0 + strlen(leading);

    std::pair byte_result = parseByte(line, number_start_offset);

    int number_end_offset = byte_result.second + number_start_offset;
    unsigned char number = byte_result.first;

    // check for a trailing semicolon
    if (line->at(number_end_offset) != ';') {
        return std::pair(false, 0);
    }

    // check entire string is consumed
    if ((number_end_offset + 1) < (line->size())) {
        return std::pair(false, 0);    
    }

    return std::pair(true, number);
}

#define ASSERT_PARSE(expression) assertParseImpl((expression)==1, #expression)
void assertParseImpl(bool condition, const char* expression) {
    if (!condition) {
        std::cerr << "ASSERT_PARSE failed for '" << expression << "'\n";
        exit(1);
    }
}

typedef struct RBG {
    unsigned char red;
    unsigned char blue;
    unsigned char green;
};


// TWO possible return stats:
// - a valid RBG and a NULL stirng pointer
// - a default RBG and a string with the line read
std::pair<RBG, std::string*> parsePixel(BufferedFile* file) {
    auto line3 = file->readStrippedLine();
    if (strcmp(line3->c_str(), "begin Pixel")!=0) {
        return std::pair(RBG{}, line3); 
    }

    std::cerr << file->lineNumber() << "\n";
    std::string * red_line = file->readStrippedLine();
    if (STRNCMP_LEN(red_line->c_str(), "set Red =") == 0) {
        std::pair result = expectSetRed(red_line);
        if (!result.first) {
            TODO();
        }
    } else {
        TODO();
    }

    std::string* blue_line = file->readStrippedLine();
    if (STRNCMP_LEN(blue_line->c_str(), "set Blue =") == 0) {
        std::pair result = expectSetBlue(blue_line);
        if (!result.first) {
            TODO();
        }
    } else {
        TODO();
    }

    std::string* green_line = file->readStrippedLine();
    if (STRNCMP_LEN(green_line->c_str(), "set Green =") == 0) {
        std::pair result = expectSetGreen(green_line);
        if (!result.first) {
            TODO();
        }
    } else {
        TODO();
    }

    if (strcmp(file->readStrippedLine()->c_str(), "end Pixel")!=0) {
        TODO();
    } else {
        std::cerr << "end Pixel\n";
        auto pair = std::pair(RBG{}, nullptr);
        return pair;
    }

    // unreachable
    TODO();
    auto pair = std::pair(RBG{}, nullptr);
    return pair;
}

ParsedFile parseFile(const char* file_path) {
    std::cerr << file_path << "\n";
    
    // auto file_path2 = std::string(file_path);
    FILE* f = std::fopen(file_path, "r");
    if (f == nullptr) {
        std::cerr << "Failed to open file '" << file_path << "' for viewing\n";
        exit(1);
    }
    std::cerr << "opened file\n";

    auto file = BufferedFile(f);

    int rows = 0;
    
    std::string* line0 = file.readStrippedLine();
    if (strcmp(line0->c_str(), "begin Image") != 0) {
        std::cerr << line0 << "\n";
        TODO();
    } 

    for (;;) {
        // rows
        auto row = file.readStrippedLine();
        std::string leftover;
        if (strcmp(row->c_str(), "begin Row") == 0) {
            // pixels in row
            for (;;) {
                
                // parse pixel
                auto data = parsePixel(&file);
                if (data.second != nullptr) {
                    leftover = *data.second;
                    break;
                }
                std::cerr << "Begin Pixel" << "\n";
                // std::cerr << "" data.first << "\n";
                std::cerr << "end Pixel\n";
            }
            
            if (STRNCMP_LEN(leftover.c_str(), "end Row") == 0) {
                continue;
            } else {
                TODO();
            }

            
        } else {
            TODO();
        }

    }

    std::string* line5 = file.readStrippedLine();
    if (strcmp(line5->c_str(), "end Image") != 0) {
        TODO();
    }


    TODO();
    // for (;;) {        
        
    //     const char* header = file.readStrippedLine();
    //     if (header == "begin Pixel") {
    //         // ASSERT_PARSE(expectBeginPixel(&file));
    //         auto pair = expectSetRed(&file);
    //         unsigned char red = pair.second;
    //         ASSERT_PARSE(pair.first == true);

            
    //         auto pair2 = expectSetBlue(&file);
    //         unsigned char blue = pair2.second;
    //         ASSERT_PARSE(pair2.first == true);

    //         auto pair3 = expectSetGreen(&file);
    //         unsigned char green = pair3.second;
    //         ASSERT_PARSE(pair3.first == true);
    //         // ASSERT_PARSE(expectEndPixel(&file));
    //     } else if (header == "end Pixel")  {

    //     } else if (header == "begin Row") {
    //         rows ++;
    //     } else if (header == "end Row") {

    //     }
    // }



    /** 
        ParsedFile parsed {};
        
        TODO();
        std::string* file_contents = read_file_to_string(file_path);
        if (file_contents == nullptr) {
            std::cerr << "Failed to open the file '" << file_path << "' ";
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

        TODO();
        parsed.rgba_bytes = data;
        parsed.rows_count = rows;

        //todo
        return parsed;
    
    **/

    TODO();

    // fclose(); xmake build view && xmake run view smiley.
    return ParsedFile{};
}

const int MIN_WINDOW_WIDTH = 800;
const int MIN_WINDOW_HEIGHT = 600;

int main(int argc, const char* argv[]) {    
    // create a raylib context
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT, "silly image format");    

    BeginDrawing();
    {
        DrawText("Loading image...", 10, 10, 20, RAYWHITE);
    }
    EndDrawing();

    std::vector<const char*> args = collectArgs(argc, argv);
    if (args.size() != 2) {
        if (args.size() > 2) {
            std::cerr << "too many arguments\n";
        }
        exitWithUsage();
    }
    const char* infile_path = args.at(1);
    std::cerr << infile_path << "\n";

    ParsedFile parsed = parseFile(infile_path);

    // calculate width and height
    int pixels_count = (parsed.rgba_bytes.size() / sizeof(Color));
    int width = pixels_count / parsed.rows_count;
    int height = parsed.rows_count;
    
    // update window size
    if ((width > MIN_WINDOW_WIDTH) || (height > MIN_WINDOW_HEIGHT)) {
        SetWindowSize(std::max(width, MIN_WINDOW_WIDTH), std::max(height, MIN_WINDOW_HEIGHT));
    }

    // create a texture
    Color* image_colours = (Color*)parsed.rgba_bytes.data();
    Texture2D texture = genTexture2d(width, height);
    UpdateTexture(texture, image_colours);

    // create a camera
    Camera2D camera = {0};
    camera.offset = Vector2{0, 0};
    camera.rotation = 0;
    camera.target = Vector2{0,0};
    camera.zoom = WINDOW_RELATIVE_SIZE * CAMERA_ZOOM_RATIO;

    Color faint_gray = { 130, 130, 130, 80};

    auto zoom_format_buffer = std::string();
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