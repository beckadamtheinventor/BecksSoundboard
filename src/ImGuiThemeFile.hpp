#pragma once
#include "../thirdparty/imgui-docking/imgui/imgui.h"
#include "../include/nlohmann/json.hpp"
#include <istream>
#include <ostream>

namespace ImGui {
    class ThemeFile : nlohmann::json {
        protected:
        ImGuiStyle style;
        ImVec2 LoadVec2(nlohmann::json j);
        ImVec4 LoadVec4(nlohmann::json j);
        public:
        ThemeFile() {}
        operator ImGuiStyle&() {
            return style;
        }
        operator ImGuiStyle*() {
            return &style;
        }
        void reset();
        void setFrom(ImGuiStyle s);
        void apply();
        bool save(std::string filename);
        bool save(std::ostream& o);
        bool load(std::string filename);
        bool load(std::istream& o);
        template<class T>
        T get(std::string key) {
            if (contains(key)) {
                try {
                    return (*this)[key].get<T>();
                } catch (nlohmann::detail::exception err) {}
            }
            return T();
        }
        template<class T>
        void set(std::string key, T val) {
            (*this)[key] = val;
        }
        static ThemeFile file(std::string filename);
    };
}