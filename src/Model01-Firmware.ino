// -*- mode: c++ -*-
// Copyright 2016 Keyboardio, inc. <jesse@keyboard.io>
// See "LICENSE" for license details

#include "Kaleidoscope.h"                        // Kaleidoscope core
#include "Kaleidoscope-EEPROM-Keymap.h"          // EEPROM-backed runtime keymap
#include "Kaleidoscope-EEPROM-Settings.h"        // EEPROM support
#include "Kaleidoscope-FocusSerial.h"            // Serial communication
#include "Kaleidoscope-HostPowerManagement.h"    // Power state event hook
#include "Kaleidoscope-LED-ActiveModColor.h"     // Color modifier presses
#include "Kaleidoscope-LEDControl.h"             // LED support
#include "Kaleidoscope-LayerFocus.h"             // Layer serial commands
#include "Kaleidoscope-Macros.h"                 // Macro keys
#include "Kaleidoscope-MagicCombo.h"             // Key chording and combos
#include "Kaleidoscope-USB-Quirks.h"             // USB HID protocol switching
#include "kaleidoscope/plugin/HostOS-Focus.h"    // HostOS detection (serial comm)
#include "Kaleidoscope-LEDEffect-BootGreeting.h" // LED key boot animation
#include "Kaleidoscope-LEDEffect-Breathe.h"      // Whole-board breathing effect
#include "Kaleidoscope-LEDEffect-Chase.h"        // Cross-board light chase effect
#include "Kaleidoscope-LEDEffect-Rainbow.h"      // Whole-board rainbow effect

#define BUILD_INFORMATION                                                                                              \
    "commit-id:" SOURCE_CONTROL_REVISION                                                                               \
    "\n"                                                                                                               \
    "commit-date:" SOURCE_CONTROL_DATE                                                                                 \
    "\n"                                                                                                               \
    "commit-user:" SOURCE_CONTROL_AUTHOR                                                                               \
    "\n"                                                                                                               \
    "commit-sub:" SOURCE_CONTROL_SAFE_SUBJECT                                                                          \
    "\n"                                                                                                               \
    "build-date:" BUILD_METADATA_DATE                                                                                  \
    "\n"                                                                                                               \
    "build-user:" BUILD_METADATA_USER "\n"

/** This 'enum' is a list of all the macros used by the Model 01's firmware
 *
 * The names aren't particularly important. What is important is that each
 * is unique.
 *
 * These are the names of your macros. They'll be used in two places.
 * The first is in your keymap definitions. There, you'll use the syntax
 * `M(MACRO_NAME)` to mark a specific keymap position as triggering `MACRO_NAME`
 *
 * The second usage is in the 'switch' statement in the `macroAction` function.
 * That switch statement actually runs the code associated with a macro when
 * a macro key is pressed.
 */

enum {
    MACRO_VERSION_INFO,
    MACRO_ANY,
    MACRO_TOGGLE_QWERTY_COLEMAK,
};

/** The Model 01's key layouts are defined as 'keymaps'. By default, there are three
 * keymaps: The standard QWERTY keymap, the "Function layer" keymap and the "Numpad"
 * keymap.
 *
 * Each keymap is defined as a list using the 'KEYMAP_STACKED' macro, built
 * of first the left hand's layout, followed by the right hand's layout.
 *
 * Keymaps typically consist mostly of `Key_` definitions. There are many, many keys
 * defined as part of the USB HID Keyboard specification. You can find the names
 * (if not yet the explanations) for all the standard `Key_` defintions offered by
 * Kaleidoscope in these files:
 *
 *    - https://github.com/keyboardio/Kaleidoscope/blob/master/src/key_defs_keyboard.h
 *    - https://github.com/keyboardio/Kaleidoscope/blob/master/src/key_defs_consumerctl.h
 *    - https://github.com/keyboardio/Kaleidoscope/blob/master/src/key_defs_sysctl.h
 *    - https://github.com/keyboardio/Kaleidoscope/blob/master/src/key_defs_keymaps.h
 *
 * Additional things that should be documented here include
 *
 *   - using ___ to let keypresses fall through to the previously active layer
 *   - using XXX to mark a keyswitch as 'blocked' on this layer
 *   - using ShiftToLayer() and LockLayer() keys to change the active keymap.
 *   - keeping NUM and FN consistent and accessible on all layers
 *
 * The PROG key is special, since it is how you indicate to the board that you
 * want to flash the firmware. However, it can be remapped to a regular key.
 * When the keyboard boots, it first looks to see whether the PROG key is held
 * down; if it is, it simply awaits further flashing instructions. If it is
 * not, it continues loading the rest of the firmware and the keyboard
 * functions normally, with whatever binding you have set to PROG. More detail
 * here:
 *
 * https://community.keyboard.io/t/how-the-prog-key-gets-you-into-the-bootloader/506/8
 *
 *
 * The "keymaps" data structure is a list of the keymaps compiled into the firmware.
 * The order of keymaps in the list is important, as the ShiftToLayer(#) and LockLayer(#)
 * macros switch to key layers based on this list.
 *
 * A key defined as 'ShiftToLayer(FUNCTION)' will switch to FUNCTION while held.
 * Similarly, a key defined as 'LockLayer(NUMPAD)' will switch to NUMPAD when tapped.
 */

