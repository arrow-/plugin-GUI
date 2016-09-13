#ifndef OE_CYCLOPS_PLUGIN_H
#define OE_CYCLOPS_PLUGIN_H

#ifdef WIN32
    #ifdef OE_CL_PLUGIN
        #define CL_SUBPLUGIN_API __declspec(dllimport)
    #else
        #define CL_SUBPLUGIN_API __declspec(dllexport)
    #endif
#else
    #define CL_SUBPLUGIN_API __attribute__((visibility("default")))
#endif

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "../CyclopsAPI/CyclopsAPI.h"

namespace cyclops{

class CL_SUBPLUGIN_API CyclopsPluginInfo;

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsSource                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

enum class operationMode;
enum class sourceType;

/**
 * @brief      A mirror of the Source Object on the teensy
 */
struct CyclopsSource
{
    operationMode    opMode;
    const int        src_id;
    const sourceType type;
};

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsPlugin                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

class CL_SUBPLUGIN_API CyclopsPlugin
{
    enum class pluginStatus : int
    {
        CHANNEL_INITIALIZED     = 0,
        SRC_NAMES_INITIALIZED   = 1
    };
private:
    int readiness;
    CyclopsPluginInfo* info;

public:
    /**
     * This array defines a mapping from _plugin-code-channel_ to
     * _OE-data-channel_, and is filled automatically by the GUI according to
     * your inputs on the editor window.
     *
     * This lets you select which data channels from the GUI data flow are
     * directed into your plugin
     */
    Array<int> Channels;

    CyclopsPlugin();
    /**
     * @brief      Contructs a Cyclops Plugin.
     * @details    Your code will never call it, don't worry about this
     *             function. This is called by the Cyclops Stimulator node when
     *             you select this plugin from the drop down.
     *
     * @param      info_struct  This holds the channel and source counts, source
     *                          names, etc.
     */

    /**
     * @brief      Sets the channel IDs as per those filled in the editor.
     *
     * @param[in]  channelIDs  Sets CyclopsPlugin::Channels
     */
    void setChannels(const int channelIDs[]);

    bool isReady();
    bool enable();
    bool disable();

    /**
     * @brief      This is where your CL-Plugin must read the event buffer and
     *             process events.
     *
     * @param[in]  eventType       The event type
     * @param      event           The event buffer
     * @param[in]  samplePosition  The sample position
     */
    virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0) = 0;

    /**
     * @brief      This is where your CL-plugin can do periodic tasks. You
     *             cannot change the timer period or stop it (as of now?).
     * @note       The time period can be set in CyclopsPluginInfo.cpp
     */
    virtual void timerTask() = 0;
};

} // NAMESPACE cyclops

#endif