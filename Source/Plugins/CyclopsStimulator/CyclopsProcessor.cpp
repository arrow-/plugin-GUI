
/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "CyclopsProcessor.h"

namespace cyclops {

int CyclopsProcessor::node_count = 0;

CyclopsProcessor::CyclopsProcessor() : GenericProcessor("Cyclops Stimulator")
                                     , isOrphan(false)
                                     , isPrimed(false)
                                     , isParticipating(false)
{
    node_count++;
}

CyclopsProcessor::~CyclopsProcessor()
{
    node_count--;
    //std::cout<<"deleting clproc"<<std::endl;
}

/**
    If the processor uses a custom editor, this method must be present.
*/
AudioProcessorEditor* CyclopsProcessor::createEditor()
{
    editor = new CyclopsEditor(this, true);
    return editor;
}

void CyclopsProcessor::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
}

void CyclopsProcessor::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
    if (isParticipating)
        ;//checkForEvents(events);
}

bool CyclopsProcessor::isReady()
{
    CyclopsEditor* cl_editor = static_cast<CyclopsEditor*>(editor.get());
    int genError, buildError, flashError;

    cl_editor->isReadyForLaunch(isOrphan, isPrimed, genError, buildError, flashError);
    if (!isOrphan && isPrimed && cl_editor->isSerialConnected()){
        pluginInfo = cl_editor->getPluginInfo();
        serialInfo = cl_editor->getSerial();
        plugin = pluginInfo->CyclopsPluginFactory();

        isParticipating = true;
    }
    channelMap = cl_editor->getChannelMap();

    //DBG (nodeId << " > orphan primed conclude (gen, build, flash) : " << isOrphan << isPrimed  << isParticipating << " E(" << genError << ", " << buildError << ", " << flashError << ") " << "\n");
    return ((genError == 0 || genError == 15) &&
            (buildError == 0 || buildError == 15) &&
            (flashError == 0 || flashError == 15));
}

void CyclopsProcessor::timerCallback()
{
    plugin->timerTask();
}

void CyclopsProcessor::updateSettings()
{
    typeCount = CyclopsProcessor::analyseChannels(this);
}

bool CyclopsProcessor::enable()
{
    if (isParticipating){
        DBG ("~~~~~~~~~ Cyclops Sub Plugin enabled! ~~~~~~~~~");
        DBG ("Name (Hook-ID)         : " << pluginInfo->Name << " (" << nodeId << ")");
        DBG ("sources (signals)      : " << pluginInfo->signalCount);
        DBG ("event-channel-slots    : " << pluginInfo->slotCount);
        // DANGEROUS! because the experiment will be launched only after all
        // processors are enabled, timer should not start ASAP!?
        startTimer(pluginInfo->timePeriod);
        return true;
    }
    return false;
}

bool CyclopsProcessor::disable()
{
    stopTimer();
    isParticipating = false;
    return true;
}

int CyclopsProcessor::getProcessorCount()
{
    return node_count;
}

std::<ChannelType, std::pair<int, int> > CyclopsProcessor::analyseChannels(CyclopsProcessor* processor)
{
    std::map<ChannelType, std::pair<int, int> > typeCount;

    for (int i=0; i<processor->eventChannels.size(); i++){
        Channel* eventChannel = processor->eventChannels[i];
        ChannelType ctype = eventChannel->getType();
        std::map<ChannelType, std::pair<int, int> >::iterator it = typeCount.find(ctype);
        if (it == typeCount.end())
            typeCount[ctype] = std::pair<int, int>(i, 0);
        else
            it->second.second += 1;
    }
    return typeCount;
}

} // NAMESPACE cyclops