
#include "../include/nlohmann/json.hpp"
#include "../thirdparty/raylib-5.0/src/external/miniaudio.h"
#include "../thirdparty/raylib-5.0/src/raylib.h"
#include "../thirdparty/rlImGui/rlImGui.h"
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <map>
#include <string>
#include <utility>

#include "JsonConfig.hpp"
#include "FileDialogs.hpp"
using namespace FileDialogs;

class ConfiguredMusic {
    public:
    Music music;
    float volume=1.0f, pan=0.5f;
    float time=0.0f, length=0.0f, pitch=1.0f;
    float start_time=0.0f, end_time=0.0f;
    std::filesystem::path path;
    std::string name;
    bool started=false, repeating=false, show_advanced=false;
    ConfiguredMusic() {}
    ConfiguredMusic(Music s, std::filesystem::path p)
        : music(s), path(p) {
            length = GetMusicTimeLength(music);
            name = std::string(path.filename().string());
            end_time = length;
        }
    void Update() {
        SetMusicVolume(music, volume);
        SetMusicPan(music, pan);
        SetMusicPitch(music, pitch);
    }
    void Stop() {
        StopMusicStream(music);
        started = false;
        time = 0.0f;
    }
    void Play() {
        PlayMusicStream(music);
    }
    void Start() {
        Play();
        if (start_time >= 0.01f) {
            SeekMusicStream(music, start_time);
        }
        started = true;
    }
    void Pause() {
        PauseMusicStream(music);
    }
    void Resume() {
        if (started) {
            ResumeMusicStream(music);
        } else {
            Start();
        }
    }
    void Seek(float t) {
        SeekMusicStream(music, t);
    }
    bool ShouldEnd(float dt) {
        return Tell()+dt*0.95f >= end_time;
    }
    float Tell() {
        return GetMusicTimePlayed(music);
    }
    void Volume(float v) {
        volume = v;
        SetMusicVolume(music, volume);
    }
    void Pan(float v) {
        pan = v;
        SetMusicPan(music, 1.0f-pan);
    }
    void Pitch(float v) {
        pitch = v;
        SetMusicPitch(music, pitch);
    }
    bool Show(float dt) {
        time = Tell();
        ImGui::Begin("Audio Controls");
        ImGui::Text("%s", name.c_str());
        bool ended = false;
        if ((ended = ShouldEnd(dt))) {
            Stop();
            if (repeating) {
                Start();
            }
        }

        char sprintf_buffer[8];
        snprintf(sprintf_buffer, sizeof(sprintf_buffer), "%.3f", end_time);
        if (ImGui::SliderFloat(sprintf_buffer, &time, start_time, end_time)) {
            Seek(time);
        }
        
        if (ImGui::Button("Stop")) {
            Stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Play/Pause")) {
            if (IsMusicStreamPlaying(music)) {
                Pause();
            } else {
                Resume();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart")) {
            Stop();
            Start();
        }
        if (ImGui::Checkbox("Repeat", &repeating)) {
            ;
        }
        if (ImGui::SliderFloat("Volume", &volume, 0.01f, 1.0f)) {
            Volume(volume);
        }
        if (ImGui::Checkbox("Advanced", &show_advanced)) {
            ;
        }
        if (show_advanced) {
            if (ImGui::SliderFloat("Pan", &pan, 0.0f, 1.0f)) {
                Pan(pan);
            }
            if (ImGui::SliderFloat("Speed/Pitch", &pitch, 0.01f, 2.0f)) {
                Pitch(pitch);
            }
            ImGui::Text("Crop");
            if (ImGui::SliderFloat("Start Time", &start_time, 0.0f, length)) {
                if (start_time > end_time) {
                    end_time = start_time;
                }
            }
            if (ImGui::SliderFloat("End Time", &end_time, 0.0f, length)) {
                if (end_time < start_time) {
                    start_time = end_time;
                    Stop();
                }
            }
        }
        ImGui::End();
        UpdateMusicStream(music);
        return !ended;
    }
    void Load(nlohmann::json cfg) {
        if (cfg.contains("v") && cfg["v"].is_number()) {
            volume = cfg["v"].get<float>();
        }
        if (cfg.contains("s") && cfg["s"].is_number()) {
            pitch = cfg["s"].get<float>();
        }
        if (cfg.contains("p") && cfg["p"].is_number()) {
            pan = cfg["p"].get<float>();
        }
        if (cfg.contains("r") && cfg["r"].is_boolean()) {
            repeating = cfg["r"].get<bool>();
        }
        if (cfg.contains("a") && cfg["a"].is_boolean()) {
            show_advanced = cfg["a"].get<bool>();
        }
        if (cfg.contains("st") && cfg["st"].is_number()) {
            start_time = cfg["st"].get<float>();
        }
        if (cfg.contains("et") && cfg["et"].is_number()) {
            end_time = cfg["et"].get<float>();
        }
    }
    nlohmann::json Save() {
        return {
            {"v", volume},
            {"s", pitch},
            {"p", pan},
            {"r", repeating},
            {"a", show_advanced},
            {"st", start_time},
            {"et", end_time},
        };
    }
};

std::vector<ConfiguredMusic*> loaded_sounds;
std::map<std::string, unsigned int> loaded_sounds_by_path;
std::map<unsigned int, unsigned int> sound_keybinds;
std::vector<std::string> console_window_lines;
std::vector<ma_device_info> available_playback_devices;

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
    InitWindow(1000, 600, "Beck's Soundboard");
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

