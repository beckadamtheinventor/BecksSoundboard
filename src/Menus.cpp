#include <fstream>
#include <random>
#include <vector>
#include <map>
#include <string>

#include "../thirdparty/raylib-5.0/src/external/miniaudio.h"
#include "../thirdparty/raylib-5.0/src/raylib.h"
#include "../thirdparty/rlImGui/rlImGui.h"
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include "../include/nlohmann/json.hpp"

#include "ConfiguredMusic.hpp"
#include "FileDialogs.hpp"
#include "ImGuiThemeFile.hpp"
#include "Menus.hpp"
#include "Hooks.hpp"
using namespace FileDialogs;


std::vector<ConfiguredMusic*> loaded_sounds;
std::map<std::string, unsigned int> loaded_sounds_by_path;
std::vector<std::string> console_window_lines;
std::vector<ma_device_info> available_playback_devices;
int selected_playback_device = -1;

FileDialogManager otherFileBrowsers;
Menus::MenuManager menuManager;
ImGui::ThemeFile global_style;
nlohmann::json sound_configs;
std::random_device random_device;
std::mt19937 random_generator(random_device());
ConfiguredMusic* current_loaded_music = nullptr;
int current_loaded_music_index = -1;
float global_volume = 1.0f;
bool play_in_sequence = false;
bool scroll_log_to_bottom = true;
bool hook_available = false;
std::map<int, std::string> sound_keybinds;

namespace Menus {

    #pragma region Helper Functions

    bool ImportSoundList(nlohmann::json j, nlohmann::json sound_configs) {
        int count = 0;
        if (!j.contains("paths")) {
            return false;
        }
        nlohmann::json json = j["paths"];
        if (!json.is_array()) {
            return false;
        }
        for (auto p : json) {
            if (p.is_string()) {
                std::string k = p.get<std::string>();
                nlohmann::json cfg;
                ConfiguredMusic* cs;
                if (sound_configs.contains(k)) {
                    cfg = sound_configs[k];
                }
                cs = ConfiguredMusic::Load(k, cfg);
                loaded_sounds.push_back(cs);
                loaded_sounds_by_path.insert(std::make_pair(k, loaded_sounds.size()-1));
                count++;
            }
        }
        TraceLog(LOG_INFO, "Imported %d sounds.", count);
        return true;
    }

    bool ImportSoundList(std::string fname, nlohmann::json sound_configs) {
        std::ifstream fd(fname);
        if (fd.is_open()) {
            nlohmann::json json;
            try {
                fd >> json;
                fd.close();
            } catch (nlohmann::detail::exception error) {
                TraceLog(LOG_ERROR, "Failed to import sounds from list \"%s\": %s", fname.c_str(), error.what());
                return false;
            }
            return ImportSoundList(json, sound_configs);
        }
        return false;
    }

