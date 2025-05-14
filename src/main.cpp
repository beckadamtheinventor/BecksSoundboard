
#include "FileDialogs.hpp"
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <imgui.h>
#include <map>
#include <rlImGui.h>
#include <raylib.h>
#include <string>

class ConfiguredSound {
    public:
    Music music;
    float volume=1.0f, pan=0.5f;
    float time=0.0f, length=0.0f, pitch=1.0f;
    std::filesystem::path path;
    std::string name;
    bool started=false;
    ConfiguredSound() {}
    ConfiguredSound(Music s, std::filesystem::path p)
        : music(s), path(p) {
            length = GetMusicTimeLength(music);
            name = std::string(path.filename().string());
        }
};

std::vector<ConfiguredSound*> loaded_sounds;
std::map<unsigned int, ConfiguredSound*> sound_keybinds;
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
    ConfiguredSound* current_loaded_music = nullptr;
    float global_volume = 1.0f;
    std::filesystem::path current_path = std::filesystem::current_path();
    while (!WindowShouldClose()) {
        static float dt = 0;
        BeginDrawing();
        ClearBackground(BLACK);

        rlImGuiBegin();
        ImGui::Begin("Options");
        ImGui::SetWindowPos({1.0f, 1.0f}, ImGuiCond_Once);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_Once);
        if (ImGui::SliderFloat("Volume", &global_volume, 0.0f, 1.0f)) {
            SetMasterVolume(global_volume);
        }
        for (int i=0; i<available_playback_devices.size(); i++) {
            auto dev = &available_playback_devices[i];
            ImGui::PushID(i);
            if (ImGui::Button("Select")) {
                CloseAudioDevice();
                InitAudioDeviceByID(&dev->id);
            }
            ImGui::SameLine();
            ImGui::Text("%s", dev->name);
            ImGui::PopID();
        }
        ImGui::End();
        ImGui::Begin("Sounds");
        ImGui::SetWindowPos({402.0f, 1.0f}, ImGuiCond_Once);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_Once);
        for (int i=0; i<loaded_sounds.size(); i++) {
            auto sound = loaded_sounds[i];
            if (sound == nullptr) continue;
            ImGui::PushID(i);
            if (ImGui::Button("Remove")) {
                if (current_loaded_music == sound) {
                    StopMusicStream(sound->music);
                    current_loaded_music = nullptr;
                }
                UnloadMusicStream(sound->music);
                delete sound;
                loaded_sounds[i] = sound = nullptr;
            }
            ImGui::SameLine();
            if (sound != nullptr) {
                if (ImGui::Button("Select")) {
                    current_loaded_music = sound;
                }
                ImGui::SameLine();
                ImGui::Text("%s", sound->name.c_str());
            }
            ImGui::PopID();
        }
        ImGui::End();
        ImGui::Begin("Console");
        ImGui::SetWindowPos({1.0f, 202.0f}, ImGuiCond_Once);
        ImGui::SetWindowSize({400.0f, 200.0f}, ImGuiCond_Once);
        for (auto line : console_window_lines) {
            if (line.size() > 0)
                ImGui::TextWrapped("%s", line.c_str());
        }
        ImGui::End();
        std::filesystem::path open_path;
        if (OpenFileDialog("Load Sound from Files", current_path, open_path)) {
            Music sound = LoadMusicStream(open_path.string().c_str());
            ConfiguredSound* now_loaded_music = nullptr;
            if (IsMusicReady(sound)) {
                loaded_sounds.push_back(new ConfiguredSound(sound, open_path));
                now_loaded_music = loaded_sounds[loaded_sounds.size()-1];
                TraceLog(LOG_INFO, ("Loaded sound file successfuly: \"" + open_path.string() + "\"").c_str());
            } else {
                TraceLog(LOG_ERROR, ("Failed to load sound file: \"" + open_path.string() + "\"").c_str());
            }
            if (now_loaded_music != nullptr) {
                SetMusicVolume(now_loaded_music->music, now_loaded_music->volume);
                if (current_loaded_music == nullptr) {
                    current_loaded_music = loaded_sounds[loaded_sounds.size()-1];
                }
            }
        }
        if (current_loaded_music) {
            ImGui::Begin("Audio Controls");
            ImGui::Text("%s", current_loaded_music->name.c_str());
            current_loaded_music->time = GetMusicTimePlayed(current_loaded_music->music);
            float music_length = GetMusicTimeLength(current_loaded_music->music);
            if (ImGui::SliderFloat(std::to_string(music_length).c_str(), &current_loaded_music->time, 0.0f, music_length)) {
                SeekMusicStream(current_loaded_music->music, current_loaded_music->time);
            }
            
            if (ImGui::Button("Stop")) {
                StopMusicStream(current_loaded_music->music);
                current_loaded_music->started = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Play")) {
                if (current_loaded_music->started) {
                    ResumeMusicStream(current_loaded_music->music);
                } else {
                    PlayMusicStream(current_loaded_music->music);
                    current_loaded_music->started = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Pause")) {
                PauseMusicStream(current_loaded_music->music);
            }
            ImGui::SameLine();
            if (ImGui::Button("Restart")) {
                PlayMusicStream(current_loaded_music->music);
            }
            if (ImGui::SliderFloat("Volume", &current_loaded_music->volume, 0.0f, 1.0f)) {
                SetMusicVolume(current_loaded_music->music, current_loaded_music->volume);
            }
            if (ImGui::SliderFloat("Pan", &current_loaded_music->pan, 0.0f, 1.0f)) {
                SetMusicPan(current_loaded_music->music, 1.0f - current_loaded_music->pan);
            }
            if (ImGui::SliderFloat("Pitch", &current_loaded_music->pitch, 0.0f, 1.0f)) {
                SetMusicPitch(current_loaded_music->music, current_loaded_music->pitch);
            }
            ImGui::End();
            UpdateMusicStream(current_loaded_music->music);
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

    return 0;
}