/**
 * Layers are "0-indexed" -- That is the first one is layer 0. The second one is layer 1.
 * The third one is layer 2.
 *
 * This 'enum' lets us use names like QWERTY, FUNCTION, and NUMPAD in place of
 * the numbers 0, 1 and 2.
 *
 */

enum { QWERTY, COLEMAK, FUNCTION }; // layers

/* This comment temporarily turns off astyle's indent enforcement
 *   so we can make the keymaps actually resemble the physical key layout better
 */

KEYMAPS(
    // clang-format off

  // QWERTY
  [QWERTY] = KEYMAP_STACKED
  (___,          Key_1, Key_2, Key_3, Key_4, Key_5, Key_LEDEffectNext,
   Key_Backtick, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Tab,
   Key_PageUp,   Key_A, Key_S, Key_D, Key_F, Key_G,
   Key_PageDown, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_Escape,
   Key_LeftControl, Key_Backspace, Key_LeftGui, Key_LeftShift,
   ShiftToLayer(FUNCTION),

   M(MACRO_ANY),  Key_6, Key_7, Key_8,     Key_9,         Key_0,         ___,
   Key_Enter,     Key_Y, Key_U, Key_I,     Key_O,         Key_P,         Key_Equals,
                  Key_H, Key_J, Key_K,     Key_L,         Key_Semicolon, Key_Quote,
   Key_RightGui,  Key_N, Key_M, Key_Comma, Key_Period,    Key_Slash,     Key_Minus,
   Key_RightShift, Key_LeftAlt, Key_Spacebar, Key_RightControl,
   ShiftToLayer(FUNCTION)),

  // COLEMAK Typing for QWERTY input use cases.
  [COLEMAK] = KEYMAP_STACKED
  (___,          Key_1, Key_2, Key_3, Key_4, Key_5, Key_LEDEffectNext,
   Key_Backtick, Key_Q, Key_W, Key_F, Key_P, Key_G, Key_Tab,
   Key_PageUp,   Key_A, Key_R, Key_S, Key_T, Key_D,
   Key_PageDown, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_Escape,
   Key_LeftControl, Key_Backspace, Key_LeftGui, Key_LeftShift,
   ShiftToLayer(FUNCTION),

   M(MACRO_ANY),  Key_6, Key_7, Key_8,     Key_9,         Key_0,         ___,
   Key_Enter,     Key_J, Key_L, Key_U,     Key_Y,         Key_Semicolon, Key_Equals,
                  Key_H, Key_N, Key_E,     Key_I,         Key_O,         Key_Quote,
   Key_RightAlt,  Key_K, Key_M, Key_Comma, Key_Period,    Key_Slash,     Key_Minus,
   Key_RightShift, Key_LeftAlt, Key_Spacebar, Key_RightControl,
   ShiftToLayer(FUNCTION)),

  [FUNCTION] =  KEYMAP_STACKED
  (M(MACRO_VERSION_INFO),      Key_F1,           Key_F2,      Key_F3,     Key_F4,        Key_F5,           M(MACRO_TOGGLE_QWERTY_COLEMAK),
   Key_Tab,  ___,              ___, ___,        ___, ___, ___,
   Key_Home, ___,       ___, ___, ___, ___,
   Key_End,  Key_PrintScreen,  Key_Insert,  ___,        ___, ___,  ___,
   ___, Key_Delete, ___, ___,
   ___,

   Consumer_ScanPreviousTrack, Key_F6,                 Key_F7,                   Key_F8,                   Key_F9,          Key_F10,          Key_F11,
   Consumer_PlaySlashPause,    Consumer_ScanNextTrack, Key_LeftCurlyBracket,     Key_RightCurlyBracket,    Key_LeftBracket, Key_RightBracket, Key_F12,
                               Key_LeftArrow,          Key_DownArrow,            Key_UpArrow,              Key_RightArrow,  ___,              ___,
   Key_PcApplication,          Consumer_Mute,          Consumer_VolumeDecrement, Consumer_VolumeIncrement, ___,             Key_Backslash,    Key_Pipe,
   ___, ___, Key_Enter, ___,
   ___)
    // clang-format on

    ) // KEYMAPS(

