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
#include <SpikeLib.h>
#include "CyclopsCanvas.h"
#include "CyclopsProcessor.h"

#include <string>
#include <iostream>


namespace cyclops{

class CyclopsProcessor;
class CyclopsCanvas;
class IndicatorLED;
class ChannelMapperDisplay;
class MapperWindowDisplay;
class SimpleWindow;

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

    /**
     * @brief      Gets the sub-plugin information from the Canvas.
     *
     * @return     The plugin information ``struct``.
     */
    CyclopsPluginInfo* getPluginInfo();

    /**
     * @brief      Gets the sub-plugin information from the canvas and updates
     *             (as well as *resets*) the ChannelMapper sliders.
     * @details    Invoked automatically when a plugin is selected with the drop-down.
     * @sa         CyclopsCanvas::unicastPluginSelected
     */
    void refreshPluginInfo();

    bool channelMapStatus();
    Array<int> getChannelMap();
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

    ScopedPointer<ChannelMapperDisplay> channelMapper;
private:

    int prepareCanvasComboList(ComboBox* combobox);
    void updateSelectorButtons();
    
    CyclopsCanvas* connectedCanvas;      /**< Pointer to the canvas which this editor connects */
    ScopedPointer<ComboBox> canvasCombo; /**< Cyclops Board chooser drop-down */
    CyclopsProcessor* processor;         /**< Parent Processor node */
    ScopedPointer<Label> comboText;
    ScopedPointer<ImageButton> mapperButton;

    ScopedPointer<Viewport> cmapViewport;
    ScopedPointer<SimpleWindow> mapperWindow;
    ScopedPointer<MapperWindowDisplay> mapperWindowDisplay;

    ScopedPointer<IndicatorLED> serialLED;
    ScopedPointer<IndicatorLED> readinessLED;

    static Image normal, down;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CyclopsEditor);
};



/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                              INDICATOR-LED                               |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */
/**
 * @brief      A small status Indicator on the CyclopsEditor, just colour and a
 *             tootip.
 * @details    IndicatorLED is stateless.
 */
class IndicatorLED : public Component
                   , public SettableTooltipClient
{
public:
    IndicatorLED (const Colour& fill, const Colour& line);
    void paint (Graphics& g);
    /**
     * @brief      Sets the colour and tooltip.
     *
     * @param[in]  fill     The fill colour.
     * @param[in]  tooltip  The tooltip string.
     */
    void update (const Colour& fill, String tooltip);
    /**
     * @brief      Sets the fill colour, outline colour and the tooltip.
     *
     * @param[in]  fill     The fill colour.
     * @param[in]  line     The outline colour.
     * @param[in]  tooltip  The tooltip string.
     */
    void update (const Colour& fill, const Colour& line, String tooltip);
private:
    Colour fillColour, lineColour;
};


/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                         CHANNEL-MAPPER-DISPLAY                           |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */
class ChannelMapperDisplay : public Component,
                             public Slider::Listener
{
public:
    ChannelMapperDisplay (CyclopsEditor* editor, CyclopsCanvas* canvas);
    void configure(BubbleMessageComponent* b);
    void update(CyclopsPluginInfo* pluginInfo);
    bool status();
    void paint(Graphics& g);
    void resized();

    void sliderValueChanged(Slider* s);
    void sliderDragStarted(Slider* s);
    void sliderDragEnded(Slider* s);

    Array<int> channelMap;
    /**
     * Maps ChannelType to (start_id, count)
     * @warning    This is stupid and bound to fail. Can be fixed only be
     *             redesigning the Channel system.
     */
    std::map<ChannelType, std::pair<int, int> > typeCount;
    CyclopsPluginInfo* pluginInfo;
private:
    CyclopsEditor* editor;
    CyclopsCanvas* canvas;
    OwnedArray<Slider> selectors;
    OwnedArray<Label> slotDetailLabels;
    BubbleMessageComponent* bubble;
};

/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                           MAPPER-WINDOW-DISPLAY                          |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */

class MapperWindowDisplay : public Component
{
public:
    MapperWindowDisplay(CyclopsEditor* editor, Viewport* viewport);
    void update();
    void resized();
    void paint(Graphics& g);

    int numChannelTypes;
    static StringArray ChannelNames;
    ScopedPointer<BubbleMessageComponent> bubble;
private:
    ScopedPointer<Label> chNameLabel;
    ScopedPointer<Label> chCountLabel;
    ScopedPointer<Label> helpLabel;
    CyclopsEditor* editor;
    Viewport* viewport;
};

/* +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 * |                               SIMPLE-WINDOW                              |
 * +~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~+
 */
/**
 * @brief      Used to hold the ChannelMapperDisplay for the CyclopsEditor
 * @details    Instance is destroyed when the CyclopsEditor is destroyed. Close
 *             operation just hides it from view.
 * @warning    This window should not be deleted by any code (esp. callback
 *             function).
 */
class SimpleWindow : public DocumentWindow
{
public:

    /**
     * @brief      Creates the window, but does not show it.
     *
     * @param[in]  title             The title
     * @param[in]  content           The content Component
     * @param[in]  closeCallback_fn  The callback function that is executed when
     *                               a window is closed.
     */
    SimpleWindow(Component* canvas, const String& title, Component* content, const void (*closeCallback_fn)(DocumentWindow* dw)=nullptr);

    /**
     * @brief      Captures the event of pressing the close button.
     * @details    Invokes the callback and hides the window.
     */
    void closeButtonPressed();
    
    /**
     * @brief      To capture ``ESCAPE`` key
     *
     * @param[in]  keyPress  The KeyPress instance.
     *
     * @return     Always ``false``, so that the event passes to "parents".
     */
    bool keyPressed(const KeyPress& key);
private:
    Component* content;
    /**
     * @brief      Called when ``ESCAPE`` key is pressed, or close button is pressed.
     * @details    After this callback is finished, we simply hide the window.
     * @warning    Do not delete the DocumentWindow in the callback function.
     */
    const void (*closeCallback)(DocumentWindow* dw);
};

} // NAMESPACE cyclops
#endif
