
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
ScopedPointer<CyclopsPluginManager> CyclopsProcessor::pluginManager = new CyclopsPluginManager();

CyclopsProcessor::CyclopsProcessor()
    : GenericProcessor("Cyclops Stimulator")
{
    node_count++;
    if (pluginManager->getNumPlugins() == 0){
        std::cout << "CPM> Making Cyclops-Plugin List" << std::endl;
        pluginManager->loadAllPlugins();
        std::cout << "CPM> Loaded " << pluginManager->getNumPlugins() << " cyclops plugin(s)." << std::endl;
    }
}

CyclopsProcessor::~CyclopsProcessor()
{
    node_count--;
    //std::cout<<"deleting clproc"<<std::endl;
}

void CyclopsProcessor::updateSettings()
{
    ;
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

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);
    //threshold = newValue;
    //std::cout << float(p[0]) << std::endl;
    editor->updateParameterButtons(parameterIndex);
}

void CyclopsProcessor::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
    checkForEvents(events);
}

void CyclopsProcessor::handleEvent(int eventType, MidiMessage& event, int samplePosition/* = 0 */)
{
    ;    
}

bool CyclopsProcessor::isReady()
{
    if (!dynamic_cast<CyclopsEditor*>(editor.get())->isReady())
    {
        editor->makeVisible();
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Prepare this hook", "Please configure this Cyclops Stimulator-hook correctly!");
        return false;
    }
    return true;
}

bool CyclopsProcessor::enable()
{
    //Serial->close();
    return true;
}

bool CyclopsProcessor::disable()
{
    //Serial->close();
    return true;
}

int CyclopsProcessor::getProcessorCount()
{
    return node_count;
}

} // NAMESPACE cyclops