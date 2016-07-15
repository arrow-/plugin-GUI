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

#include "CyclopsCanvas.h"

namespace cyclops {

OwnedArray<CyclopsCanvas> CyclopsCanvas::canvasList;
std::map<std::string, cl_serial> CyclopsCanvas::SerialMap;
const int CyclopsCanvas::BAUDRATES[12] = {300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400};

CyclopsCanvas::CyclopsCanvas(CyclopsEditor* editor) : tabIndex(-1)
                                                    , dataWindow(nullptr)
                                                    , progress(0)
                                                    , in_a_test(false)
{
    // Add "port" list
    portCombo = new ComboBox();
    portCombo->addListener(this);
    portCombo->setTooltip("Select the serial port on which Cyclops is connected.");
    portCombo->addItemList(getDevices(), 1);
    addAndMakeVisible(portCombo);

    // Add refresh button
    refreshButton = new UtilityButton("R", Font("Default", 9, Font::plain));
    refreshButton->setRadius(3.0f);
    refreshButton->addListener(this);
    addAndMakeVisible(refreshButton);

    // Add baudrate list
    baudrateCombo = new ComboBox();
    baudrateCombo->addListener(this);
    baudrateCombo->setTooltip("Set the baud rate (115200 recommended).");

    Array<int> baudrates(getBaudrates());
    for (int i = 0; i < baudrates.size(); i++)
    {
        baudrateCombo->addItem(String(baudrates[i]), baudrates[i]);
    }
    baudrateCombo->setSelectedId(115200);
    addAndMakeVisible(baudrateCombo);
    // Add TEST buttons
    for (int i=0; i < 4; i++){
        testButtons.add(new UtilityButton(String("Test ") + String(i), Font("Default", 11, Font::bold)));
        testButtons[i]->addListener(this);
        addAndMakeVisible(testButtons[i]);
    }
    progressBar = new ProgressBar(progress);
    progressBar->setPercentageDisplay(false);
    addChildComponent(progressBar);
    pstep = 0.01;
    // communicate with teensy.
    
}

CyclopsCanvas::~CyclopsCanvas()
{
    //std::cout<<"deleting clcan"<<std::endl;
}

void CyclopsCanvas::beginAnimation()
{
    std::cout << "CyclopsCanvas beginning animation." << std::endl;

    startCallbacks();
}

void CyclopsCanvas::endAnimation()
{
    std::cout << "CyclopsCanvas ending animation." << std::endl;

    stopCallbacks();
}

void CyclopsCanvas::setParameter(int x, float f)
{

}

void CyclopsCanvas::setParameter(int a, int b, int c, float d)
{
}

void CyclopsCanvas::update()
{
    std::cout << "Updating CyclopsCanvas" << std::endl;
}


void CyclopsCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void CyclopsCanvas::resized()
{
    int width = getWidth(), height = getHeight();
    baudrateCombo->setBounds(width-75, 5, 70, 20);
    portCombo->setBounds(width-75-5-60, 5, 60, 20);
    refreshButton->setBounds(width-75-5-60-5-20, 5, 20, 20);
    for (int i=0; i < 4; i++){
        testButtons[i]->setBounds(jmax(100, width-53), jmax(45, (height/5))*(i+1)-(25/2), 50, 25);
    }
    progressBar->setBounds(2, height-16, width-4, 16);
}

/*
void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
}
*/
void CyclopsCanvas::refresh()
{
    repaint();
}

void CyclopsCanvas::disableAllInputWidgets()
{
    // Disable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(false);
    portCombo->setEnabled(false);
    baudrateCombo->setEnabled(false);
    refreshButton->setEnabled(false);
}

void CyclopsCanvas::enableAllInputWidgets()
{
    // Reenable the whole gui
    for (int i=0; i<4; i++)
        testButtons[i]->setEnabled(true);
    portCombo->setEnabled(true);
    baudrateCombo->setEnabled(true);
    refreshButton->setEnabled(true);
}


void CyclopsCanvas::paint(Graphics& g)
{
    g.fillAll(Colours::darkgrey);
    if (!in_a_test)
        progressBar->setVisible(false);
}

void CyclopsCanvas::buttonClicked(Button* button)
{  
    int test_index = -1;
    for (int i=0; i < 4; i++){
        if (button == testButtons[i]){
            test_index = i;
            break;
        }
    }
    if (test_index >= 0){
        disableAllInputWidgets();
        std::cout << "Testing LED channel " << test_index << "\n";
        in_a_test = true;
        //private_serial.testChannel(test_index);
        progressBar->setTextToDisplay("Testing LED channel" + String(test_index));
        progressBar->setVisible(true);
        startTimer(20);
        test_index = -1;
        //for (auto& )
    }
    if (button == refreshButton)
    {
        // Refresh list of devices
        portCombo->clear();
        portCombo->addItemList(getDevices(), 1);
    }
}

void CyclopsCanvas::comboBoxChanged(ComboBox* comboBox)
{
    if (comboBox == portCombo)
    {
        setDevice(comboBox->getText().toStdString());
    }
    else if (comboBox == baudrateCombo)
    {
        setBaudrate(comboBox->getSelectedId());
    }
}

bool CyclopsCanvas::keyPressed(const KeyPress& key)
{
    return true;
}

void CyclopsCanvas::timerCallback()
{
    if (in_a_test){
        progress += pstep;
        if (progress >= 1.0){
            progressBar->setVisible(false);
            progress = 0;
            in_a_test = false;
            stopTimer();
            enableAllInputWidgets();
        }
    }
}

bool CyclopsCanvas::screenLikelyNames(const String& portName)
{
    #ifdef TARGET_OSX
        return portName.contains("cu.") || portName.contains("tty.");
    #endif
    #ifdef TARGET_LINUX
        return portName.contains("ttyUSB") || portName.contains("ttyA");
    #endif
    return true; // for TARGET_WIN32
}

StringArray CyclopsCanvas::getDevices()
{
    vector<ofSerialDeviceInfo> allDeviceInfos = private_serial.getDeviceList();
    StringArray allDevices;
    String portName;
    for (unsigned int i = 0; i < allDeviceInfos.size(); i++)
    {
        portName = allDeviceInfos[i].getDeviceName();
        if (screenLikelyNames(portName))
        {
            allDevices.add(portName);
        }
    }
    return allDevices;
}

Array<int> CyclopsCanvas::getBaudrates()
{
    Array<int> allBaudrates(BAUDRATES, 12);
    return allBaudrates;
}

void CyclopsCanvas::setDevice(string port)
{
    ;
}

void CyclopsCanvas::setBaudrate(int baudrate)
{
    ;
}

void CyclopsCanvas::pushEditor(CyclopsEditor* editor)
{
    registeredEditors.push_front(editor);
}

void CyclopsCanvas::popEditor(CyclopsEditor* editor)
{
    registeredEditors.remove(editor);
}

const std::forward_list<CyclopsEditor*>& CyclopsCanvas::getRegisteredEditors() const
{
    return registeredEditors;
}

void CyclopsCanvas::saveVisualizerParameters(XmlElement* xml)
{
    XmlElement* parameters = xml->createNewChildElement("PARAMETERS");
    parameters->setAttribute("device", portCombo->getText().toStdString());
    parameters->setAttribute("baudrate", baudrateCombo->getSelectedId());
}

void CyclopsCanvas::loadVisualizerParameters(XmlElement* xml)
{
    forEachXmlChildElement(*xml, subNode) {
        if (subNode->hasTagName("PARAMETERS")) {
            portCombo->setText(subNode->getStringAttribute("device", ""));
            baudrateCombo->setSelectedId(subNode->getIntAttribute("baudrate"));
        }
    }
}

} // NAMESPACE cyclops