/** versionInfoMacro handles the 'firmware version info' macro
 *  When a key bound to the macro is pressed, this macro
 *  prints out the firmware build information as virtual keystrokes
 */
static void versionInfoMacro(uint8_t keyState) {
    if (keyToggledOn(keyState)) {
        Macros.type(PSTR("Keyboardio Model 01 - Kaleidoscope\n"));
        Macros.type(PSTR("kaleidoscope-commit: " KALEIDOSCOPE_COMMIT "\n"));
        Macros.type(PSTR(BUILD_INFORMATION));
    }
}

/** anyKeyMacro is used to provide the functionality of the 'Any' key.
 *
 * When the 'any key' macro is toggled on, a random alphanumeric key is
 * selected. While the key is held, the function generates a synthetic
 * keypress event repeating that randomly selected key.
 *
 */
static void anyKeyMacro(uint8_t keyState) {
    static Key lastKey;
    bool toggledOn = false;
    if (keyToggledOn(keyState)) {
        lastKey.setKeyCode(Key_A.getKeyCode() + (uint8_t)(millis() % 36));
        toggledOn = true;
    }

    if (keyIsPressed(keyState)) {
        Kaleidoscope.hid().keyboard().pressKey(lastKey, toggledOn);
    }
}

static void toggleQwertyColemak(uint8_t keyState) {
    static uint8_t baseLayout;
    if (!keyToggledOn(keyState)) {
        return;
    }

    if (baseLayout == QWERTY) {
        baseLayout = COLEMAK;
    } else {
        // initialize to the lowest.
        baseLayout = QWERTY;
    }

    Layer.move(baseLayout);
}

/** macroAction dispatches keymap events that are tied to a macro
    to that macro. It takes two uint8_t parameters.

    The first is the macro being called (the entry in the 'enum' earlier in this
   file). The second is the state of the keyswitch. You can use the keyswitch
   state to figure out if the key has just been toggled on, is currently pressed
   or if it's just been released.

    The 'switch' statement should have a 'case' for each entry of the macro
   enum. Each 'case' statement should call out to a function to handle the macro
   in question.

 */

const macro_t* macroAction(uint8_t macroIndex, uint8_t keyState) {
    switch (macroIndex) {
    case MACRO_TOGGLE_QWERTY_COLEMAK:
        toggleQwertyColemak(keyState);
        break;

    case MACRO_VERSION_INFO:
        versionInfoMacro(keyState);
        break;

    case MACRO_ANY:
        anyKeyMacro(keyState);
        break;
    }

    return MACRO_NONE;
}

/** toggleLedsOnSuspendResume toggles the LEDs off when the host goes to sleep,
 * and turns them back on when it wakes up.
 */
void toggleLedsOnSuspendResume(kaleidoscope::plugin::HostPowerManagement::Event event) {
    switch (event) {
    case kaleidoscope::plugin::HostPowerManagement::Suspend:
        LEDControl.disable();
        break;
    case kaleidoscope::plugin::HostPowerManagement::Resume:
        LEDControl.enable();
        break;
    case kaleidoscope::plugin::HostPowerManagement::Sleep:
        // host-is-still-asleep tick event
        break;
    }
}

