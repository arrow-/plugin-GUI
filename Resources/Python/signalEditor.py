import numpy as np
import matplotlib.pyplot as plt
import menu
import signalIO


def new_signal(menu_item):
    print("New Sig")


def save(menu_item):
    print("Saving all sigs to file")


def onSelect(signal_item):
    global line, fig, plt
    print("click", signal_item.signal.name)
    if signal_item.signal.np_holdTime is None:
        signal_item.signal.np_holdTime = np.insert(np.cumsum(np.array(signal_item.signal.holdTime)), 0, 0)
        signal_item.signal.np_voltage  = np.insert(np.array(signal_item.signal.voltage), 0, signal_item.signal.voltage[-1])
        #signal_item.signal.holdTime_min = np.amin(signal_item.signal.np_holdTime)
        #signal_item.signal.holdTime_max = np.amax(signal_item.signal.np_holdTime)

    #line.set_data(signal_item.signal.np_holdTime, signal_item.signal.np_voltage)
    plt.cla()
    ax.set_ylim(-2, 4096)
    ax.grid(True)
    plt.step(signal_item.signal.np_holdTime, signal_item.signal.np_voltage, where='pre')
    plt.xlim(-2, signal_item.signal.np_holdTime[-1]+2)
    #fig.canvas.draw()

def makeSignalMenu(fig):
    sig_item_list = []
    if len(SignalList) > 0:
        for signal in SignalList:
            item = menu.MenuItem.makeSignalMenuItem(fig, signal, onSelect)
            sig_item_list.append(item)
    else:
        SignalList.append(signalIO.Signal())
        sig_item_list.append(menu.MenuItem.makeSignalMenuItem(fig, SignalList[0], onSelect))
    return sig_item_list, menu.Menu(fig, sig_item_list, (5, 80))

try:
    __IPYTHON__
    # imported in IPython
    args = signalIO.parser.parse_args([])
except:
    # invoked from terminal
    print('~'*31 + "\n| Cyclops Signal Editor 0.0.0 |\n" + '~'*31)
    args = signalIO.parser.parse_args()

# globals
# globals::signalIO
pathTable = signalIO.makePathTable(args)
SignalList = signalIO.getSignalDatabase(pathTable['sig_file'])

if __name__ == '__main__':
    # global::matplotlib
    fig = plt.figure(facecolor='#efe7da')
    fig.set_size_inches(((menu.Menu.signalButtonWidth+80)*5)/float(fig.dpi), 7, forward=True)
    fig.subplots_adjust(left=0.2, right=0.98)
    ax = fig.add_subplot(111)
    ax.set_ylim(-2, 4096)
    ax.grid(True)

    # global::menu::main
    menuItems = [menu.MenuItem.makeMainMenuItem(fig, r'$New$', new_signal),
                 menu.MenuItem.makeMainMenuItem(fig, r'$Save$', save)]
    mainMenu = menu.Menu(fig, menuItems, (5, 10))

    # global::menu::signals
    signalMenuItems, signalMenu = makeSignalMenu(fig)

    plt.show()