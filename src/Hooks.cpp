#include "Hooks.hpp"
#include <cstring>
#include <map>
#include <queue>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#endif

// references:
// - https://gist.github.com/sbarratt/3077d5f51288b39665350dc2b9e19694
// - https://learn.microsoft.com/en-us/windows/win32/winmsg/keyboardproc
// (and related pages)

namespace Hooks {

std::queue<MaskedVirtualKeycode> charactersIn;
std::vector<MaskedVirtualKeycode> charactersBound;
MaskedVirtualKeycode characterLastInput = {0};

#ifdef _WIN32
#pragma region Windows Virtual Keycode Strings
const std::map<int, std::string> WindowsVirtualCharacterStrings = {
    {VK_LBUTTON, "Left mouse button"},
    {VK_RBUTTON, "Right mouse button"},
    {VK_CANCEL, "Control-break processing"},
    {VK_MBUTTON, "Middle mouse button"},
    {VK_XBUTTON1, "X1 mouse button"},
    {VK_XBUTTON2, "X2 mouse button"},
    {VK_BACK, "Backspace"},
    {VK_TAB, "Tab"},
    {VK_CLEAR, "Clear"},
    {VK_RETURN, "Enter"},
    {VK_SHIFT, "Shift"},
    {VK_CONTROL, "Ctrl"},
    {VK_MENU, "Alt"},
    {VK_PAUSE, "Pause"},
    {VK_CAPITAL, "Caps lock"},
    {VK_KANA, "IME Kana mode"},
    {VK_HANGUL, "IME Hangul mode"},
    {VK_IME_ON, "IME On"},
    {VK_JUNJA, "IME Junja mode"},
    {VK_FINAL, "IME final mode"},
    {VK_HANJA, "IME Hanja mode"},
    {VK_KANJI, "IME Kanji mode"},
    {VK_IME_OFF, "IME Off"},
    {VK_ESCAPE, "Esc"},
    {VK_CONVERT, "IME convert"},
    {VK_NONCONVERT, "IME nonconvert"},
    {VK_ACCEPT, "IME accept"},
    {VK_MODECHANGE, "IME mode change request"},
    {VK_SPACE, "Spacebar"},
    {VK_PRIOR, "Page up"},
    {VK_NEXT, "Page down"},
    {VK_END, "End"},
    {VK_HOME, "Home"},
    {VK_LEFT, "Left arrow"},
    {VK_UP, "Up arrow"},
    {VK_RIGHT, "Right arrow"},
    {VK_DOWN, "Down arrow"},
    {VK_SELECT, "Select"},
    {VK_PRINT, "Print"},
    {VK_EXECUTE, "Execute"},
    {VK_SNAPSHOT, "Print screen"},
    {VK_INSERT, "Insert"},
    {VK_DELETE, "Delete"},
    {VK_HELP, "Help"},
    {'0', "0"},
    {'1', "1"},
    {'2', "2"},
    {'3', "3"},
    {'4', "4"},
    {'5', "5"},
    {'6', "6"},
    {'7', "7"},
    {'8', "8"},
    {'9', "9"},
    {'A', "A"},
    {'B', "B"},
    {'C', "C"},
    {'D', "D"},
    {'E', "E"},
    {'F', "F"},
    {'G', "G"},
    {'H', "H"},
    {'I', "I"},
    {'J', "J"},
    {'K', "K"},
    {'L', "L"},
    {'M', "M"},
    {'N', "N"},
    {'O', "O"},
    {'P', "P"},
    {'Q', "Q"},
    {'R', "R"},
    {'S', "S"},
    {'T', "T"},
    {'U', "U"},
    {'V', "V"},
    {'W', "W"},
    {'X', "X"},
    {'Y', "Y"},
    {'Z', "Z"},
    {VK_LWIN, "Left Windows logo"},
    {VK_RWIN, "Right Windows logo"},
    {VK_APPS, "Application"},
    {VK_SLEEP, "Computer Sleep"},
    {VK_NUMPAD0, "Numpad 0"},
    {VK_NUMPAD1, "Numpad 1"},
    {VK_NUMPAD2, "Numpad 2"},
    {VK_NUMPAD3, "Numpad 3"},
    {VK_NUMPAD4, "Numpad 4"},
    {VK_NUMPAD5, "Numpad 5"},
    {VK_NUMPAD6, "Numpad 6"},
    {VK_NUMPAD7, "Numpad 7"},
    {VK_NUMPAD8, "Numpad 8"},
    {VK_NUMPAD9, "Numpad 9"},
    {VK_MULTIPLY, "Multiply"},
    {VK_ADD, "Add"},
    {VK_SEPARATOR, "Separator"},
    {VK_SUBTRACT, "Subtract"},
    {VK_DECIMAL, "Decimal"},
    {VK_DIVIDE, "Divide"},
    {VK_F1, "F1"},
    {VK_F2, "F2"},
    {VK_F3, "F3"},
    {VK_F4, "F4"},
    {VK_F5, "F5"},
    {VK_F6, "F6"},
    {VK_F7, "F7"},
    {VK_F8, "F8"},
    {VK_F9, "F9"},
    {VK_F10, "F10"},
    {VK_F11, "F11"},
    {VK_F12, "F12"},
    {VK_F13, "F13"},
    {VK_F14, "F14"},
    {VK_F15, "F15"},
    {VK_F16, "F16"},
    {VK_F17, "F17"},
    {VK_F18, "F18"},
    {VK_F19, "F19"},
    {VK_F20, "F20"},
    {VK_F21, "F21"},
    {VK_F22, "F22"},
    {VK_F23, "F23"},
    {VK_F24, "F24"},
    {VK_NUMLOCK, "Num lock"},
    {VK_SCROLL, "Scroll lock"},
    {VK_LSHIFT, "Left Shift"},
    {VK_RSHIFT, "Right Shift"},
    {VK_LCONTROL, "Left Ctrl"},
    {VK_RCONTROL, "Right Ctrl"},
    {VK_LMENU, "Left Alt"},
    {VK_RMENU, "Right Alt"},
    {VK_BROWSER_BACK, "Browser Back"},
    {VK_BROWSER_FORWARD, "Browser Forward"},
    {VK_BROWSER_REFRESH, "Browser Refresh"},
    {VK_BROWSER_STOP, "Browser Stop"},
    {VK_BROWSER_SEARCH, "Browser Search"},
    {VK_BROWSER_FAVORITES, "Browser Favorites"},
    {VK_BROWSER_HOME, "Browser Start and Home"},
    {VK_VOLUME_MUTE, "Volume Mute"},
    {VK_VOLUME_DOWN, "Volume Down"},
    {VK_VOLUME_UP, "Volume Up"},
    {VK_MEDIA_NEXT_TRACK, "Next Track"},
    {VK_MEDIA_PREV_TRACK, "Previous Track"},
    {VK_MEDIA_STOP, "Stop Media"},
    {VK_MEDIA_PLAY_PAUSE, "Play/Pause Media"},
    {VK_LAUNCH_MAIL, "Start Mail"},
    {VK_LAUNCH_MEDIA_SELECT, "Select Media"},
    {VK_LAUNCH_APP1, "Start Application 1"},
    {VK_LAUNCH_APP2, "Start Application 2"},
    {VK_OEM_1, "Semicolon/Colon"},
    {VK_OEM_PLUS, "Equals/Plus"},
    {VK_OEM_COMMA, "Comma/Less Than"},
    {VK_OEM_MINUS, "Dash/Underscore"},
    {VK_OEM_PERIOD, "Period/Greater Than"},
    {VK_OEM_2, "Forward Slash/Question Mark"},
    {VK_OEM_3, "Grave/Tilde"},
    {VK_OEM_4, "Left Brace"},
    {VK_OEM_5, "Backslash/Pipe"},
    {VK_OEM_6, "Right Brace"},
    {VK_OEM_7, "Apostrophe/Double Quotation Mark"},
    {VK_OEM_8, "Right Ctrl"},
    {VK_OEM_102, "Backslash/Pipe"},
    {VK_PROCESSKEY, "IME PROCESS"},
    {VK_ATTN, "Attn"},
    {VK_CRSEL, "CrSel"},
    {VK_EXSEL, "ExSel"},
    {VK_EREOF, "Erase EOF"},
    {VK_PLAY, "Play"},
    {VK_ZOOM, "Zoom"},
    {VK_NONAME, "Reserved"},
    {VK_PA1, "PA1"},
    {VK_OEM_CLEAR, "Clear"},
};
#pragma endregion
#endif

MaskedVirtualKeycode GetKeycode() {
    if (charactersIn.size() > 0) {
        MaskedVirtualKeycode code = charactersIn.front();
        charactersIn.pop();
        return code;
    }
    return {0};
}

void ClearLastKeycode() {
    characterLastInput = 0;
}

MaskedVirtualKeycode GetLastKeycode() {
    return characterLastInput;
}

void BindKeycode(MaskedVirtualKeycode code, bool bind) {
    if (bind) {
        charactersBound.push_back(code);
    } else {
        for (int i=0; i<charactersBound.size(); i++) {
            if (charactersBound[i] == code) {
                if (i+1 < charactersBound.size()) {
                    memcpy(
                        &charactersBound[i],
                        &charactersBound[i+1],
                        (charactersBound.size() - 1 - i)*sizeof(MaskedVirtualKeycode)
                    );
                }
                charactersBound.pop_back();
                break;
            }
        }
    }
}

std::vector<MaskedVirtualKeycode> GetBoundKeycodes() {
    return charactersBound;
}

#ifdef _WIN32
#pragma region Windows Hooks

HHOOK hook;

int shift_active() {
	return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0;
}

int capital_active() {
	return (GetKeyState(VK_CAPITAL) & 1) == 1;
}

int alt_active() {
    return GetKeyState(VK_MENU) < 0;
}

int control_active() {
    return GetKeyState(VK_CONTROL) < 0;
}

MaskedVirtualKeycode keymod(int vk) {
    return {vk | (shift_active() << 24) | (capital_active() << 25) | (control_active() << 26) | (alt_active() << 27)};
}

bool isBound(MaskedVirtualKeycode ch) {
    for (MaskedVirtualKeycode b : charactersBound) {
        if (ch == b) {
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK KeyboardProc(
    _In_ int code,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
) {
    // don't process if we're instructed not to
    // don't process the being down more than once
    // (this event gets called constantly while the is down)
    if (code >= 0 && wParam == WM_KEYDOWN) {
        // printf("0x%08X | 0x%08llX | 0x%08llX\n", code, wParam, lParam);
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (!(kbdStruct->vkCode == VK_LSHIFT ||
              kbdStruct->vkCode == VK_RSHIFT ||
              kbdStruct->vkCode == VK_LCONTROL ||
              kbdStruct->vkCode == VK_RCONTROL ||
              kbdStruct->vkCode == VK_CAPITAL ||
              kbdStruct->vkCode == VK_MENU)) {
            MaskedVirtualKeycode ch = keymod(kbdStruct->vkCode);
            if (isBound(ch)) {
                charactersIn.push(ch);
            }
            characterLastInput = ch;
        }
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool InitKeyboardHook() {
    return (hook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyboardProc, NULL, 0)) != nullptr;
}

bool EndKeyboardHook() {
    return UnhookWindowsHookEx(hook);
}

std::string MaskedVirtualKeycode::tostring() {
    std::string modifiers = "";
    if (shift) modifiers += "Shift+";
    if (capital) modifiers += "Caps+";
    if (control) modifiers += "Control+";
    if (alt) modifiers += "Alt+";
    if (WindowsVirtualCharacterStrings.count(vk) > 0) {
        return modifiers + "(" + WindowsVirtualCharacterStrings.at(vk) + ")";
    }
    return modifiers + "(Unknown Keycode " + std::to_string(vk) + ")";
}

#pragma endregion
#else
#pragma region Linux Hooks

bool InitKeyboardHook() {
    return false;
}
bool EndKeyboardHook() {
    return true;
}
std::string MaskedVirtualKeycode::tostring() {
    std::string modifiers = "";
    if (shift) modifiers += "Shift+";
    if (capital) modifiers += "Caps+";
    if (control) modifiers += "Control+";
    if (alt) modifiers += "Alt+";
    return modifiers + "(Keycode " + std::to_string(vk) + ")";
};

#pragma endregion
#endif

}