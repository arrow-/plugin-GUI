#ifndef CL_PLUGIN_INFO_H
#define CL_PLUGIN_INFO_H
/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                             CyclopsPluginInfo                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "../../Headers/ProcessorHeaders.h"
#include "CyclopsPlugin.h"
#include <vector>
#include <string>

namespace cyclops{

/**
 * @brief      Contains all configuration info about this sub-plugin. Tells the
 *             GUI what and how to set everything up for this sub-plugin.
 * @details    This struct is filled by hand, by the user in each sub-plugin's
 *             ``CyclopsPluginInfo.cpp`` file.
 * @see        CyclopsPlugin
 */
struct CyclopsPluginInfo
{
    std::string Name; /**< Name of this sub-plugin. */
    int signalCount; /**< No. of signals (source objects) that this sub-plugin uses. */

    /**
     * @brief      Each string is the codename of the Signal objects used in the
     *             sub-plugin. These names appear on the CyclopsCanvas
     *             (HookView).
     * @details    The sub-plugin assumes that the user has selected the
     *             _actual_ Signal objects for each of these _codenames_ (on the
     *             CyclopsCanvas).
     *
     *             The sub-plugin code uses these _names_ freely, and
     *             essentially, this array maps those "uses" to actual Signal
     *             objects that are chosen on the CyclopsCanvas.
     */
    std::vector<std::string> signalCodeNames;

    /**
     * @brief      This defines what _type of Signal_ is expected for a
     *             particular %Signal codename.
     */
    std::vector<sourceType>  sourceCodeTypes;
    
    /**
     * This is the index of the Signal which must be used initially by the
     * Cyclops Device, when acquisition begins.
     */
    int initialSignal;
    /**
     * @brief      The operation mode of the "Initial Signal", when acquisition begins.
     * @sa         initialSignal
     */
    operationMode allInitialMode;

    /**
     * @brief      No. of OE GUI event channels that this sub-plugin listens to.
     * @sa         CyclopsPlugin
     */
    int slotCount;
    /**
     * @brief      Each element of this array is the type of the Event Channel
     *             that is expected for this slot.
     * @note       The length of this array must match slotCount.
     * @sa         ChannelType
     */
    std::vector<ChannelType> slotTypes;

    /**
    * The interval at which _the periodic task_ must be performed, if the
    * CyclopsPlugin::timerTask has been defined.
    */
    int timePeriod;

    CyclopsPlugin* (*CyclopsPluginFactory)();
};

typedef void (*CLPluginInfoFunction)(CyclopsPluginInfo&);

} // NAMESPACE cyclops

#endif