/** hostPowerManagementEventHandler dispatches power management events (suspend,
 * resume, and sleep) to other functions that perform action based on these
 * events.
 */
void hostPowerManagementEventHandler(kaleidoscope::plugin::HostPowerManagement::Event event) {
    toggleLedsOnSuspendResume(event);
}

/** This 'enum' is a list of all the magic combos used by the Model 01's
 * firmware The names aren't particularly important. What is important is that
 * each is unique.
 *
 * These are the names of your magic combos. They will be used by the
 * `USE_MAGIC_COMBOS` call below.
 */
enum {
    // Toggle between Boot (6-key rollover; for BIOSes and early boot) and NKRO
    // mode.
    COMBO_TOGGLE_NKRO_MODE
};

/** A tiny wrapper, to be used by MagicCombo.
 * This simply toggles the keyboard protocol via USBQuirks, and wraps it within
 * a function with an unused argument, to match what MagicCombo expects.
 */
static void toggleKeyboardProtocol(uint8_t combo_index) { USBQuirks.toggleKeyboardProtocol(); }

/** Magic combo list, a list of key combo and action pairs the firmware should
 * recognise.
 */
USE_MAGIC_COMBOS({.action = toggleKeyboardProtocol,
                  // Left Fn + Esc + Shift
                  .keys = {R3C6, R2C6, R3C7}});

// First, tell Kaleidoscope which plugins you want to use.
// The order can be important. For example, LED effects are
// added in the order they're listed here.
KALEIDOSCOPE_INIT_PLUGINS(

    EEPROMSettings, // EEPROM subsystem
    EEPROMKeymap,   // EEPROM backed keymap storage

    Focus,                // Serial subsystem
    FocusEEPROMCommand,   // EEPROM manipulation commands
    FocusHostOSCommand,   // HostOS serial command
    FocusLEDCommand,      // LED serial commands
    FocusSettingsCommand, // Basic serial commands
    LayerFocus,           // Layer manipulation serial command

    LEDControl,           // LED subsystem
    BootGreetingEffect,   // LED boot animation
    ActiveModColorEffect, // Color based on the active modifiers.
    LEDOff,               // LED deactivation at boot
    LEDRainbowEffect,     // Cross keyboard rainbow color effect
    LEDRainbowWaveEffect, // Whole keyboard rainbow color effect
    LEDChaseEffect,       // LEDs chasing LEDs for great fun
    LEDBreatheEffect,     // Keyboard might be alive - breaths in and out in color

    Macros,              // Macro support
    HostPowerManagement, // Host power state event hooks
    MagicCombo,          // chording and combination key presses

    // The USBQuirks plugin lets you do some things with USB that we aren't
    // comfortable - or able - to do automatically, but can be useful
    // nevertheless. Such as toggling the key report protocol between Boot (used
    // by BIOSes) and Report (NKRO).
    USBQuirks

);

/** The 'setup' function is one of the two standard Arduino sketch functions.
 * It's called when your keyboard first powers up. This is where you set up
 * Kaleidoscope and any plugins.
 */
void setup() {
    // First, call Kaleidoscope's internal setup function
    Kaleidoscope.setup();

    // Make the led button aqua when booting, its nicer looking than
    // blue.
    BootGreetingEffect.hue = 120;

    // We set the brightness of the rainbow effects to 150 (on a scale of 0-255)
    // This draws more than 500mA, but looks much nicer than a dimmer effect
    LEDRainbowEffect.brightness(200);
    LEDRainbowWaveEffect.brightness(200);

    // We want to make sure that the firmware starts with LED effects off
    // This avoids over-taxing devices that don't have a lot of power to share
    // with USB devices
    LEDOff.activate();

    // To make the keymap editable without flashing new firmware, we store
    // additional layers in EEPROM. For now, we reserve space for five layers. If
    // one wants to use these layers, just set the default layer to one in EEPROM,
    // by using the `settings.defaultLayer` Focus command.
    EEPROMKeymap.setup(5);
}

/** loop is the second of the standard Arduino sketch functions.
 * As you might expect, it runs in a loop, never exiting.
 *
 * For Kaleidoscope-based keyboard firmware, you usually just want to
 * call Kaleidoscope.loop(); and not do anything custom here.
 */

void loop() { Kaleidoscope.loop(); }
