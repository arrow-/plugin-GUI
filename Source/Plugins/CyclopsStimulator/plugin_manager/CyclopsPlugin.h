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
 * @brief      A mirror of the Source Object on the Cyclops Device.
 * @details    Let's the sub-plugin code keep track of the Source objects on the
 *             Cyclops Device.
 */
struct CyclopsSource
{
    operationMode    opMode;
    const int        src_id;
    const sourceType type;
};

/**
 * @brief      Event
 */
struct Event
{
    int eventType;
    MidiMessage* message;
    int samplePosition;
};

/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                                 CyclopsPlugin                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/
/**
 * @brief      The Cyclops sub-plugin base class.
 * @details    Each sub-plugin is executed in a CyclopsProcessor. It takes
 *             event-streams (_OE GUI event channels_) from the
 *             CyclopsProcessor, performs CyclopsPlugin::handleEvent over them,
 *             and invokes @ref ns-cyclops-api "CyclopsAPI" calls.
 *
 *             The sub-plugin code expects that certain Source objects are
 *             available at the Cyclops Device at runtime, but these Sources are
 *             not hard-coded into the code. The user must choose a suitable
 *             Signal object on the CyclopsCanvas (HookView).
 *
 *             Each sub-plugin recieves **all** event-streams from the
 *             CyclopsProcessor. Since the sub-plugin function is highly
 *             specialised, it would probably work with just a few of these
 *             event-streams. Instead of hard-coding which event-streams are to
 *             be acted upon (since their relative/absolute positions might
 *             change whenever organisms are rewired/experiment is altered, the
 *             ChannelMapperDisplay of the CyclopsEditor can be used to
 *             additionally "route" a few of the OE GUI event channels into
 *             the sub-plugin's **event-stream slots**.
 *
 *             The sub-plugin code can now assume that say, event-stream from
 *             ``slot[0]`` is always ``electrode k`` from _some_ organism. This
 *             allows you to drop and chain multiple sub-plugin instances, which
 *             perform (same or different) function, but operate over different
 *             sets of OE GUI event channels -- by choosing the
 *             "set-of-channels" for each sub-plugin on it's
 *             ChannelMapperDisplay, at runtime.
 * @see        CyclopsPluginInfo
 */
class CL_SUBPLUGIN_API CyclopsPlugin
{
private:
    CyclopsPluginInfo* info;
    /**
     * This array defines a mapping from _plugin-code-channel_ to
     * _OE-data-channel_, and is filled automatically by the GUI according to
     * your inputs on the editor window.
     *
     * This lets you select which data channels from the GUI data flow are
     * directed into your plugin.
     */
    Array<int> slotChannels;
public:

    /**
     * @brief      Contructs a Cyclops Plugin.
     * @details    Your code will never call it, don't worry about this
     *             function. This is called by the Cyclops Stimulator node when
     *             you select this plugin from the drop down.
     *
     * @param      info_struct  This holds the channel and source counts, source
     *                          names, etc.
     */
    CyclopsPlugin();

    /**
     * @brief      Sets the channel IDs as per those filled in the editor.
     * @details    This array defines a mapping from _plugin-code-channel_ to
     *             _OE-data-channel_, and is filled automatically by the GUI
     *             according to your inputs on the editor window.
     *
     *             This lets you select which data channels from the GUI data
     *             flow are directed into your plugin.
     *
     * @param[in]  slot_channels  Sets CyclopsPlugin::slotChannels
     */
    void setSlotChannels(const Array<int> slot_channels);

    bool isReady();
    bool enable();
    bool disable();

    /**
     * @brief      This is where the sub-plugin logic lies, and is automatically
     *             invoked by CyclopsProcessor.
     * @param[in]  slotStreams  Each ``kᵗʰ`` element is an Array<Event> of events
     *                          captured in
     *                          ``(CyclopsPlugin::slotChannels[k])ᵗʰ`` OE GUI
     *                          event-channel.
     * @sa         setSlotChannels, CyclopsPlugin, handleOtherEvents
     */
    virtual void handleSlotEvents(Array<Array<Event> > slotStreams) = 0;

    /**
     * @brief      Events from other channels are collected in eventStream, and
     *             can be processed here.
     *
     * @param[in]  eventStream  An Array of Event objects.
     */
    virtual void handleOtherEvents(Array<Event> eventStream);

    /**
     * @brief      This is where your CL-plugin can do periodic tasks. You
     *             cannot change the timer period or stop it (as of now?).
     * @note       The time period can be set in CyclopsPluginInfo.cpp
     */
    virtual void timerTask() = 0;
};

} // NAMESPACE cyclops

#endif