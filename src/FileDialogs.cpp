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

namespace FileDialogs {
std::vector<std::filesystem::path> pinned_folders;

// create a copy of the string with only the characters that fit in 8 bits.
// this is a probably a bad way of doing it, but meh
std::string NarrowString16To8(std::wstring w) {
    std::string s;
    for (wchar_t c : w) {
        if (c < 256)
            s.push_back(c);
    }
    return s;
}

std::wstring ExpandString8To16(std::string s) {
    std::wstring w;
    for (char c : s) {
        w.push_back(c);
    }
    return w;
}

bool CanNarrowString16To8(std::wstring w) {
    for (wchar_t c : w) {
        if (c >= 256) {
            return false;
        }
    }
    return true;
}

std::vector<std::filesystem::path> DirList(std::filesystem::path path, bool folders, bool recursive) {
    std::vector<std::filesystem::path> found;
    if (recursive) {
        std::filesystem::recursive_directory_iterator iter(path);
        for (auto file : iter) {
            if (folders && file.is_directory()) {
                found.push_back(file.path());
            }
            if (!folders && file.is_regular_file()) {
                found.push_back(file.path());
            }
        }
    } else {
        std::filesystem::directory_iterator iter(path);
        for (auto file : iter) {
            if (folders && file.is_directory()) {
                found.push_back(file.path());
            }
            if (!folders && file.is_regular_file()) {
                found.push_back(file.path());
            }
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

void AddPinnedFolder(std::filesystem::path p) {
    for (auto& f : pinned_folders) {
        if (f == p) {
            return;
        }
    }
    pinned_folders.push_back(p);
}

std::vector<std::filesystem::path> GetPinnedFolders() {
    return pinned_folders;
}

bool FileDialog::Show(std::filesystem::path& selected) {
    bool clicked = false;
    if (needs_dirlist) {
        needs_dirlist = false;
        listed_folders.clear();
        listed_files.clear();
        listed_folders = DirList(path, true);
        listed_files = DirList(path, false);
    }
    bool is_open = true;
    ImGui::Begin(title.c_str(), &is_open);
    
#ifdef WIN32
    ImGui::Text("Drives");
    char drive_letter_buffer[512];
    int num_drive_letter_chars = GetLogicalDriveStringsA(sizeof(drive_letter_buffer), drive_letter_buffer);
    int j = 0;
    while (j < num_drive_letter_chars) {
        if (ImGui::Button(&drive_letter_buffer[j])) {
            path = std::string(&drive_letter_buffer[j]);
            needs_dirlist = true;
        }
        j += strlen(&drive_letter_buffer[j]) + 1;
    }
#endif

    if (pinned_folders.size() > 0) {
        int to_remove = -1;
        ImGui::PushID("Pinned");
        ImGui::Text("Pinned");
        for (int i = 0; i < pinned_folders.size(); i++) {
            ImGui::PushID(i+1+listed_folders.size()+listed_files.size());
            std::string str = NarrowString16To8(pinned_folders[i].wstring());
            if (ImGui::Button("Unpin")) {
                to_remove = i;
            }
            ImGui::SameLine();
            if (ImGui::Button("Open")) {
                path = pinned_folders[i];
                needs_dirlist = true;
            }
            ImGui::SameLine();
            ImGui::Text("%s", str.c_str());
            ImGui::PopID();
        }
        if (to_remove >= 0) {
            pinned_folders.erase(pinned_folders.begin() + to_remove);
        }
        ImGui::PopID();
    }

    ImGui::Text("%s", NarrowString16To8(path.wstring()).c_str());

    if (ImGui::Button("..")) {
        path = path.parent_path();
        needs_dirlist = true;
    }

    ImGui::Text("Folders");

    for (int i = 0; i < listed_folders.size(); i++) {
        auto& folder = listed_folders[i];
        bool can_be_loaded = CanNarrowString16To8(folder.filename().wstring());
        std::string str = NarrowString16To8(folder.filename().wstring());
        ImGui::PushID(i+1);
        if (!can_be_loaded) {
            ImGui::PushStyleColor(ImGuiCol_Text, {255, 0, 0, 255});
            ImGui::PushStyleColor(ImGuiCol_Button, {60, 60, 60, 255});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, {60, 60, 60, 255});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {60, 60, 60, 255});
        }
        if (ImGui::Button("Pin") && can_be_loaded) {
            AddPinnedFolder(folder);
        }
        ImGui::SameLine();
        if (ImGui::Button("Open") && can_be_loaded) {
            path = folder;
            needs_dirlist = true;
        }
        if (!can_be_loaded) {
            ImGui::SameLine();
            ImGui::Text("Has Unicode Characters");
            ImGui::PopStyleColor(4);
        }
        ImGui::SameLine();
        ImGui::Text("%s", str.c_str());
        ImGui::PopID();
    }

    if (saveas) {
        static char buf[512];
        ImGui::InputTextWithHint("File Name", "file.json", buf, sizeof(buf));
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            selected = std::filesystem::path(buf);
            clicked = true;
        }
    }

    ImGui::Text("Files");

    for (int i = 0; i < listed_files.size(); i++) {
        auto& file = listed_files[i];
        bool can_be_loaded = CanNarrowString16To8(file.filename().wstring());
        std::string str = NarrowString16To8(file.filename().wstring());
        ImGui::PushID(i+1+listed_folders.size());
        if (!can_be_loaded) {
            ImGui::PushStyleColor(ImGuiCol_Text, {255, 0, 0, 255});
            ImGui::PushStyleColor(ImGuiCol_Button, {60, 60, 60, 255});
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, {60, 60, 60, 255});
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {60, 60, 60, 255});
        }
        if (ImGui::Button("Open") && can_be_loaded) {
            selected = file;
            needs_dirlist = true;
            clicked = true;
        }
        if (!can_be_loaded) {
            ImGui::SameLine();
            ImGui::Text("Has Unicode Characters");
            ImGui::PopStyleColor(4);
        }
        ImGui::SameLine();
        ImGui::Text("%s", str.c_str());
        ImGui::PopID();
    }
    ImGui::End();
    if (!is_open) {
        selected.clear();
        return true;
    }
    return clicked;
}

}