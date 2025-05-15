#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::string NarrowString16To8(std::wstring w);
std::wstring ExpandString8To16(std::string s);
void AddPinnedFolder(std::filesystem::path p);
std::vector<std::filesystem::path> GetPinnedFolders();
std::vector<std::filesystem::path> DirList(std::filesystem::path path, bool folders=false);
bool OpenFileDialog(std::string title, std::filesystem::path& path, std::filesystem::path& selected, bool* is_open=nullptr);