    rlImGuiSetup(true);
    // ImGuiIO& io = ImGui::GetIO();
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // bool currently_loading_sound = false;
    ConfiguredMusic* current_loaded_music = nullptr;
    int current_loaded_music_index = -1;
    float global_volume = 1.0f;
    std::filesystem::path current_path = std::filesystem::current_path();
    FileDialog fileBrowser;
    bool play_in_sequence = false;

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
        nlohmann::json sound_configs = config.contains("sound_configs") ? config["sound_configs"] : nlohmann::json();
        for (std::string p : loaded_sound_paths) {
            Music music = LoadMusicStream(p.c_str());
            if (IsMusicReady(music)) {
                auto cs = new ConfiguredMusic(music, std::filesystem::path(p));
                if (sound_configs.contains(p)) {
                    nlohmann::json cfg = sound_configs[p];
                    cs->Load(cfg);
                }
                loaded_sounds.push_back(cs);
                loaded_sounds_by_path.insert(std::make_pair(cs->path.string(), loaded_sounds.size()-1));
                cs->Update();
            }
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
            fileBrowser.AddPinnedFolder(std::filesystem::path(s));
        }
    }

    SetMasterVolume(global_volume);

    while (!WindowShouldClose()) {
        static float dt = 0;
        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();
        ImGui::Begin("Options");
        ImGui::SetWindowPos({1.0f, 1.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_FirstUseEver);
        if (ImGui::SliderFloat("Volume", &global_volume, 0.0f, 1.0f)) {
            SetMasterVolume(global_volume);
        }
        if (ImGui::Checkbox("Play in Sequence", &play_in_sequence)) {
            ;
        }
        ImGui::Text("Available Playback Devices");
        for (int i=0; i<available_playback_devices.size(); i++) {
            auto dev = &available_playback_devices[i];
            ImGui::PushID(i);
            if (ImGui::Button("Select")) {
                CloseAudioDevice();
                InitAudioDeviceByID(&dev->id);
                SetMasterVolume(global_volume);
            }
            ImGui::SameLine();
            ImGui::Text("%s", dev->name);
            ImGui::PopID();
        }
        ImGui::End();
        ImGui::Begin("Sounds");
        ImGui::SetWindowPos({402.0f, 1.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_FirstUseEver);
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
        ImGui::Begin("Console");
        ImGui::SetWindowPos({1.0f, 202.0f}, ImGuiCond_FirstUseEver);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_FirstUseEver);
        for (auto line : console_window_lines) {
            if (line.size() > 0)
                ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::End();
        std::filesystem::path open_path;
        if (fileBrowser.Show("Load Sound from Files", current_path, open_path)) {
            Music sound = LoadMusicStream(open_path.string().c_str());
            ConfiguredMusic* now_loaded_music = nullptr;
            if (IsMusicReady(sound)) {
                auto cs = new ConfiguredMusic(sound, open_path);
                loaded_sounds.push_back(cs);
                loaded_sounds_by_path.insert(std::make_pair(cs->path.string(), loaded_sounds.size()-1));
                now_loaded_music = loaded_sounds[loaded_sounds.size()-1];
                TraceLog(LOG_INFO, ("Loaded sound file successfuly: \"" + open_path.string() + "\"").c_str());
            } else {
                TraceLog(LOG_ERROR, ("Failed to load sound file: \"" + open_path.string() + "\"").c_str());
            }
            if (now_loaded_music != nullptr) {
                now_loaded_music->Update();
                if (current_loaded_music == nullptr) {
                    current_loaded_music = loaded_sounds[(current_loaded_music_index = loaded_sounds.size()-1)];
                }
            }
        }
        if (current_loaded_music) {
            if (!current_loaded_music->Show(dt)) {
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
                    current_loaded_music = loaded_sounds[current_loaded_music_index];
                    if (current_loaded_music != nullptr) {
                        PlayMusicStream(current_loaded_music->music);
                        current_loaded_music->started = true;
                    }
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
    std::vector<std::filesystem::path> pinned_folder_paths = fileBrowser.GetPinnedFolders();
    std::vector<std::string> pinned_folders;
    for (auto p : pinned_folder_paths) {
        pinned_folders.push_back(NarrowString16To8(p.wstring()));
    }
    config.set("pinned_folders", pinned_folders);
    config.save();

    CloseAudioDevice();
    CloseWindow();

    return 0;
}