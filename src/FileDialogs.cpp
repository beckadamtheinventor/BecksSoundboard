#include <algorithm>
#include <cstring>
#include <filesystem>
#include <locale>
#include <string>
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include "FileDialogs.hpp"

#ifdef WIN32
#define _AMD64_ 1
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#endif

std::vector<std::filesystem::path> listed_folders;
std::vector<std::filesystem::path> listed_files;
bool needs_dirlist = true;


// this is a probably a dumb way of doing it, but meh
std::string NarrowString16Ti8(std::wstring w) {
    std::string s;
    for (wchar_t c : w) {
        if (c < 256)
            s.push_back(c);
    }
    return s;
}

std::vector<std::filesystem::path> DirList(std::filesystem::path path, bool folders) {
    std::vector<std::filesystem::path> found;
    std::filesystem::directory_iterator iter(path);
    for (auto file : iter) {
        if (folders && file.is_directory()) {
            found.push_back(file.path());
        }
        if (!folders && file.is_regular_file()) {
            found.push_back(file.path());
        }
    }
    auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
    std::sort(found.begin(), found.end(), [&f](std::filesystem::path& a, std::filesystem::path& b) -> bool {
        std::wstring as = a.wstring();
        std::wstring bs = b.wstring();
        return std::lexicographical_compare(
            as.begin(), as.end(), bs.begin(), bs.end(), [&f](wchar_t ai, wchar_t bi) {
                return f.tolower(ai) < f.tolower(bi);
        });
    });
    return found;
}

bool OpenFileDialog(std::string title, std::filesystem::path& path, std::filesystem::path& selected, bool* is_open) {
    bool clicked = false;
    if (needs_dirlist) {
        needs_dirlist = false;
        listed_folders.clear();
        listed_files.clear();
        listed_folders = DirList(path, true);
        listed_files = DirList(path, false);
    }

    ImGui::Begin(title.c_str(), is_open);
    
#ifdef WIN32
    ImGui::Text("Drives");
    char drive_letter_buffer[512];
    size_t num_drive_letter_chars = GetLogicalDriveStringsA(sizeof(drive_letter_buffer), drive_letter_buffer);
    size_t i = 0;
    while (i < num_drive_letter_chars) {
        if (ImGui::Button(&drive_letter_buffer[i])) {
            path = std::string(&drive_letter_buffer[i]);
            needs_dirlist = true;
        }
        i += strlen(&drive_letter_buffer[i]) + 1;
    }
#endif

    ImGui::Text("%s", path.string().c_str());

    if (ImGui::Button("..")) {
        path = path.parent_path();
        needs_dirlist = true;
    }

    ImGui::Text("Folders");

    for (auto folder : listed_folders) {
        std::string str = NarrowString16Ti8(folder.filename().wstring());
        if (ImGui::Button(str.c_str())) {
            path = folder;
            needs_dirlist = true;
        }
    }

    ImGui::Text("Files");

    for (auto file : listed_files) {
        std::string str = NarrowString16Ti8(file.filename().wstring());
        if (ImGui::Button(str.c_str())) {
            selected = file;
            needs_dirlist = true;
            clicked = true;
        }
    }
    ImGui::End();
    return clicked;
}
