#!/usr/bin/python

################################################################
#              Best usage is in an IPython shell:              #
#                                                              #
# $ ipython --matplotlib --no-confirm-exit -i hello.py -- -v   #
#                                                              #
# TemporarySignal is your best friend. You can play with it    #
# in friendly ways (numpy arrays, helper methods, template     #
# objects, etc), without worrying about corrupting the data-   #
# base.                                                        #
#                                                              #
# It's trivial to convert from TemporarySignal to a Signal     #
# and save all changes.                                        #
# Infact, the save method let's you choose which changes must  #
# be saved!                                                    #
#                                                              #
# Use ipython object introspection: The '?' to view docstrings #
#                                                              #
################################################################

import numpy as np
import matplotlib.pyplot as plt
from copy import deepcopy

import menu
import signalIO

def termShow(message):
    print("MPL_GUI >", message)

class EditorGUI:
    """EditorGUI manages a `matplotlib` figure. The figure includes a menu, which helps in quickly switching between signals.

    When opened inside an IPython shell, the internals of this object are not exposed.
    Each EditorGUI has a _manager_ which can access the internals of the figure, and
    provides a safe and clean interface to this object as well as other objects
    """

    @property
    def viewList(self):
        """This is the order in which signals will be or are seen on the GUI."""
        return self._viewList
    
    def __init__(self, viewList, manager=None, main_menu=True):
        """Creates a matplotlib figure and shows it."""
        self.manager = manager
        self._viewList = deepcopy(viewList)
        self.main_menu = main_menu

        self.fig = plt.figure(facecolor='#efe7da')
        self.fig.subplots_adjust(left=0.2, right=0.98)
        self.ax = self.fig.add_subplot(111)
        self.ax.set_ylim(-2, 4096)
        self.ax.grid(True)
        if main_menu:
            # menu::main
            self.menuItems = [menu.MenuItem.makeMainMenuItem(self.fig, r'$New$', self.new_signal),
                              menu.MenuItem.makeMainMenuItem(self.fig, r'$Save$', self.save)]
            self.mainMenu = menu.Menu(self.fig, self.menuItems, (5, 10))
        # menu::signals
        self.signalMenu = self.makeSignalMenu()
        # state variables
        self.selected = None  # Holds the name of the signal which is currently shown on canvas.

        self.line, = plt.step([1], [1], where='pre')
        self.fig.canvas.mpl_connect('close_event', self.manager.onClose)

    def new_signal(self, menu_item):
        """Not Used"""
        termShow("New Sig")

    def save(self, menu_item):
        """Not Used"""
        termShow("Saving all signals to file")

    def save_temp(self, name):
        """Adds the temporary signal to the Permanent list of the manager"""
        if name in self.manager._temporal:
            ts = self.manager._temporal[name]
            self.manager._SignalMap[name] = signalIO.Signal(ts.name, ts.sourceType, ts.size, ts.holdTime.tolist(), ts.voltage.tolist())
            return True
        return False

    def refreshMenuItem(self, new_name, old_name):
        for item in self.signalMenu.menuItems:
            if item.labelstr == new_name:
                break
        item.hovering = False
        item.clicked = True
        item.props.set_toDown()
        item.refresh()
        for item in self.signalMenu.menuItems:
            if item.labelstr == old_name:
                break
        item.hovering = False
        item.clicked = False
        item.props.set_toIdle()
        item.refresh()

    def onSelect(self, tempSignal, menuItem=None):
        """GUI callback for mouse click on MenuItem"""
        termShow("(re)plot: " + tempSignal.name)
        if menuItem is None:
            self.refreshMenuItem(tempSignal.name, self.selected)
        self.line.set_xdata(tempSignal._tData)
        self.line.set_ydata(tempSignal._vData)
        self.ax.set_xlim(-10, tempSignal._tData[-1]+10)

        self.selected = tempSignal.name
        self.fig.canvas.draw_idle()  # Not really required

    def makeSignalMenu(self):
        """Used by constructor to make the Menu from a list of signal names"""
        sig_item_list = []
        for name in self.viewList:
            item = menu.MenuItem.makeSignalMenuItem(self.fig, self.manager._temporal[name], self.onSelect)
            sig_item_list.append(item)
        # The (5, 40) is location of the menu on the screen (from top left)
        # If main menu is also show, the position is (5, 80)
        return menu.Menu(self.fig, sig_item_list, (5, 80 if self.main_menu else 40))

    def addToMenu(self, names):
        """Adds many "new" MenuItems onto the signalMenu

        It is assumed that the elements in `names` will be valid keys of `manager.temporalMap`
        """
        if not isinstance(names, list):
            print("Expected a [`tuple` or `list`] of names (`str`)!")
            return False
        newItems = []
        newNames = []
        for name in names:
            if name in self.manager._temporal:
                newNames.append(name)
                newItems.append(menu.MenuItem.makeSignalMenuItem(self.fig, self.manager._temporal[name], self.onSelect))
        self.viewList.extend(newNames)
        return self.signalMenu.addToMenu(newItems)

