#ifndef __MEDIA_KEYS_H_
#define __MEDIA_KEYS_H_

#include <Kaleidoscope.h>

#include <Kaleidoscope-FocusSerial.h>
#include <kaleidoscope/hooks.h>

namespace prag {
namespace serial {
using namespace kaleidoscope;

class FocusMediaKeys : public Plugin {
  public:
    FocusMediaKeys() {}

    EventHandlerResult onFocusEvent(const char* command) {
        // Don't handle events, we only propagate.
        return EventHandlerResult::OK;
    }

    static void pressed(uint8_t keyID) { Focus.send(F("hi")); }
};
} // namespace serial
} // namespace prag

prag::serial::FocusMediaKeys FocusMediaKeys;

#endif // __MEDIA_KEYS_H_
