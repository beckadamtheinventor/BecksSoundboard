
#include "ImGuiThemeFile.hpp"
#include <fstream>

namespace ImGui {
    static const std::vector<std::string> themeColors = {
        "Text",
        "TextDisabled",
        "WindowBg",
        "ChildBg",
        "PopupBg",
        "Border",
        "BorderShadow",
        "FrameBg",
        "FrameBgHovered",
        "FrameBgActive",
        "TitleBg",
        "TitleBgActive",
        "TitleBgCollapsed",
        "MenuBarBg",
        "ScrollbarBg",
        "ScrollbarGrab",
        "ScrollbarGrabHovered",
        "ScrollbarGrabActive",
        "CheckMark",
        "SliderGrab",
        "SliderGrabActive",
        "Button",
        "ButtonHovered",
        "ButtonActive",
        "Header",
        "HeaderHovered",
        "HeaderActive",
        "Separator",
        "SeparatorHovered",
        "SeparatorActive",
        "ResizeGrip",
        "ResizeGripHovered",
        "ResizeGripActive",
        "Tab",
        "TabHovered",
        "TabActive",
        "TabUnfocused",
        "TabUnfocusedActive",
        "DockingPreview",
        "DockingEmptyBg",
        "PlotLines",
        "PlotLinesHovered",
        "PlotHistogram",
        "PlotHistogramHovered",
        "TableHeaderBg",
        "TableBorderStrong",
        "TableBorderLight",
        "TableRowBg",
        "TableRowBgAlt",
        "TextSelectedBg",
        "DragDropTarget",
        "NavHighlight",
        "NavWindowingHighlight",
        "NavWindowingDimBg",
        "ModalWindowDimBg",
    };

