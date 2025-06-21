#pragma once
#include <filesystem>
#include <math.h>

#include "../thirdparty/raylib-5.0/src/raylib.h"
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include "../include/nlohmann/json.hpp"
#include "FileDialogs.hpp"

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
    static ConfiguredMusic* Load(std::filesystem::path p, nlohmann::json cfg) {
        Music m = LoadMusicStream(FileDialogs::NarrowString16To8(p.wstring()).c_str());
        if (!IsMusicReady(m)) {
            return nullptr;
        }
        ConfiguredMusic* cs = new ConfiguredMusic(m, p);
        cs->Load(cfg);
        cs->Update();
        return cs;
    }
    void Unload() {
        UnloadMusicStream(music);
        music = {0};
    }
    void Update() {
        SetMusicVolume(music, volume);
        SetMusicPan(music, pan);
        SetMusicPitch(music, pitch);
    }
    void Stop() {
        started = false;
        time = 0.0f;
        StopMusicStream(music);
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
        if (t < GetMusicTimeLength(music)) {
            SeekMusicStream(music, t);
        }
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
    // returns 0 if the music is playing.
    // returns 1 if the music has ended and is not set to loop.
    // returns 2 if the previous button was pressed.
    // returns 3 if the next button was pressed.
    int Show(float dt) {
        time = Tell();
        ImGui::Begin("Audio Controls");
        ImGui::Text("%s", name.c_str());
        int rval = 0;
        if ((rval = ShouldEnd(dt))) {
            Stop();
            if (repeating) {
                Start();
                rval = 0;
            }
        }

        if (end_time >= length) {
            end_time = length;
        }
        if (start_time < 0) {
            start_time = 0;
        }
        if (ImGui::SliderFloat("seek", &time, start_time, end_time)) {
            if (IsMusicStreamPlaying(music)) {
                Pause();
            }
            Seek(time);
        }
        ImGui::Text("%d:%02d:%02d.%03d / %d:%02d:%02d.%03d",
            int(floorf(time/3600.0f)), // hours
            int(fmodf(floorf(time/60.0f), 60.0f)), // minutes
            int(fmodf(time, 60.0f)), // seconds
            int(fmodf(time*1000.0f, 1000.0f)), // milliseconds
            int(floorf(length/3600.0f)), // hours
            int(fmodf(floorf(length/60.0f), 60.0f)), // minutes
            int(fmodf(length, 60.0f)), // seconds
            int(fmodf(length*1000.0f, 1000.0f)) // milliseconds
        );
        if (ImGui::Button("<")) {
            rval = 2;
        }
        ImGui::SameLine();
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
        ImGui::SameLine();
        if (ImGui::Button(">")) {
            rval = 3;
        }
        if (ImGui::Checkbox("Loop", &repeating)) {
            ;
        }
        if (ImGui::SliderFloat("Volume", &volume, 0.01f, 2.0f)) {
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
        return rval;
    }
    void UpdateStream() {
        UpdateMusicStream(music);
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
        if (cfg.contains("t") && cfg["t"].is_number()) {
            Seek(cfg["t"].get<float>());
        }
    }
    nlohmann::json Save() {
        return {
            {"v", volume},
            {"s", pitch},
            {"p", pan},
            {"r", repeating},
            {"a", show_advanced},
            {"t", Tell()},
            {"st", start_time},
            {"et", end_time},
        };
    }
};
