#include "CyclopsPlugin.h"
#include "CyclopsPluginInfo.h"

namespace cyclops
{

Event::Event(): eventType(-1), message(nullptr), samplePosition(-1)
{}

Event::Event(int etype, MidiMessage* msg, int samplePos/* = 0 */)
: eventType(etype),
  message(msg),
  samplePosition(samplePos)
{}

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
