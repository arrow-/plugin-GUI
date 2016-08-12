#ifndef CYCLOPS_STIMULATOR_PLUGIN_H
#define CYCLOPS_STIMULATOR_PLUGIN_H
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
    int sourceCount;
    int channelCount;
    int timePeriod;
    std::vector<std::string> sourceCodeNames;
    std::vector<sourceType>  sourceCodeTypes;
    CyclopsPlugin* (*CyclopsPluginFactory)();
};

typedef void (*CLPluginInfoFunction)(CyclopsPluginInfo&);

} // NAMESPACE cyclops

#endif