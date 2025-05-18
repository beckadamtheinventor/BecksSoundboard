#pragma once
#include <filesystem>

#include "../thirdparty/raylib-5.0/src/raylib.h"
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include "../include/nlohmann/json.hpp"

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
    // returns true unless the music has ended and is not set to loop.
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
        if (ImGui::Checkbox("Loop", &repeating)) {
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
