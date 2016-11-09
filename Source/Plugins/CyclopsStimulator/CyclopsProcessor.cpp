
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

int CyclopsProcessor::checkForEvents (MidiBuffer& midiMessages)
{
    Array<Array<Event> > slotEvents;
    Array<Event> others;
    if (midiMessages.getNumEvents() > 0)
    {
        // int m = midiMessages.getNumEvents();
        //std::cout << m << " ev ents received by node " << getNodeId() << std::endl;

        MidiBuffer::Iterator i (midiMessages);
        MidiMessage message (0xf4);

        int samplePosition = 0;
        i.setNextSamplePosition (samplePosition);
        /*
        while (i.getNextEvent (message, samplePosition))
        {
            const uint8* dataptr = message.getRawData();
            switch (*dataptr){
                case TTL:
                    eventChannel = *(dataptr + 3);
                    break;
            }
        }
        */
    }

    return -1;
}

bool CyclopsProcessor::isReady()
{
    CyclopsEditor* cl_editor = dynamic_cast<CyclopsEditor*>(editor.get());
    int genError, buildError, flashError;

    cl_editor->isReadyForLaunch(isOrphan, isPrimed, genError, buildError, flashError);
    if (!isOrphan && isPrimed && cl_editor->isSerialConnected()){
        pluginInfo = cl_editor->getPluginInfo();
        serialInfo = cl_editor->getSerial();
        plugin = pluginInfo->CyclopsPluginFactory();

        isParticipating = true;
    }
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
    for (auto& eventChannel : eventChannels){
        //std::cout << eventChannel->getName() << ", type: ";
        ChannelType ctype = eventChannel->getType();
        switch (ctype){
            case HEADSTAGE_CHANNEL:
                //std::cout << "HEADSTAGE_CHANNEL";
                break;
            case AUX_CHANNEL:
                //std::cout << "AUX_CHANNEL";
                break;
            case ADC_CHANNEL:
                //std::cout << "ADC_CHANNEL";
                break;
            case EVENT_CHANNEL:
                //std::cout << "EVENT_CHANNEL";
                break;
            case ELECTRODE_CHANNEL:
                //std::cout << "ELECTRODE_CHANNEL";
                break;
            case MESSAGE_CHANNEL:
                //std::cout << "MESSAGE_CHANNEL";
                break;
        }
    }
}

bool CyclopsProcessor::enable()
{
    if (isParticipating){
        DBG ("~~~~~~~~~ Cyclops Sub Plugin enabled! ~~~~~~~~~");
        DBG ("Name (Hook-ID)    : " << pluginInfo->Name << " (" << nodeId << ")");
        DBG ("sources (signals) : " << pluginInfo->signalCount);
        DBG ("input-channels    : " << pluginInfo->channelCount);
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

} // NAMESPACE cyclops