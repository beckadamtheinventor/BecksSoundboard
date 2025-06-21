#include "Hooks.hpp"
#include <map>
#include <queue>

#ifdef _WIN32
#include <windows.h>
#else
#endif

namespace Hooks {

std::queue<int> charactersIn;
std::map<int, bool> charactersBound;

int GetKeycode() {
    if (charactersIn.size() > 0) {
        int code = charactersIn.front();
        charactersIn.pop();
        return code;
    }
    return 0;
}

void BindKeycode(int keycode, bool bind) {
    charactersBound[keycode] = bind;
}

#ifdef _WIN32

// referenced https://gist.github.com/sbarratt/3077d5f51288b39665350dc2b9e19694
// and https://learn.microsoft.com/en-us/windows/win32/winmsg/keyboardproc
// (and related pages)

HHOOK hook;

int shift_active() {
	return GetKeyState(VK_LSHIFT) < 0 || GetKeyState(VK_RSHIFT) < 0;
}

int capital_active() {
	return (GetKeyState(VK_CAPITAL) & 1) == 1;
}


LRESULT CALLBACK KeyboardProc(
    _In_ int code,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
) {
    // don't process if we're instructed not to
    // don't process the key being down more than once
    // (this event gets called constantly while the key is down)
    if (code >= 0 && !(lParam&0xffff)) {
        charactersIn.push(wParam);
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

bool InitKeyboardHook() {
    return (hook = SetWindowsHookExA(WH_KEYBOARD, KeyboardProc, NULL, 0)) != NULL;
}

#else

bool InitKeyboardHook() {
    return false;
}

#endif

}