    ImVec2 ThemeFile::LoadVec2(nlohmann::json j) {
        if (j.is_array()) {
            return {
                j[0].get<float>(),
                j[1].get<float>()
            };
        }
        return {0,0};
    }
    ImVec4 ThemeFile::LoadVec4(nlohmann::json j) {
        if (j.is_array()) {
            return {
                j[0].get<float>(),
                j[1].get<float>(),
                j[2].get<float>(),
                j[3].get<float>()
            };
        }
        return {0,0,0,0};
    }
    void ThemeFile::reset() {
        StyleColorsDark(&style);
        setFrom(style);
    }
    void ThemeFile::setFrom(ImGuiStyle style) {
        nlohmann::json colors;
        #define a style.Colors[i]
        for (int i=0; i<themeColors.size(); i++) {
            colors[themeColors[i]] = {a.x, a.y, a.z, a.w};
        }
        #undef a
        (*this)["Colors"] = colors;
        nlohmann::json styleVars;
        styleVars["Alpha"] = style.Alpha;
        styleVars["DisabledAlpha"] = style.DisabledAlpha;
        styleVars["WindowPadding"] = {style.WindowPadding.x, style.WindowPadding.y};
        styleVars["WindowRounding"] = style.WindowRounding;
        styleVars["WindowBorderSize"] = style.WindowBorderSize;
        styleVars["WindowMinSize"] = {style.WindowMinSize.x, style.WindowMinSize.y};
        styleVars["WindowTitleAlign"] = {style.WindowTitleAlign.x, style.WindowTitleAlign.y};
        styleVars["ChildRounding"] = style.ChildRounding;
        styleVars["ChildBorderSize"] = style.ChildBorderSize;
        styleVars["PopupRounding"] = style.PopupRounding;
        styleVars["PopupBorderSize"] = style.PopupBorderSize;
        styleVars["FramePadding"] = {style.FramePadding.x, style.FramePadding.y};
        styleVars["FrameRounding"] = style.FrameRounding;
        styleVars["FrameBorderSize"] = style.FrameBorderSize;
        styleVars["ItemSpacing"] = {style.ItemSpacing.x, style.ItemSpacing.y};
        styleVars["ItemInnerSpacing"] = {style.ItemInnerSpacing.x, style.ItemInnerSpacing.y};
        styleVars["IndentSpacing"] = style.IndentSpacing;
        styleVars["CellPadding"] = {style.CellPadding.x, style.CellPadding.y};
        styleVars["ScrollbarSize"] = style.ScrollbarSize;
        styleVars["ScrollbarRounding"] = style.ScrollbarRounding;
        styleVars["GrabMinSize"] = style.GrabMinSize;
        styleVars["GrabRounding"] = style.GrabRounding;
        styleVars["TabRounding"] = style.TabRounding;
        styleVars["ButtonTextAlign"] = {style.ButtonTextAlign.x, style.ButtonTextAlign.y};
        styleVars["SelectableTextAlign"] = {style.SelectableTextAlign.x, style.SelectableTextAlign.y};
        styleVars["SeparatorTextBorderSize"] = style.SeparatorTextBorderSize;
        styleVars["SeparatorTextAlign"] = {style.SeparatorTextAlign.x, style.SeparatorTextAlign.y};
        styleVars["SeparatorTextPadding"] = {style.SeparatorTextPadding.x, style.SeparatorTextPadding.y};
        (*this)["StyleVars"] = styleVars;
    }
    void ThemeFile::apply() {
        if (contains("Colors")) {
            nlohmann::json j = at("Colors");
            for (int i=0; i<themeColors.size(); i++) {
                if (j.contains(themeColors[i])) {
                    style.Colors[i] = LoadVec4(j[themeColors[i]]);
                }
            }
        }
        if (contains("StyleVars")) {
            nlohmann::json j = at("StyleVars");
            if (j.contains("Alpha")) {
                style.Alpha = j["Alpha"].get<float>();
            }
            if (j.contains("DisabledAlpha")) {
                style.DisabledAlpha = j["DisabledAlpha"].get<float>();
            }
            if (j.contains("WindowPadding")) {
                style.WindowPadding = LoadVec2(j["WindowPadding"]);
            }
            if (j.contains("WindowRounding")) {
                style.WindowRounding = j["WindowRounding"].get<float>();
            }
            if (j.contains("WindowBorderSize")) {
                style.WindowBorderSize = j["WindowBorderSize"].get<float>();
            }
            if (j.contains("WindowMinSize")) {
                style.WindowMinSize = LoadVec2(j["WindowMinSize"]);
            }
            if (j.contains("WindowTitleAlign")) {
                style.WindowTitleAlign = LoadVec2(j["WindowTitleAlign"]);
            }
            if (j.contains("ChildRounding")) {
                style.ChildRounding = j["ChildRounding"].get<float>();
            }
            if (j.contains("ChildBorderSize")) {
                style.ChildBorderSize = j["ChildBorderSize"].get<float>();
            }
            if (j.contains("PopupRounding")) {
                style.PopupRounding = j["PopupRounding"].get<float>();
            }
            if (j.contains("PopupBorderSize")) {
                style.PopupBorderSize = j["PopupBorderSize"].get<float>();
            }
            if (j.contains("FramePadding")) {
                style.FramePadding = LoadVec2(j["FramePadding"]);
            }
            if (j.contains("FrameRounding")) {
                style.FrameRounding = j["FrameRounding"].get<float>();
            }
            if (j.contains("FrameBorderSize")) {
                style.FrameBorderSize = j["FrameBorderSize"].get<float>();
            }
            if (j.contains("ItemSpacing")) {
                style.ItemSpacing = LoadVec2(j["ItemSpacing"]);
            }
            if (j.contains("ItemInnerSpacing")) {
                style.ItemInnerSpacing = LoadVec2(j["ItemInnerSpacing"]);
            }
            if (j.contains("IndentSpacing")) {
                style.IndentSpacing = j["IndentSpacing"].get<float>();
            }
            if (j.contains("CellPadding")) {
                style.CellPadding = LoadVec2(j["CellPadding"]);
            }
            if (j.contains("ScrollbarSize")) {
                style.ScrollbarSize = j["ScrollbarSize"].get<float>();
            }
            if (j.contains("ScrollbarRounding")) {
                style.ScrollbarRounding = j["ScrollbarRounding"].get<float>();
            }
            if (j.contains("GrabMinSize")) {
                style.GrabMinSize = j["GrabMinSize"].get<float>();
            }
            if (j.contains("GrabRounding")) {
                style.GrabRounding = j["GrabRounding"].get<float>();
            }
            if (j.contains("TabRounding")) {
                style.TabRounding = j["TabRounding"].get<float>();
            }
            if (j.contains("ButtonTextAlign")) {
                style.ButtonTextAlign = LoadVec2(j["ButtonTextAlign"]);
            }
            if (j.contains("SelectableTextAlign")) {
                style.SelectableTextAlign = LoadVec2(j["SelectableTextAlign"]);
            }
            if (j.contains("SeparatorTextBorderSize")) {
                style.SeparatorTextBorderSize = j["SeparatorTextBorderSize"].get<float>();
            }
            if (j.contains("SeparatorTextAlign")) {
                style.SeparatorTextAlign = LoadVec2(j["SeparatorTextAlign"]);
            }
            if (j.contains("SeparatorTextPadding")) {
                style.SeparatorTextPadding = LoadVec2(j["SeparatorTextPadding"]);
            }
        }
        ImGui::GetStyle() = style;
    }
    bool ThemeFile::save(std::string filename) {
        std::ofstream fd(filename);
        if (fd.is_open()) {
            bool rv = save(fd);
            fd.close();
            return rv;
        }
        return false;
    }
    bool ThemeFile::save(std::ostream& o) {
        setFrom(style);
        try {
            std::string s = this->dump(1, '\t', true);
            o.write(s.c_str(), s.length());
        } catch (nlohmann::detail::exception err) {
            return false;
        }
        return true;
    }

    bool ThemeFile::load(std::string filename) {
        std::ifstream fd(filename);
        if (fd.is_open()) {
            bool rv = load(fd);
            fd.close();
            return rv;
        }
        return false;
    }

    bool ThemeFile::load(std::istream& o) {
        try {
            o >> *this;
        } catch (nlohmann::detail::exception err) {
            return false;
        }
        return true;
    }

    ThemeFile ThemeFile::file(std::string filename) {
        ThemeFile th;
        th.load(filename);
        return th;
    }

}