[TOC]

# OE-Cyclops Interface
I read the GUI code-base to understand how it functions, how the Buffers are passed, by whom and how events can be passed around (like `SpikeObjects` are).

### What triggers a stimulus signal?
1. An event, such as a Spike.
2. Frequency of the Low-Pass band in neural signal.
3. ???

Plugins are available / can be created to detect such `events`. But the `events` cannot be passed as such to the Cyclops board _(maybe they can?)_. The `events` are piped to a `stimulus-creator` (or whatever be it's name) processor.

### How is the stimulus signal created? [Part 1]
We can't say beforehand, that will depend on the experiment!
So, we need the `stimulus-creator` processor to process `events` into commands/data that Cyclops can understand.
This processor uses a user-written-program which responds to the `events`. These programs can be called "Closed Loop Experiment plugins".
Users can define their very own closed-loop experiments in this way.

`CLE` plugins process *any* type of events to generate a single stream of commands/data for **a single LED channel**. These programs would use the Cyclops API.

>Million Dollar questions:
Should data be piped into a `stimulus-creator`?
Is there a Base Event Class? We would need one if there will be different kinds of events...

#### Usage
1. Create a signal chain that would pipe `events` into a `stimulus-creator` .node (using `mergers`, if needed).
2. Choose a `CLE` plugin from the dropdown.
3. Map all event-channels to parameters of the plugin manually (UI detail).
4. Select the Cyclops board from dropdown / connect to a `Cyclops Sink`.
5. Select the LED channel.
6. A **a single LED channel** is now ready.

### How is the stimulus signal (actually) created? [Part 2]
1. The parameters of a stored Waveform on cyclops are controlled by `stimulus-creator::CLE_plugin`. Also, Teensy can be asked to "generate" waveforms [see below](#teensy-waveform-generation)
2. Real Time streaming of data-points:
For each data-point just voltage is sent. The LED voltage is updated at "uniform" width time-intervals.

>Let's try and avoid (2) because of bandwidth contraints in Serial transmission of real time multi channel data, though it's completely do-able otherwise.

I'll create a `CyclopsControlChannel` from `ChannelExtraData` and a `CyclopsControlObject` `struct` (like those in `SpikeObject.h`).
`CyclopsControlObjects` will be the output of the processor.

### Sending to Cyclops [Cyclops Sink]

Each `stimulus-creator` controls a single LED channel and upto 4 LEDs can be controlled on a Cyclops. A sink node is needed that can collect all `CyclopsControl` event buffers, serialise them and send to Cyclops via Serial Port ASAP. This node can be made in 2 ways,

1. All the `stimulus-creator` nodes connect to it as input, using `mergers`. Looks messy and `merger` chains are not easily made (I can give example).
2. Make a **hidden** "Cyclops Sink" node, like the default `audio` and `record` nodes (added to the `ProcessorGraph` at startup). This node be created, added and destroyed on-demand.
    - All `stimulus-creator` nodes would register themselves to it.
    - GUI elements for selection of the Cyclops Board _(in case of multiple Cyclops boards)_ and the LED channel can be kept on `stimulus-creator` Editor.
There's no need to clutter the viewport and signal chain.

>More Million Dollar questions:
What about messages from Cyclpos to OE? Are they forwarded to the `stimulus-creator`? I don't know how that can be done... probably with 2 hidden nodes, one for sending to Cyclops and another to dispatch messages from it...
In CASE 2, is the `stimulus-creator` a sink? (Probably not...)

![processor diagram](img/processors.png)
[New Processors](http://i.imgur.com/21pEQY0.png)

# Teensy Waveform generation
There is a hardware DSP available on the Teensy which can be used to synthesize simple waveform data-points (sawtooth, sine, noise, etc) using the [Audio](http://www.pjrc.com/teensy/td_libs_Audio.html) library.
We should be able to generate other waveforms too _(I need to brush up my DSP knowledge before that :smile:)._

Instead of creating analog audio from them, these data-points can be sent via SPI to the LED driver.

For Teensy DSP details see [the CMSIS DSP library](http://www.nxp.com/files/microcontrollers/doc/app_note/AN4489.pdf)

# Feedback Needed!

Apart from remarks on the design _(and the million $$ questions :wink:)_,

2. Is this interface simple enough for non-developers?
3. Should the stimulus logic programmable? If not, would a simple rule table suffice?
4. Will this bring a lot of overhead computation? Is there a faster approach?
5. Should just one `stimulus-creator` control all 4 LED channels?