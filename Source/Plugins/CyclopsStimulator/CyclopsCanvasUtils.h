#ifndef CYCLOPS_STIM_CANVAS_UTILS_H
#define CYCLOPS_STIM_CANVAS_UTILS_H

#include <EditorHeaders.h>
#include <SerialLib.h>
#include <string>
#include <vector>
#include <bitset>
#include <map>
#include <algorithm>

#include "plugin_manager/CLPluginManager.h"
#include "code_gen/Programs.h"

namespace cyclops{
  namespace CyclopsColours{
    const Colour disconnected(0xffff3823);
    const Colour notVerified(0xff3d64ff);
    const Colour connected(0xffc1d045);
    const Colour errorGenFlash(0xffff7400);
    const Colour notReady       = disconnected;
    const Colour Ready          = connected;
    const Colour pluginSelected = notVerified;
    }

enum class CanvasEvent{
    WINDOW_BUTTON,
    TAB_BUTTON,
    COMBO_BUTTON,
    SERIAL,
    PLUGIN_SELECTED,
    CODE_GEN,
    BUILD,
    FLASH,
    TRANSFER_DROP,
    FREEZE,
    THAW,
};

struct cl_serial
{
    std::string portName;
    ScopedPointer<ofSerial> Serial;
    int baudRate;

    cl_serial()
    {
        portName = "";
        Serial = new ofSerial();
        baudRate = -1;
    }
};

} // NAMESPACE cyclops

#endif