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

#ifndef CYCLOPS_EDITOR_H
#define CYCLOPS_EDITOR_H

#include <VisualizerEditorHeaders.h>
#include "CyclopsCanvas.h"
#include "CyclopsProcessor.h"

#include <string>
#include <iostream>


namespace cyclops{

class CyclopsProcessor;
class CyclopsCanvas;
class IndicatorLED;

/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                              CYCLOPS-EDITOR                              |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */

class CyclopsEditor : public VisualizerEditor
                    , public ComboBox::Listener
                    , public CyclopsCanvas::Listener
{
public:
    
    /** The class constructor, used to initialize any members. */
    CyclopsEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors=false);

    /** The class destructor, used to deallocate memory */
    ~CyclopsEditor();

    /**
     * @brief      Creates a new canvas. _But, we don't use it!_
     * @details    VisualizerEditor calls this in buttonClicked() If someone
     *             changes VisualizerEditor, such that this is called from
     *             elsewhere, then Cyclops Plugin WILL FAIL  
     *             *(Failure is desired)*
     *
     * @warning    THIS METHOD SHOULD NEVER BE CALLED!
     *
     * @return     { description_of_the_return_value }
     */
    Visualizer* createNewCanvas();

    void buttonClicked(Button* button);

    /**
     * @brief      Called when the shared window is closed. Used to update the
     *             SelectorButton state of _other_ CyclopsEditors.
     * @details    Does not delete instance of DataWindow.
     */
    void windowClosed();

    /** This method executes whenever a custom button is pressed */
    void buttonEvent(Button* button);

    /** Combobox listener callback, called when a combobox is changed. */
    void comboBoxChanged(ComboBox* box);

    void timerCallback();
    void paint(Graphics& g);

    /** Disables all input widgets on the editor. */
    void disableAllInputWidgets();
    /** Enables all input widgets on the editor. */
    void enableAllInputWidgets();

    /*
     * @brief      Determines if processor is ready for launch.
     * @details    Call Graph:
     *             * ProcessorGraph::enableProcessors {loops through all
     *               processors}
     *             * CyclopsProcessor.isReady
     *             * _this()_
     *             * CyclopsCanvas::getSummary(int, bool&)
     *             * CyclopsCanvas::generateCode {if getSummary() returns
     *               ``true``}
     *             * CyclopsCanvas::flashDevice {if generateCode() returns
     *               ``true``}
     *
     * @param      isOrphan    Indicates if _orphan_, ie. not attached to any
     *                         canvas/cyclops device.
     * @param      isPrimed    Indicates if completely configured (input
     *                         channel, plugin, signals, led port)
     * @param      genError    The error identifier in case of any error during
     *                         code-generation.
     * @param      buildError  The error identifier in case of any error during
     *                         compilation of the generated code.
     * @param      flashError  The error identifier in case of any error during
     *                         flashing the device.
     */
    void isReadyForLaunch(bool& isOrphan, bool& isPrimed, int& genError, int& buildError, int& flashError);
    bool isSerialConnected();

    /** Called to inform the editor that acquisition is about to start*/
    void startAcquisition();

    /** Called to inform the editor that acquisition has just stopped*/
    void stopAcquisition();

    /** Called whenever there is a change in the signal chain or it refreshes.
        It's called after the processors' same named method.
    */
    void updateSettings();
    
    /**
     * @brief      Called by CyclopsCanvas when the ``readinessLED`` Indicator needs to be
     *             updated.
     * @details    Color    | Description
     *             -------- | -----------
     *             Red      | Not ready, atleast 1 sub-plugin option not configured.
     *             Green    | All sub-plugin options configured. Code generated.
     *             Orange   | Code generation / Flashing failed or Device did not respond.
     *             Black    | Editor is disabled / orphaned.
     *
     * @param[in]  event  The canvas event.
     */
    void updateReadinessIndicator(CanvasEvent event, int attribute);

    /**
     * @brief      Called by CyclopsCanvas when the ``serialLED`` Indicator needs to be
     *             updated.
     * @details    Color    | Description
     *             -------- | -----------
     *             Red      | Serial Port not configured and/or configured
     *             Green    | Serial Port properly configured, device identified and ready.
     *             Blue     | Ambiguous state, serial device connected but _might_ not be ready/configured.
     *             Black    | Editor is disabled / orphaned.
     */
    void updateSerialIndicator(CanvasEvent event);


    CyclopsPluginInfo* refreshPluginInfo();
    bool channelMapStatus();
    void changeCanvas(CyclopsCanvas* dest);

    /**
     * @brief      Updates the WindowSelector buttons on the top right.
     */
    void updateButtons(CanvasEvent whichButton, bool state);
    void setInteractivity(CanvasEvent interactivity);
    int  getEditorId();

    cl_serial* getSerial();

    void saveEditorParameters(XmlElement* xmlNode);
    void loadEditorParameters(XmlElement* xmlNode);

private:

    int prepareCanvasComboList(ComboBox* combobox);
    void updateSelectorButtons();
    
    CyclopsCanvas* connectedCanvas;      /**< Pointer to the canvas which this editor connects */
    ScopedPointer<ComboBox> canvasCombo; /**< Cyclops Board chooser drop-down */
    CyclopsProcessor* processor;         /**< Parent Processor node */
    ScopedPointer<Label> comboText;
    ScopedPointer<Label> myID;           /**< The Processor ID provided by OE core */

    ScopedPointer<IndicatorLED> serialLED;
    ScopedPointer<IndicatorLED> readinessLED;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsEditor);
};



/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                              INDICATOR-LED                               |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */
class IndicatorLED : public Component
                   , public SettableTooltipClient
{
public:
    IndicatorLED (const Colour& fill, const Colour& line);
    void paint (Graphics& g);
    void update (const Colour& fill, String tooltip);
    void update (const Colour& fill, const Colour& line, String tooltip);
private:
    Colour fillColour, lineColour;
};

} // NAMESPACE cyclops
#endif
