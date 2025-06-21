#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <random>
#include <string>
#include <thread>
#include <utility>

#include "../include/nlohmann/json.hpp"
#include "../thirdparty/raylib-5.0/src/external/miniaudio.h"
#include "../thirdparty/raylib-5.0/src/raylib.h"
#include "../thirdparty/rlImGui/rlImGui.h"
#include "../thirdparty/imgui-docking/imgui/imgui.h"

#include "JsonConfig.hpp"
#include "FileDialogs.hpp"
#include "Menus.hpp"
using namespace FileDialogs;
#include "ConfiguredMusic.hpp"
#include "Hooks.hpp"

extern std::vector<ConfiguredMusic*> loaded_sounds;
extern std::map<std::string, unsigned int> loaded_sounds_by_path;
extern std::map<unsigned int, unsigned int> sound_keybinds;
extern std::vector<std::string> console_window_lines;
extern std::vector<ma_device_info> available_playback_devices;
extern int selected_playback_device;
extern std::filesystem::path current_path;
extern FileDialogs::FileDialog fileBrowser;
extern FileDialogs::FileDialogManager otherFileBrowsers;
extern Menus::MenuManager menuManager;
extern nlohmann::json sound_configs;
extern std::random_device random_device;
extern std::mt19937 random_generator;
extern ConfiguredMusic* current_loaded_music;
extern int current_loaded_music_index;
extern float global_volume;
extern bool play_in_sequence;
extern bool scroll_log_to_bottom;

void __TraceLogCallback(int level, const char* fmt, va_list va) {
    std::string levels[] = {
        "TRACE",
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "FATAL",
    };
    char buffer[1024];
    if (level <= 0 || level > LOG_ERROR) {
        level = 1;
    }
    std::string console_line = "[" + levels[level - 1] + "] ";
    printf("%s", console_line.c_str());
    vsnprintf(buffer, sizeof(buffer), fmt, va);
    printf("%s\n", buffer);
    console_line += std::string(buffer);
    console_window_lines.push_back(console_line);
}

