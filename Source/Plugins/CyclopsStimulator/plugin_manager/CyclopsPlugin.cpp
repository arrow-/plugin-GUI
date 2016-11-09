#include "CyclopsPlugin.h"
#include "CyclopsPluginInfo.h"

namespace cyclops
{

CyclopsPlugin::CyclopsPlugin()
{}

void CyclopsPlugin::setSlotChannels(Array<int> slot_channels){
    slotChannels = slot_channels;
}

void CyclopsPlugin::handleOtherEvents(Array<Event> eventStream)
{} 

bool CyclopsPlugin::isReady()
{
    return true;
}

bool CyclopsPlugin::enable()
{
    return true;
}

bool CyclopsPlugin::disable()
{
    return true;
}

} // NAMESPACE cyclops
