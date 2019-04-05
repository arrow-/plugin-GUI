[TOC]

# Cyclops Plugins for reaction to events.
The `cyclops-stimulator` node is a sink. Each node can control upto 4 LEDs of a _single_ board. Multiple `cyclops-stimulator` nodes can exist, provided that the assignment of channels follows:
> 1. _One_ `node` controls LEDs of only _one_ Cyclops (aka Teensy). `1 node -> 1 teensy`
> 2. _One_ Cyclops may be controlled by _many_ different `nodes`. `1 teensy (controlled by) -> many nodes`
> 3. A plugin may control k channels, `ch1, ch2, ..., chk` but **NO** other plugin (for the same Cyclops) must control any of `ch1, ch2, ..., chk`. `1 channel (controlled by) -> 1 plugin` and `1 plugin -> many channels`
> 4. And obviously, `1 node -> many plugins`

`Plugins/Headers/SerialLib` has objects to open and operate serial ports.

# Rule map worse than c++
It is very difficult to make a UI for defining "rules" that trigger actions on the cyclops board. The most common and important use of cyclops is to maintain "spike" activity at a certain level.
Writing a feedback loop for that effect is way easier and simpler than fiddling with a graphical rule definition -- graphical tools will never have the same expressive power as a program.

>A very common problem with a graphical rule table would be when the user defines a trigger for a certain type of spike, what should be done if a similar spike occurs very soon? Definitely not retrigger! To handle that, the rule table would no longer be just a _rule table_...

Not every scientist/biologist can code in `C++`, so the plugin-language needs to be improved, by

1. **[PREFERRED]** Providing a `genericCyclopsPlugin` base class. Each user "plugin" would define an derivation.
2. write the event logic in a scripting language such as python. _[Can be done if (1) works]_,
3. making a scripting language ([flex](http://flex.sourceforge.net/), [bison](https://www.gnu.org/software/bison/)). _[Fun, but overkill]_

# `cyclops-plugin` Class
I do not exactly know how the OE GUI can detect pre-compiled derivations of `GenericProcessor` and invoke their code, perhaps the plugins are compiled into shared objects (libraries) whose `text` is imported at runtime?

In a similar manner, let's introduce `genericCyclopsPlugin` (or `gCP` in short) for OE. These plugins are different from OE plugins because **they are not processors**.
These plugins basically define 2 things, the datastructures required for functioning and the actual event logic in `gCP::handleEvent(...)`
The `gCP` might look like this,
>note that this is just an example and could by syntactically wrong :)

```c++
#include "CyclopsAPI"
class genericCyclopsPlugin{
public:
  int channel_count; // no. of channels that plugin will control
  ScopedArray<int> *LEDchannels; // That's supposed to be a pointer to an array, even if it's not.

  int signal_count;  // no. of signals that will be needed.
  /* Names of signals, these will appear on GUI.
  It's a simple mapping, first string is the "name" of the first signal, and so on..
  */
  ScopedArray<string> *signalName;
  ScopedArray<CyclopsAPI::SignalType> *signalType; // list of enums, used by GUI.

  /* ctor, dtor to define channel_count, etc */
  // called before acquisition starts, or an upload to teensy, or queried about "readiness" by GUI
  virtual void setup() = 0;
 
  // core logic. This function is called in `CyclopsStimulator.handleEvent()`
  virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition = 0);
 
  // other helpers
};
```
And a plugin might look like,

```cpp
class myCL_plugin : genericCyclopsPlugin{
public:
  /* Data structures required for operation */
private:
  /* Data structures required for operation */
public:

  myCL_plugin(int led_channels[]){
    channel_count = 2;
    signal_count = 5;
    LEDchannels = new ScopedArray(led_channels);
    signalName = new ScopedArray(signal_count);
    signalName[0] = "name0";
    signalType[0] = SQUARE;
    signalName[1] = "name1";
    signalType[1] = STORED;
    signalName[2] = "name2";
    signalType[2] = SQUARE;
  }

  virtual void setup(){
    /* intialise something? Get channel count or whatever.. */
  }
  
  virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition){
    /* core logic using data structures */
    if (eventType == X) { /* utilize the cyclops API */ }
    if (eventType == Y) {
        CyclopsAPI::do_operation_blah(<signal_id>, <channel>, <args>);
    }
  }
};
```

A `plugin` can control any no. of channels.
>A plugin that controls 2 channels can be configured to control channels 2,3 in one experiment but channels 1,4 in another. The constructor takes these as argument.

The array of `SignalType` objects just hold enums for the kind of `Source` object. The GUI ensures that a `Source` of the correct type is assigned. This will get clear in the next section

## How does the `stimulator` node setup `plugin` object?

When the plugin is selected from the dropdown / file "open", the user has to assign the channel(s) which will be controlled.
This is enough to _instantiate_ the `plugin` object.

Now, the user must assign `Signals` on the Cyclops board. So that the cyclops board (teensy) knows which signal and channel to manipulate when the plugin sends the RPC command.
>This "assignment" is already present inside the `plugin` code.

>The user can edit the assigned signals from the GUI (when acquisition/experiment is stopped).

Now, the plugin is ready to be used.
The User Interaction (after writing the plugin) is detailed in the other file/post.

## What happens during acquisition?
The `cyclops-stimulator` (processor) registers the plugins that have been added, by appending a reference to the object into a `ScopedArray<gCP *>`.  
The `handleEvent()` of the `cyclops-stimulator` is simply:

```cpp
virtual void handleEvent(int eventType, MidiMessage& event, int samplePosition){
  for (int plugin_index=0; plugin_index < num_of_plugins; plugin_index++){
    array_of_gcp_ptrs[i].handleEvent(eventType, event, samplePosition);
  }
}
```