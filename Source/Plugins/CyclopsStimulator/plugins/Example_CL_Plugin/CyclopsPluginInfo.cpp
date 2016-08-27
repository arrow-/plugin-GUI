#include "../plugin_manager/CyclopsPluginInfo.h"
#include "Example_CL_Plugin.h"
#include <string>
#include <vector>
#ifdef WIN32
#include <Windows.h>
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

/**
 * @brief      This function should return a pointer to an instance of your
 *             Cyclops SubPlugin.
 */
cyclops::CyclopsPlugin* maker_function()
{
    return new Example_CL_Plugin;
}

/**
 * @brief      Gets the cyclops plugin information.
 * @details    Here you have to specify the no. of LED channels that this plugin
 *             would control, the no. of "source" objects that need to be
 *             pre-loaded onto the Teensy (for this plugin).
 *
 *             Also specify the "names" of the Sources, this will be shown on
 *             the Cyclops-Stimulator canvas, where you can map these "names"
 *             with _actual_ Signals.
 *
 * @param      infoStruct  The information structure which needs to be filled.
 */
extern "C" EXPORT void getCyclopsPluginInfo(cyclops::CyclopsPluginInfo& infoStruct) {
    // Name of your Cyclops SubPlugin. This will appear on the GUI.
    infoStruct.Name = "Example_CL_Plugin";

    // The no. of LED channels that will be controlled.
    infoStruct.channelCount = 1;
    // The no. of Sources needed on the Teensy, should be same as length of the vector below.
    infoStruct.sourceCount = 4;

    // These are the "Code Names" of the sources (same as the enums you made in <your-plugin>.cpp)
    // These will appear on the GUI.
    infoStruct.sourceCodeNames = { "FastSquare"
                                 , "SlowSquare"
                                 , "Triangle"
                                 , "Sawtooth"};

    // This array holds the "type" information of the sources listed above.
    infoStruct.sourceCodeTypes = { cyclops::sourceType::SQUARE
                                 , cyclops::sourceType::SQUARE
                                 , cyclops::sourceType::STORED
                                 , cyclops::sourceType::STORED};

    infoStruct.CyclopsPluginFactory = maker_function;

    // If you wish to perform a periodic task on the GUI, during aquisition, you
    // must set the period here (in milli-seconds)
    infoStruct.timePeriod = 250; // ms
}

