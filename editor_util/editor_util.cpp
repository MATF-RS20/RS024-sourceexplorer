#include "editor_util.hpp"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <array>
#include "../imgui_util/misc/cpp/imgui_stdlib.h"

namespace fs = std::filesystem;

void save(const char* new_filename, const std::string& buffer)
{
    std::ofstream outfile(new_filename, std::fstream::out);
    if(!outfile.is_open())
    {
        std::cerr << "Failed to open file\n";
        exit(1);
    }
    outfile << buffer;

    outfile.close();   
}

std::vector<std::string> get_directory_files(const std::string& pathname)
{   
    std::string path(pathname);
    std::vector<std::string> res;
    res.push_back("<= BACK");
    for(auto& entry : fs::directory_iterator(pathname))
    {   
        path = fs::canonical(entry).string();
        if(fs::is_directory(path))
        {
            res.push_back(fs::canonical(path));
        }
    
        // filter extension .cpp .hpp .h
        else
        {
            if(fs::path(path).extension() == ".cpp" || fs::path(path).extension() == ".hpp" || fs::path(path).extension() == ".h"
                    || fs::path(path).extension() == ".cc" || fs::path(path).extension() == ".c")
                res.push_back(path);
        }
    }

    std::sort(std::begin(res)+1, std::end(res), [](const auto entry1, const auto entry2){
        if(fs::is_directory(entry1))
        {
            if(!fs::is_directory(entry2))
                return true;
            return entry1.compare(entry2) <= 0;
        }
        else if(fs::is_directory(entry2))
        {
            return false;
        }
        else return entry1.compare(entry2) <= 0; 
        
    });

    return res;
}

bool warning = false;
std::string new_name = "";

void draw_filebrowser(const char* action, std::string& filename, bool& write, bool& is_clicked_OPEN)
{
    static std::string error_msg = "";
    ImGui::SetNextWindowSize(ImVec2(500, 400));
    
    if(ImGui::Begin(action, &is_clicked_OPEN))
    {
        if(!fs::is_directory(filename))
        {
            size_t pos = filename.find_last_of("/");
            if(pos == filename.npos)
                pos = 0;
            else
                pos++;
            std::string name = filename.substr(pos);
            new_name = name;
            filename = fs::canonical(filename).parent_path();
        }

        ImGui::Text("[D] %s\n\n", filename.c_str());
        std::vector<std::string> files = get_directory_files(filename);
        
        for(auto& file : files)
        {
            // getting name of file
            size_t pos = file.find_last_of("/");
            pos = pos == file.npos ? 0 : pos + 1;
            
            std::string name = file.substr(pos);
            if(ImGui::Selectable(name.c_str()))
            {
                if(file == "<= BACK")
                    filename = fs::canonical(filename).parent_path();
                else    
                    filename = fs::canonical(fs::path(file));
                if(fs::is_regular_file(filename))
                {
                    new_name = name;
                    filename = fs::canonical(filename).parent_path();
                }
            }
        }
        
        ImGui::InputText("###input_filename", &new_name);
        ImGui::SameLine();
        ImGui::Text("(*.cpp, *.hpp, *.h)"); //.cpp, hpp, .h, .cc, .c
        ImGui::Separator();

        if(warning)
        {
            ImGui::TextColored(ImVec4(218.f/255.f, 10.f/255.f, 10.f/255.f, 1.f), "%s", error_msg.c_str());
        }

        if(ImGui::Button("OK"))
        {
            if(fs::is_directory(filename))
            {
                if(new_name == "")
                {
                    warning = true;
                    error_msg = "Please enter file name\n";
                }
                else
                {
                    filename.append("/");
                    filename.append(new_name);
                    is_clicked_OPEN = false;
                    write = true;
                    warning = false;
                }
                
            }
            else
            {
                warning = true;
                error_msg = "File already exist!";
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
        {
            is_clicked_OPEN = false;
        }
        

        ImGui::End();
    }
}
