#!/usr/bin/python

#############################################################
#                DO NOT IMPORT THIS IN A SCRIPT             #
#                                                           #
#              Best usage is in an IPython shell:           #
#                                                           #
# $> ipython --matplotlib                                   #
# In [1] : import signalEditor.Interactive as interactive   #
# Out[1]                                                    #
# In [2] : sig_editor = interactive()                       #
# Out[2]                                                    #
# In [3] : sig_editor.launchGUI()                           #
#############################################################

import numpy as np
import matplotlib.pyplot as plt
from copy import deepcopy

import menu
import signalIO


def termShow(message):
    print("MPL_GUI >", message)

class editorGUI:
    @property
    def viewList(self):
        return self._viewList
    
    def __init__(self, viewList, manager=None, main_menu=True):
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
        self.selected = None
        self.editing = False

        self.line, = plt.step([1], [1], where='pre')
        self.fig.canvas.mpl_connect('close_event', self.manager.onClose)

    def new_signal(self, menu_item):
        termShow("New Sig")

    def save(self, menu_item):
        termShow("Saving all signals to file")

    def save_temp(self, name):
        if name in self.manager._temporal:
            ts = self.manager._temporal[name]
            self.manager._SignalMap[name] = signalIO.Signal(ts.name, ts.sourceType, ts.size, ts.holdTime.tolist(), ts.voltage.tolist())
            return True
        return False

    def onSelect(self, tempSignal):
        termShow("(re)plot: " + tempSignal.name)
        self.line.set_xdata(tempSignal._tData)
        self.line.set_ydata(tempSignal._vData)
        self.ax.set_xlim(-2, tempSignal._tData[-1]+2)

        self.selected = tempSignal.name
        self.fig.canvas.draw_idle()

    def makeSignalMenu(self):
        sig_item_list = []
        for name in self.viewList:
            item = menu.MenuItem.makeSignalMenuItem(self.fig, self.manager._temporal[name], self.onSelect)
            sig_item_list.append(item)
        return menu.Menu(self.fig, sig_item_list, (5, 80 if self.main_menu else 40))

    def addToMenu(self, names):
        if not isinstance(names, list):
            print("Expected a [`tuple` or `list`] of names (`str`)!")
            return False
        newItems = []
        for name in names:
            newItems.append(menu.MenuItem.makeSignalMenuItem(self.fig, self.manager._temporal[name], self.onSelect))
        self.viewList.extend(names)
        return self.signalMenu.addToMenu(newItems)

def makeTemporal(name_list, sigMap, se_interactive):
    res = {}
    for name in name_list:
        res[name] = signalIO.TemporarySignal(se_interactive, sigMap[name])
    return res

class Interactive:
    def __init__(self, args):
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
        if self._eg is not None and self._eg.selected is not None:
            return self._temporal.get(self._eg.selected, None)
        else:
            return None
    def gcs(self):
        return self.getCurrentSignal()

    def setCurrentSignal(self, sig_name):
        if self._eg is not None and sig_name in self._temporal:
            self._eg.selected = sig_name
            self.onSelect(self._temporal[sig_name])
            return self._temporal[sig_name]
        return None
    def scs(self, sig_name):
        return self.setCurrentSignal(sig_name)

    def getSignal(self, sig_name):
        if sig_name not in self._temporal:
            print("No such signal with name:", sig_name)
            print("List all signals with `se.list()` or `se.ls()`")
            return None
        return self._temporal[sig_name]
    def gs(self, sig_name):
        return self.getSignal(sig_name)

    def list(self):
        if self._eg is not None:
            return deepcopy(self._eg.viewList)
        else:
            return list(self._temporal.keys())
    def ls(self):
        return self.list()

    def new(self, name, src_type):
        assert(src_type != 1 and name not in self._temporal)
        s = signalIO.Signal(name=name, sourceType=src_type)
        self._temporal[name] = signalIO.TemporarySignal(self, s)
        if self._eg is None:
            self._OrigOrderList.append(name)
            self._SignalMap[name] = s
        else:
            self._eg.addToMenu([name])
            self.onSelect(self._temporal[name])
        return self._temporal[name]

    def launchGUI(self):
        selected = None
        if self._eg is None:
            self._eg = editorGUI(self._OrigOrderList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)
        else:
            selected = self._eg.selected
            self._eg = editorGUI(self._eg.viewList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)
            self.onSelect(self._temporal[selected])
        return self

    def save(self):
        chosen = []
        for name in self._temporal:
            if self._temporal[name].dirty:
                if signalIO.getChoice('', 'Save changes to `'+name+'`? >>', default=True):
                    chosen.append(name)
        if len(chosen) > 0:
            for name in chosen:
                if self._eg.save_temp(name):
                    if self.verbose:
                        print('Moved `' + name +'` to permanent list.')
                else:
                    print('Could not save `' + name +'`')
            if self.verbose:
                print('Saving to file...')
            signalIO.saveSignalDatabase(self.pathTable['sig_file'], self._eg.viewList, _SignalMap)
            return True
        return False

    def onSelect(self, tempSignal):
        self._eg.onSelect(tempSignal)

    def onClose(self):
        print('Closing')
        return True



























if __name__ == '__main__':
    class nonInteractive:
        def __init__(self, args):
            self._eg = None
            self.verbose = args.verbose
            self.pathTable = signalIO.makePathTable(args)
            self.OrigOrderList, self.SignalMap = signalIO.getSignalDatabase(self.pathTable['sig_file'])
            self._temporal = makeTemporal(self.OrigOrderList, self.SignalMap, self)

        def launchGUI(self):
            self._eg = editorGUI(self.OrigOrderList, manager=self, main_menu=False)
            self._eg.fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(self._eg.fig.dpi), 7, forward=True)

        def onClose(self, event):
            print("Not saving changes.")

    print('~'*31 + "\n| Cyclops Signal Editor 0.0.0 |\n" + '~'*31)
    args = signalIO.parser.parse_args()
    nI = nonInteractive(args)
    nI.launchGUI()
    plt.show()