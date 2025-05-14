#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::filesystem::path> DirList(std::filesystem::path path, bool folders=false);
bool OpenFileDialog(std::string title, std::filesystem::path& path, std::filesystem::path& selected, bool* is_open=nullptr);
