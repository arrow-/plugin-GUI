#ifndef CL_PLUGIN_INFO_H
#define CL_PLUGIN_INFO_H
/*+~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
  |                             CyclopsPluginInfo                                      |
  +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
*/

#include "../../../../JuceLibraryCode/JuceHeader.h"
#include "CyclopsPlugin.h"
#include <vector>
#include <string>

namespace cyclops{

struct CyclopsPluginInfo
{
    std::string Name;
    int signalCount;
    int channelCount;
    int timePeriod;

    std::vector<std::string> signalCodeNames;
    std::vector<sourceType>  sourceCodeTypes;
    int initialSignal;
    operationMode allInitialMode;

    CyclopsPlugin* (*CyclopsPluginFactory)();
};

typedef void (*CLPluginInfoFunction)(CyclopsPluginInfo&);

} // NAMESPACE cyclops

#endif