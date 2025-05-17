#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace FileDialogs {
    std::string NarrowString16To8(std::wstring w);
    std::wstring ExpandString8To16(std::string s);
    std::vector<std::filesystem::path> DirList(std::filesystem::path path, bool folders=false, bool recursive=false);

    class FileDialog {
        bool needs_dirlist = true;
        protected:
        std::vector<std::filesystem::path> pinned_folders;
        std::vector<std::filesystem::path> listed_folders;
        std::vector<std::filesystem::path> listed_files;
        public:
        void AddPinnedFolder(std::filesystem::path p);
        std::vector<std::filesystem::path> GetPinnedFolders();
        bool Show(std::string title, std::filesystem::path& path, std::filesystem::path& selected, bool* is_open=nullptr);
    };
}