    bool ExportSoundList(std::string p, std::map<std::string, unsigned int> paths) {
        std::ofstream fd(p);
        if (fd.is_open()) {
            nlohmann::json json = nlohmann::json::array();
            for (auto e : paths) {
                json.push_back(e.first);
            }
            json = {{"paths", json}};
            try {
                fd << json.dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore);
                fd.close();
                TraceLog(LOG_INFO, "Exported %d sounds.", paths.size());
                return true;
            } catch (nlohmann::detail::exception ignored) {}
        }
        TraceLog(LOG_ERROR, "Failed to export sound list \"%s\"", p.c_str());
        return false;
    }

    #pragma endregion

    #pragma region Options Menu
    void OptionsMenu::show() {
        ImGui::Begin("Options");
        ImGui::SetWindowPos({1.0f, 1.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 300.0f}, ImGuiCond_FirstUseEver);
        if (ImGui::SliderFloat("Volume", &global_volume, 0.0f, 1.0f)) {
            SetMasterVolume(global_volume);
        }
        if (ImGui::Checkbox("Play in Sequence", &play_in_sequence)) {}
        ImGui::SameLine();
        if (ImGui::Checkbox("Scroll Log to Bottom", &scroll_log_to_bottom)) {}
        if (selected_playback_device >= 0) {
            ImGui::Text("Output Device: %s", available_playback_devices[selected_playback_device].name);
        } else {
            ImGui::Text("Output Device: Default");
        }
        ImGui::Text("Available Playback Devices");
        for (int i=0; i<available_playback_devices.size(); i++) {
            auto& dev = available_playback_devices[i];
            ImGui::PushID(i);
            if (ImGui::Button("Select")) {
                CloseAudioDevice();
                InitAudioDeviceByID(&dev.id);
                selected_playback_device = i;
                SetMasterVolume(global_volume);
            }
            ImGui::SameLine();
            ImGui::Text("%s", dev.name);
            ImGui::PopID();
        }
        ImGui::End();
    }
    #pragma endregion

    #pragma region Sounds Menu
    void SoundsMenu::show() {
        ImGui::Begin("Sounds");
        ImGui::SetWindowPos({402.0f, 1.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_FirstUseEver);
        if (ImGui::Button("Import")) {
            otherFileBrowsers.openIfNotAlready("Import Sound List", [] (std::string p) {
                return ImportSoundList(p, sound_configs);
            });
        }
        ImGui::SameLine();
        if (ImGui::Button("Export")) {
            otherFileBrowsers.openIfNotAlready("Export Sound List", [] (std::string p) {
                return ExportSoundList(p, loaded_sounds_by_path);
            }, true);
        }
        ImGui::SameLine();
        static bool clear_ays = false;
        if (ImGui::Button(clear_ays ? "Are you sure?" : "Clear")) {
            if (clear_ays) {
                for (auto e : loaded_sounds) {
                    if (e != nullptr) {
                        e->Unload();
                    }
                }
                loaded_sounds.clear();
                loaded_sounds_by_path.clear();
                current_loaded_music = nullptr;
                current_loaded_music_index = -1;
                clear_ays = false;
            } else {
                clear_ays = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Import Folder")) {
            otherFileBrowsers.openIfNotAlready("Import from Folder", [] (std::string path) {
                auto files = DirList(path, false, true);
                for (auto f : files) {
                    std::string p = NarrowString16To8(f.wstring());
                    if (loaded_sounds_by_path.count(p) > 0) {
                        continue;
                    }
                    nlohmann::json cfg;
                    if (sound_configs.contains(p)) {
                        cfg = sound_configs[p];
                    }
                    ConfiguredMusic* cs;
                    if ((cs = ConfiguredMusic::Load(p, cfg))) {
                        loaded_sounds.push_back(cs);
                        loaded_sounds_by_path.insert(std::make_pair(p, loaded_sounds.size()-1));
                        TraceLog(LOG_INFO, "Loaded sound file successfuly: \"%s\"", p.c_str());
                    } else {
                        TraceLog(LOG_ERROR, "Failed to load sound file: \"%s\"", p.c_str());
                    }
                }
                return false;
            }, false, true);
        }
        if (ImGui::Button("Sort A-Z")) {
            std::vector<ConfiguredMusic*> newList;
            for (auto m : loaded_sounds)
                if (m != nullptr) newList.push_back(m);
            auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
            std::sort(newList.begin(), newList.end(), [&f](ConfiguredMusic* ia, ConfiguredMusic* ib) -> bool {
                std::wstring as = ia->path.wstring();
                std::wstring bs = ib->path.wstring();
                return std::lexicographical_compare(
                    as.begin(), as.end(), bs.begin(), bs.end(), [&f](wchar_t ai, wchar_t bi) {
                        return f.tolower(ai) < f.tolower(bi);
                });
            });
            loaded_sounds.clear();
            loaded_sounds_by_path.clear();
            loaded_sounds = newList;
            for (unsigned int i = 0; i < loaded_sounds.size(); i++) {
                loaded_sounds_by_path.insert(std::make_pair(NarrowString16To8(loaded_sounds[i]->path.wstring()), i));
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Shuffle")) {
            std::vector<ConfiguredMusic*> newList;
            for (auto m : loaded_sounds)
                if (m != nullptr) newList.push_back(m);
            std::shuffle(newList.begin(), newList.end(), random_generator);
            loaded_sounds.clear();
            loaded_sounds_by_path.clear();
            loaded_sounds = newList;
            for (unsigned int i = 0; i < loaded_sounds.size(); i++) {
                loaded_sounds_by_path.insert(std::make_pair(NarrowString16To8(loaded_sounds[i]->path.wstring()), i));
            }
        }

        for (int i=0; i<loaded_sounds.size(); i++) {
            auto sound = loaded_sounds[i];
            if (sound == nullptr) continue;
            ImGui::PushID(i);
            if (ImGui::Button("Remove")) {
                if (current_loaded_music == sound) {
                    StopMusicStream(sound->music);
                    current_loaded_music = nullptr;
                    current_loaded_music_index++;
                    if (current_loaded_music_index >= loaded_sounds.size()) {
                        if (loaded_sounds.size() > 0) {
                            current_loaded_music_index = 0;
                        } else {
                            current_loaded_music_index = -1;
                        }
                    }
                }
                UnloadMusicStream(sound->music);
                delete sound;
                loaded_sounds[i] = sound = nullptr;
            }
            ImGui::SameLine();
            if (sound != nullptr) {
                if (ImGui::Button("Select")) {
                    current_loaded_music = sound;
                    current_loaded_music_index = i;
                }
                ImGui::SameLine();
                ImGui::Text("%s", sound->name.c_str());
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
    #pragma endregion

    #pragma region Console Menu
    void ConsoleMenu::show() {
        ImGui::Begin("Console");
        ImGui::SetWindowPos({1.0f, 302.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_FirstUseEver);
        for (auto line : console_window_lines) {
            if (line.size() > 0)
                ImGui::TextWrapped("%s", line.c_str());
        }
        if (scroll_log_to_bottom) {
            ImGui::SetScrollY(ImGui::GetCursorPosY() - ImGui::GetWindowHeight());
        }
        ImGui::End();
    }
    #pragma endregion

    #pragma region Theme Menu
    void ThemeMenu::show() {
        ImGui::Begin("Theme");
        if (ImGui::Button("Import from File")) {
            otherFileBrowsers.openIfNotAlready("Import Theme from File", [] (std::string fname) {
                if (fname.length()) {
                    bool rv = global_style.load(fname);
                    if (rv) global_style.apply();
                    return rv;
                }
                return false;
            });
        }
        ImGui::SameLine();
        if (ImGui::Button("Export to File")) {
            otherFileBrowsers.openIfNotAlready("Export Theme to File", [] (std::string fname) {
                if (fname.length()) {
                    return global_style.save(fname);
                }
                return false;
            }, true);
        }
        ImGui::Text("Note: make sure to click \"save ref\" before exporting!");
        ImGui::SeparatorText("Theme Configurator");
        ImGui::ShowStyleEditor(global_style);
        ImGui::End();
    }
    #pragma endregion

    #pragma region Keybind Manager Window
    void HookManagerMenu::show() {
        ImGui::Begin("Keybinds");
        auto bound = Hooks::GetBoundKeycodes();
        int remove = -1;
        for (int i=0; i<bound.size(); i++) {
            ImGui::PushID(i);
            if (ImGui::Button("X")) {
                Hooks::BindKeycode(bound[i], false);
                sound_keybinds.erase(bound[i]);
            }
            ImGui::SameLine();
            ImGui::Text("%s %s", bound[i].tostring().c_str(), sound_keybinds[(int)bound[i]].c_str());
            ImGui::PopID();
        }
        if (remove >= 0) {
            Hooks::BindKeycode(bound[remove], false);
        }
        if (ImGui::Button("Add Keybind")) {
            ImGui::End();
            int vk = 0;
            Hooks::ClearLastKeycode();
            while (!vk) {
                EndDrawing();
                BeginDrawing();
                ClearBackground(BLACK);
                DrawText("Press a key", 32, 32, 32, WHITE);
                vk = Hooks::GetLastKeycode();
            }
            Hooks::BindKeycode(vk);
            sound_keybinds[vk] = NarrowString16To8(current_loaded_music->path.wstring());
        } else {
            ImGui::Text("(to play currently loaded music)");
            ImGui::End();
        }
    }
    #pragma endregion

}