int main(int argc, char** argv) {
    SetTraceLogCallback(__TraceLogCallback);
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1200, 600, "Beck's Soundboard");
    SetTargetFPS(60);
    SetExitKey(-1);
    InitAudioDevice();
    if (IsAudioDeviceReady()) {
        TraceLog(LOG_INFO, "Initialized audio device.");
    } else {
        TraceLog(LOG_FATAL, "Failed to init audio device!");
    }
    unsigned int playbackDevicesCount;
    auto playbackDevices = GetPlaybackDevices(&playbackDevicesCount);
    available_playback_devices.reserve(playbackDevicesCount);
    for (unsigned int i=0; i<playbackDevicesCount; i++) {
        available_playback_devices.push_back(playbackDevices[i]);
    }
    // set up ImGUI
    {
        rlImGuiSetup(true);
        ImGui::StyleColorsDark();
        ImGuiIO &io = ImGui::GetIO();
        io.FontGlobalScale = 1.2f;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    JsonConfig config("config.json", {
        {"global_volume", global_volume},
        {"play_in_sequence", play_in_sequence},
        {"current_path", current_path.string()},
        {"currently_playing", ""},
        {"loaded_sounds", {}},
        {"pinned_folders", {}},
    });

    if (config.load()) {
        global_volume = config.get<float>("global_volume");
        play_in_sequence = config.get<bool>("play_in_sequence");
        current_path = std::filesystem::path(config.get<std::string>("current_path"));
        std::vector<std::string> loaded_sound_paths = config.get<std::vector<std::string>>("loaded_sounds");
        sound_configs = config.contains("sound_configs") ? config["sound_configs"] : nlohmann::json();
        for (std::string p : loaded_sound_paths) {
            nlohmann::json cfg;
            ConfiguredMusic* cs;
            if (sound_configs.contains(p)) {
                cfg = sound_configs[p];
            }
            cs = ConfiguredMusic::Load(p, cfg);
            loaded_sounds.push_back(cs);
            loaded_sounds_by_path.insert(std::make_pair(p, loaded_sounds.size()-1));
        }
        std::string currently_playing = config.get<std::string>("currently_playing");
        if (currently_playing.length() > 0) {
            if (loaded_sounds_by_path.count(currently_playing)) {
                current_loaded_music_index = loaded_sounds_by_path[currently_playing];
                current_loaded_music = loaded_sounds[current_loaded_music_index];
                current_loaded_music->started = false;
            }
        }
        std::vector<std::string> pinned_folders = config.get<std::vector<std::string>>("pinned_folders");
        for (auto s : pinned_folders) {
            AddPinnedFolder(std::filesystem::path(s));
        }
    }

    SetMasterVolume(global_volume);

    std::thread musicUpdater = std::thread([] () {
        while (!WindowShouldClose()) {
            if (current_loaded_music) {
                current_loaded_music->UpdateStream();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000/60));
        }
    });

    menuManager.add(new Menus::OptionsMenu());
    menuManager.add(new Menus::SoundsMenu());
    menuManager.add(new Menus::ConsoleMenu());

    while (!WindowShouldClose()) {
        static float dt = 0;

        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();
        otherFileBrowsers.show();
        menuManager.show();

        std::filesystem::path open_path;
        if (fileBrowser.Show(open_path)) {
            nlohmann::json cfg;
            std::string p = NarrowString16To8(open_path.wstring());
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
            if (cs != nullptr) {
                cs->Update();
                if (current_loaded_music == nullptr) {
                    current_loaded_music = loaded_sounds[(current_loaded_music_index = loaded_sounds.size()-1)];
                }
            }
        }
        if (current_loaded_music) {
            int action = current_loaded_music->Show(dt);
            bool load_new_song = true;
            if (action == 1) {
                if (play_in_sequence && loaded_sounds.size() > 1) {
                    current_loaded_music_index++;
                    bool has_wrapped_around = false;
                    if (current_loaded_music_index >= loaded_sounds.size()) {
                        current_loaded_music_index = 0;
                        has_wrapped_around = true;
                    }
                    while (loaded_sounds[current_loaded_music_index] == nullptr) {
                        current_loaded_music_index++;
                        if (current_loaded_music_index >= loaded_sounds.size()) {
                            current_loaded_music_index = 0;
                            if (has_wrapped_around) {
                                break;
                            }
                            has_wrapped_around = true;
                        }
                    }
                }
            } else if (action == 2) {
                // previous
                current_loaded_music_index--;
                bool has_wrapped_around = false;
                if (current_loaded_music_index < 0) {
                    current_loaded_music_index = loaded_sounds.size() - 1;
                    has_wrapped_around = true;
                }
                while (loaded_sounds[current_loaded_music_index] == nullptr) {
                    if (current_loaded_music_index < 0) {
                        current_loaded_music_index = loaded_sounds.size();
                        if (has_wrapped_around) {
                            break;
                        }
                        has_wrapped_around = true;
                    }
                    current_loaded_music_index--;
                }
            } else if (action == 3) {
                // previous
                current_loaded_music_index++;
                bool has_wrapped_around = false;
                if (current_loaded_music_index >= loaded_sounds.size()) {
                    current_loaded_music_index = 0;
                    has_wrapped_around = true;
                }
                while (loaded_sounds[current_loaded_music_index] == nullptr) {
                    if (current_loaded_music_index >= loaded_sounds.size()) {
                        current_loaded_music_index = 0;
                        if (has_wrapped_around) {
                            break;
                        }
                        has_wrapped_around = true;
                    }
                    current_loaded_music_index++;
                }
            } else {
                load_new_song = false;
            }
            if (load_new_song && current_loaded_music_index < loaded_sounds.size()) {
                current_loaded_music = loaded_sounds[current_loaded_music_index];
                if (current_loaded_music != nullptr) {
                    PlayMusicStream(current_loaded_music->music);
                    current_loaded_music->started = true;
                }
            }
        }
        rlImGuiEnd();
        // if (currently_loading_sound) {
        //     static float loading_timer = 0;
        //     DrawCircle(1, 1, 3, WHITE);
        //     for (float ct=0.5f; ct<3.0f; ct+=0.5f) {
        //         if (loading_timer >= ct) {
        //             DrawCircle(1 + ct*6.0f, 1, 3, WHITE);
        //         }
        //     }
        //     loading_timer += dt;
        //     if (loading_timer >= 4.0f) {
        //         loading_timer -= 4.0f;
        //     }
        // }
        EndDrawing();
        dt = GetFrameTime();
    }

    config.set("global_volume", global_volume);
    config.set("play_in_sequence", play_in_sequence);
    config.set("current_path", current_path.string());
    if (current_loaded_music) {
        config.set("currently_playing", current_loaded_music->path.string());
    } else {
        config.set("currently_playing", "");
    }
    std::vector<std::string> saved_sound_paths;
    nlohmann::json saved_sound_configs;
    for (auto pair : loaded_sounds_by_path) {
        auto cs = loaded_sounds[pair.second];
        if (cs) {
            saved_sound_paths.push_back(pair.first);
            saved_sound_configs[pair.first] = cs->Save();
        }
    }
    config.set("loaded_sounds", saved_sound_paths);
    config.set("sound_configs", saved_sound_configs);
    std::vector<std::filesystem::path> pinned_folder_paths = GetPinnedFolders();
    std::vector<std::string> pinned_folders;
    for (auto p : pinned_folder_paths) {
        pinned_folders.push_back(NarrowString16To8(p.wstring()));
    }
    config.set("pinned_folders", pinned_folders);
    config.save();

    CloseAudioDevice();
    CloseWindow();
    musicUpdater.join();

    return 0;
}