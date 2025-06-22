#include <string>
#include <vector>
namespace Hooks {

class MaskedVirtualKeycode {
    public:
    union {
        int vk : 24;
        struct {
            unsigned char _0,_1,_2;
            bool shift   : 1;
            bool capital : 1;
            bool control : 1;
            bool alt     : 1;
        };
    };
    bool operator==(MaskedVirtualKeycode other) {
        return *(unsigned int*)this == *(unsigned int*)&other;
    }
    MaskedVirtualKeycode(int v) {
        *(unsigned int*)this = v;
    }
    operator unsigned int() {
        return *(unsigned int*)this;
    }
    std::string tostring();
    operator std::string() {
        return tostring();
    }
};


// Initialize keyboard hook
// returns true if success, otherwise failed
bool InitKeyboardHook();
// Remove the keyboard hook
// returns true if success, otherwise failed
bool EndKeyboardHook();
// get a virtual keycode from the queue
// returns 0 if the queue is empty
MaskedVirtualKeycode GetKeycode();
// bind or unbind a given virtual keycode
void BindKeycode(MaskedVirtualKeycode keycode, bool bind=true);
// returns the list of bound keycodes
std::vector<MaskedVirtualKeycode> GetBoundKeycodes();
// returns the last keycode recieved by the hook regardless if it was bound or not
MaskedVirtualKeycode GetLastKeycode();
// clears the value returned by GetLastKeycode
// chances are you'll want to call this before waiting on GetLastKeycode
void ClearLastKeycode();
}