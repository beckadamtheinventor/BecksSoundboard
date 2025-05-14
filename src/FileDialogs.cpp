#include <cstring>
#include <filesystem>
#include <imgui.h>
#include "FileDialogs.hpp"

#ifdef WIN32
#include <windef.h>
#include <winnt.h>
#include <winbase.h>
#endif

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
    return found;
}

bool OpenFileDialog(std::string title, std::filesystem::path& path, std::filesystem::path& selected, bool* is_open) {
    static std::vector<std::filesystem::path> folders;
    static std::vector<std::filesystem::path> files;
    static bool needs_dirlist = true;
    bool clicked = false;
    if (needs_dirlist) {
        needs_dirlist = false;
        folders.clear();
        files.clear();
        folders = DirList(path, true);
        files = DirList(path, false);
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

    for (auto folder : folders) {
        if (ImGui::Button(folder.filename().string().c_str())) {
            path = folder;
            needs_dirlist = true;
        }
    }

    ImGui::Text("Files");

    for (auto file : files) {
        if (ImGui::Button(file.filename().string().c_str())) {
            selected = file;
            needs_dirlist = true;
            clicked = true;
        }
    }
    ImGui::End();
    return clicked;
}