def makeTemporal(name_list, sigMap, se_interactive):
    """Helper function to create `signalIO.TemporarySignal`

    Reads a name_list<str> of "names" and converts only those `Signal`s from
    sigMap<str, signalIO.Signal> into `signalIO.TemporarySignal` objects.
    These `TemporarySignal`s are returned as a `dict`.
    """
    res = {}
    for name in name_list:
        res[name] = signalIO.TemporarySignal(se_interactive, sigMap[name])
    return res

class Interactive:
    """Interactive is a "manager" class, and provides a clean, friendly interface to the `signalEditor`.

    This manager should be used if you want to do interactive introspection and modification.
    To launch the GUI use:
        $ipy> In [k]: `se.launchGUI()`
                   (OR)
        $ipy> In [k]: `/se.launchGUI`

    Read Only Members:
     signalMap               -- The signal DB as read/written to file
     namesInOriginalOrder
     temporalMap             -- An editable version of the Signal DB.
     guiViewList             -- The list of signal names seen on the GUI as a menu.

    Easy Accessor Methods:
    Use these methods to "get" info/object
     getCurrentSignal   == gcs  -- Get the TemporarySignal which is currently
                                   plotted on the GUI canvas.
     getSignal          == gs   -- Get the TemporarySignal by its name
     list               == ls   -- List all the names of signals

    Easy Setter Methods:
    Use these methods to "set" object attributes or to perform GUI manipulation.
     setCurrentSignal   == scs  -- Set the "selected" signal as <signal-name>,
                                   and update GUI.
     new                        -- Create a new signal, apply a template,
                                   and update GUI.

    Generation Methods:
    To create template signals. These methods return (holdTime, voltage) pair of `numpy.ndarray`s
     square      -- Create square wave with given/default parameters
     triangle    -- Create triangle wave with given/default parameters
     sine        -- Create sine wave with given/default parameters

    ## Also see "Methods of `TemporarySignal` objects" ##
    """
    def __init__(self, args):
        """Create a manager, and fetch the signals database"""
        self._eg = None
        self.verbose = args.verbose
        self.pathTable = signalIO.makePathTable(args)
        self._OrigOrderList, self._SignalMap = signalIO.getSignalDatabase(self.pathTable['sig_file'])

        self._temporal = makeTemporal(self._OrigOrderList, self._SignalMap, self)

    @property
    def signalMap(self):
        """This is a collection (`dict`) of `signalIO.Signals` which were read from the Database. You cannot access this map directly."""
        return {name : {'sourceType' : self._SignalMap[name].sourceType,
                        'size'       : self._SignalMap[name].size}
                for name in self._SignalMap}
    
    @property
    def temporalMap(self):
        """This is a collection (`dict`) of `signalIO.TemporarySignals`. You cannot access the map directly, but can access elements via `se.gcs()`."""
        return {name : {'sourceType' : self._temporal[name].sourceType,
                        'size'       : self._temporal[name].size}
                for name in self._temporal}

    @property
    def namesInOriginalOrder(self):
        """This is the order in which signals were found when file was first read.

        The order is maintained, and new signals are appended when saving to
        file.
        """
        return deepcopy(self._OrigOrderList)

    @property
    def guiViewList(self):
        """This is the order in which signals will be or are seen on the GUI."""
        if self._eg:
            return deepcopy(self._eg.viewList)
        else:
            return None
    
    def getCurrentSignal(self):
        """Returns the TemporarySignal which is currently plotted on the GUI canvas"""
        if self._eg is not None and self._eg.selected is not None:
            return self._temporal.get(self._eg.selected, None)
        else:
            return None
    def gcs(self):
        """Returns the TemporarySignal which is currently plotted on the GUI canvas"""
        return self.getCurrentSignal()

    def setCurrentSignal(self, sig_name):
        """Changes current/active signal to the one indexed by `sig_name`

        Updates the plot on the GUI canvas
        """
        if self._eg is not None and sig_name in self._temporal:
            self.onSelect(self._temporal[sig_name])
            return self._temporal[sig_name]
        return None
    def scs(self, sig_name):
        """Changes current/active signal to the one indexed by `sig_name`

        Updates the plot on the GUI canvas
        """        
        return self.setCurrentSignal(sig_name)

    def getSignal(self, sig_name):
        """Returns the TemporarySignal indexed by `sig_name`
        
        Does NOT update the GUI canvas.
        """
        if sig_name not in self._temporal:
            print("No such signal with name:", sig_name)
            print("List all signals with `se.list()` or `se.ls()`")
            return None
        return self._temporal[sig_name]
    def gs(self, sig_name):
        """Returns the TemporarySignal indexed by `sig_name`
        
        Does NOT update the GUI canvas.
        """
        return self.getSignal(sig_name)

    def list(self):
        """List all the names of signals which are in the database."""
        if self._eg is not None:
            return deepcopy(self._eg.viewList)
        else:
            return list(self._temporal.keys())
    def ls(self):
        """List all the names of signals which are in the database."""
        return self.list()

    def new(self, name, src_type):
        """Create a new signal, automatically populates a template signal based on the `src_type.

        Arguments:
        name     -- Name of the new signal (cannot be changed later!)
        src_type -- Type of the new signal, this determines which default
                    template will be used. (cannot be changed later!)
        """
        assert(src_type != 1 and name not in self._temporal)
        # This is automatically a Square signal
        s = signalIO.Signal(name=name, sourceType=src_type)
        if src_type == 0:
            ht, v = self.sine(units=50, startAngle=-np.pi*4/9, endAngle=np.pi*4/9, offset=2047, scale=2000)
            s.holdTime = ht.tolist()
            s.voltage = v.tolist()
            s.size = len(ht)
        self._temporal[name] = signalIO.TemporarySignal(self, s)
        self._temporal[name]._dirty = True
        if self._eg is None:
            self._OrigOrderList.append(name)
            self._SignalMap[name] = s
        else:
            self._eg.addToMenu([name])
            self.onSelect(self._temporal[name])
        return self._temporal[name]

    def launchGUI(self):
        """Creates the EditorGUI instance, which automatically opens it too (in IPython session)"""
        selected = None
        if self._eg is None:
            self._eg = EditorGUI(self._OrigOrderList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)
        else:
            # the GUI needs to be re-made.
            # self._eg.fig.show() re-opens the window but the events don't propagate correctly
            selected = self._eg.selected
            self._eg = EditorGUI(self._eg.viewList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)
            self.onSelect(self._temporal[selected])
        return self

    def save(self):
        """Cherry-pick changes and save to file.

        Allows you to selectively save changes made to signals.
        """
        # find signals which have been modified, ask user to choose
        chosen = []
        for name in self._temporal:
            if self._temporal[name].dirty:
                if signalIO.getChoice('', 'Save changes to `'+name+'`? >>', default=True):
                    chosen.append(name)
        if len(chosen) > 0:
            for name in chosen:
                if self._eg.save_temp(name):
                    self._temporal[name]._dirty = False
                    if name not in self.namesInOriginalOrder:
                        self._OrigOrderList.append(name)
                    if self.verbose:
                        print('Moved `' + name +'` to permanent list.')
                else:
                    print('Could not save `' + name +'`')
            if self.verbose:
                print('Saving to file...')
            # save in same order as original
            signalIO.saveSignalDatabase(self.pathTable['sig_file'], self.namesInOriginalOrder, self._SignalMap)
            return True
        return False

    def onSelect(self, tempSignal):
        """Internal Function. Do not use."""
        self._eg.onSelect(tempSignal)

    def onClose(self):
        """Callback when EditorGUI is closed
    
        Upstream-Bug: This does not work with IPython shell.
        """
        print('Closing')
        return True

    def square(self, offLevel=0, onLevel=4095, period=280, pulsewidth=0.5):
        """Create `np.ndarray` for a square wave of `period` usec and `pulsewidth`

        The switching point is exact (to nearest (smaller) `int`)
        Returns tuple of 2 `numpy.ndarray`s : (holdTime, voltage)
        """
        assert(pulsewidth < 1.0 and pulsewidth > 0.0)
        onTime = period*pulsewidth
        offTime = period-onTime
        m = min(onTime, offTime)
        if (onTime < signalIO.Signal.MIN_HOLD_TIME) or (offTime < signalIO.Signal.MIN_HOLD_TIME):
            print("This makes smallest holdTime =", m, "usec. But, it must remain >", signalIO.Signal.MIN_HOLD_TIME)
            return (None, None)
        print("Smallest holdTime is", m, "usec.")
        return (np.array([offTime, onTime]), np.array([offLevel, onLevel]))

    def triangle(self,
                 leftLevel=0, middleLevel=4095, rightLevel=0,
                 leftUnits=20, rightUnits=20,
                 resolution=signalIO.Signal.MIN_HOLD_TIME):
        """Create a triangle wave at given `resolution`. `leftUnits` and `rightUnits` can be 0, but not together.

        It takes `leftUnits` of `resolution` units for signal to "change" from `leftLevel`
        to `middleLevel`, and `rightUnits` of units for `middleLevel` to `rightLevel`.

                              ._.  ______middleLevel
                            ._| |_.
                          ._| ; : |_.
                        ._|   ' :   |_.
        leftLevel___  ._|     ; :     |_.
                      |       ' :       |_.
                      |       ; :         |_.
                      |       ' :           |_.  _____rightLevel
                      <--leftU-->             :
                          (5) ; :             :
                              ' <----rightU--->
                              ;        (7)
        
        In the special case when `leftUnits` is zero,

            ._.  ______middleLevel
            | |_.
            | : |_.
            | :   |_.
            | :     |_.
            | :       |_.
            | :         |_.
            | :           |_.
            | :             |_.
            ; :               ;
            ' <~~not~rightU~~~>   X---,   ,----------------------------------,
            ;                 ;       |--(  The "middleLevel" is included in |
            <------rightU-----> <-----'   \ the`rightUnits` this time.       |
                     (9)                   `---------------------------------'
        
        Returns tuple of 2 `numpy.ndarray`s : (holdTime, voltage)
        """
        assert(resolution >= signalIO.Signal.MIN_HOLD_TIME)
        assert(leftUnits >= 0)
        assert(rightUnits >= 0)
        assert(leftUnits != 0 or rightUnits != 0)

        vl = np.linspace(leftLevel, middleLevel, leftUnits)
        vr = np.linspace(middleLevel, rightLevel, rightUnits+1)
        if leftUnits > 0:
            v = np.concatenate((vl, vr[1:]), axis=0)
        elif leftUnits == 0:
            v = np.linspace(middleLevel, rightLevel, rightUnits)
        return (np.ones(leftUnits+rightUnits)*resolution, v)

    def sine(self,
             startAngle=0, endAngle=2*np.pi,
             scale=2000, offset=1000,
             units=800,
             resolution=signalIO.Signal.MIN_HOLD_TIME):
        """Create a sine wave from `startAngle` to `endAngle` ( > `startAngle`) in `units` steps @ `resolution`

        Returns tuple of 2 `numpy.ndarray`s : (holdTime, voltage)
        """
        assert(resolution >= signalIO.Signal.MIN_HOLD_TIME)
        assert(units >= 1)
        assert(startAngle < endAngle)

        t = np.arange(0, units)/(units-1)*(endAngle-startAngle)+startAngle
        return (np.ones(units)*resolution, np.sin(t)*scale+offset)

    def getResHoldTime(self, units, resolution=signalIO.Signal.MIN_HOLD_TIME):
        """Returns tuple of 2 `numpy.ndarray`s : (holdTime, time)

        holdTime -- an array of size `units` containing `resolution` only. Ex: [90, 90, 90, ... 90]
        time     -- an array of size `units` from [0, `units`-1]
        """
        print("That's", units*resolution, "usec long.")
        return np.ones(units)*resolution, np.arange(0, units)
    def get_ht(self, units, resolution=signalIO.Signal.MIN_HOLD_TIME):
        """Returns tuple of 2 `numpy.ndarray`s : (holdTime, time)

        holdTime -- an array of size `units` containing `resolution` only, Ex: [90, 90, 90, ... 90]
        time     -- an array of size `units` from [0, `units`-1]
        """
        return self.getResHoldTime(units, resolution)
    def get_max_ht(self, units):
        """Returns tuple of 2 `numpy.ndarray`s : (holdTime, time) @ max resolution!

        holdTime -- an array of size `units` containing `max_resolution` only. Ex: [50, 50, 50, ... 50]
        time     -- an array of size `units` from [0, `units`-1]
        """
        return self.getResHoldTime(units)























if __name__ == '__main__':
    class nonInteractive:
        def __init__(self, args):
            self._eg = None
            self.verbose = args.verbose
            self.pathTable = signalIO.makePathTable(args)
            self.OrigOrderList, self.SignalMap = signalIO.getSignalDatabase(self.pathTable['sig_file'])
            self._temporal = makeTemporal(self.OrigOrderList, self.SignalMap, self)

        def launchGUI(self):
            self._eg = EditorGUI(self.OrigOrderList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)

        def onClose(self, event):
            print("Not saving changes.")

    print('~'*31 + "\n| Cyclops Signal Editor 0.0.0 |\n" + '~'*31)
    args = signalIO.parser.parse_args()
    nI = nonInteractive(args)
    nI.launchGUI()
    plt.show()