
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

CyclopsProcessor::CyclopsProcessor()
    : GenericProcessor("Cyclops Stimulator")
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
    checkForEvents(events);
}

void CyclopsProcessor::handleEvent(int eventType, MidiMessage& event, int samplePosition/* = 0 */)
{
    // std::cout << eventType << " " << std::endl;
    plugin->handleEvent(eventType, event, samplePosition);
}

bool CyclopsProcessor::isReady()
{
    CyclopsEditor* cl_editor = dynamic_cast<CyclopsEditor*>(editor.get());
    if (!cl_editor->isReady())
    {
        editor->makeVisible();
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Prepare this hook", "Please configure this (" + String(nodeId) + ") Cyclops Stimulator-hook correctly!");
        return false;
    }
    return true;
}

void CyclopsProcessor::updateSettings()
{
    
}

bool CyclopsProcessor::enable()
{
    CyclopsEditor* cl_editor = dynamic_cast<CyclopsEditor*>(editor.get());
    pluginInfo = cl_editor->refreshPluginInfo();
    serialInfo = cl_editor->getSerial();
    plugin = pluginInfo->CyclopsPluginFactory();
    if (pluginInfo == nullptr || plugin == nullptr || serialInfo == nullptr)
        return false;
    std::cout << "Name: " << pluginInfo->Name << std::endl;
    std::cout << "sources: " << pluginInfo->sourceCount << std::endl;
    std::cout << "channels: " << pluginInfo->channelCount << std::endl;
    return true;
}

bool CyclopsProcessor::disable()
{
    return true;
}

int CyclopsProcessor::getProcessorCount()
{
    return node_count;
}

} // NAMESPACE cyclops