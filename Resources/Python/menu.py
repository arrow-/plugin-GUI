import numpy as np
import matplotlib
import matplotlib.colors as colors
import matplotlib.patches as patches
import matplotlib.mathtext as mathtext
import matplotlib.pyplot as plt
import matplotlib.artist as artist
import matplotlib.image as image

class ItemProperties(object):
    def __init__(self, fontSize=15,
                 labelColors  = ['black', 'black'],  # normal, hover, down
                 bgColors     = ['yellow', 'blue']): # normal, hover, down
        self.fontSize = fontSize
        assert(len(labelColors) > 0)
        assert(len(bgColors)    > 0)
        
        labelColors = [colors.colorConverter.to_rgb(x) for x in labelColors]
        bgColors = [colors.colorConverter.to_rgb(x) for x in bgColors]

        self.labelColor_idle = labelColors[0]
        self.bgColor_idle    = bgColors[0]
        self.bgAlpha_idle    = colors.colorConverter.to_rgba(self.bgColor_idle)[3]

        if len(labelColors) > 1:
            self.labelColor_hover = labelColors[1]
            if len(labelColors) > 2:
                self.labelColor_down = labelColors[2]
            else:
                self.labelColor_down = labelColors[0]
        else:
            self.labelColor_hover = self.labelColor_idle
            self.labelColor_down = self.labelColor_idle
        
        if len(bgColors) > 1:
            self.bgColor_hover = bgColors[1]
            if len(bgColors) > 2:
                self.bgColor_down = bgColors[2]
            else:
                self.bgColor_down = bgColors[0]
        else:
            self.bgColor_hover = self.bgColor_idle
            self.bgColor_down = self.bgColor_idle
        self.bgAlpha_hover = colors.colorConverter.to_rgba(self.bgColor_hover)[3]
        self.bgAlpha_down = colors.colorConverter.to_rgba(self.bgColor_down)[3]

        self.labelColor = self.labelColor_idle
        self.bgColor    = self.bgColor_idle
        self.bgAlpha    = self.bgAlpha_idle

    def set_toHover(self):
        self.bgColor    = self.bgColor_hover
        self.labelColor = self.labelColor_hover
        self.bgAplha    = self.bgAlpha_hover

    def set_toIdle(self):
        self.labelColor = self.labelColor_idle
        self.bgColor    = self.bgColor_idle
        self.bgAlpha    = self.bgAlpha_idle

    def set_toDown(self):
        self.labelColor = self.labelColor_down
        self.bgColor    = self.bgColor_down
        self.bgAlpha    = self.bgAlpha_down


menuItemProps = {
    'main'   : ItemProperties(bgColors=['#a5668b', '#d3bcc0'], labelColors=['white', 'black']),
    'signal' : ItemProperties(bgColors=['#05668d', '#427aa1'], labelColors=['black', 'black']),
    'stored' : ItemProperties(bgColors=['#e91e63', '#f06292'], labelColors=['black', 'black']),
    'square' : ItemProperties(bgColors=['#1de9b6', '#a7ffeb'], labelColors=['black', 'black'])
}

class MenuItem(artist.Artist):
    parser = mathtext.MathTextParser("Bitmap")
    padx = 5
    pady = 5

    def __init__(self, fig, labelstr,
                 props=None,
                 on_select=None,
                 tempSignal=None):
        artist.Artist.__init__(self)

        self.set_figure(fig)
        self.labelstr = labelstr
        self.tempSignal = tempSignal
        self.hovering = False
        self.clicked = False

        if props is None:
            if tempSignal == None:
                props = ItemProperties()
            else:
                if tempSignal.sourceType == 0:
                    props = menuItemProps['stored']
                elif tempSignal.sourceType == 2:
                    props = menuItemProps['square']
                else:
                    props = menuItemProps['signal']
        self.props = props
        self.props.set_toIdle()

        self.on_select = on_select

        x, self.depth = self.parser.to_mask(
            labelstr, fontsize=props.fontSize, dpi=fig.dpi)

        self.labelwidth = x.shape[1]
        self.labelheight = x.shape[0]

        self.labelArray = np.zeros((x.shape[0], x.shape[1], 4))
        self.labelArray[:, :, -1] = x/255.0

        self.label = image.FigureImage(fig, origin='upper')
        self.label.set_array(self.labelArray)

        # we'll update these later
        self.rect = patches.Rectangle((0, 0), 1, 1)
        self.recolorLabel()
        self.rect.set(facecolor=self.props.bgColor, alpha=self.props.bgAlpha)
        fig.canvas.mpl_connect('button_release_event', self.check_select)

    def check_select(self, event):
        over, junk = self.rect.contains(event)
        if not over:
            self.props.set_toIdle()
            self.clicked = False
        elif self.on_select is not None:
            self.props.set_toDown()
            self.clicked = True
            self.on_select(self.tempSignal)
            self.figure.canvas.draw_idle()
        self.recolorLabel()
        self.rect.set(facecolor=self.props.bgColor, alpha=self.props.bgAlpha)

    def set_extent(self, x, y, w, h):
        self.rect.set_x(x)
        self.rect.set_y(y)
        self.rect.set_width(w)
        self.rect.set_height(h)

        self.label.ox = x + self.padx
        self.label.oy = y - self.depth + self.pady

        self.rect._update_patch_transform()

    def draw(self, renderer):
        self.rect.draw(renderer)
        self.label.draw(renderer)

    def set_hover(self, event):
        hovering, junk = self.rect.contains(event)
        changed = (self.hovering != hovering)
        self.hovering = hovering
        if not self.clicked:
            if self.hovering:
                self.props.set_toHover()
            else:
                self.props.set_toIdle()
            self.recolorLabel()
            self.rect.set(facecolor=self.props.bgColor, alpha=self.props.bgAlpha)
        return changed

    def recolorLabel(self):
        r, g, b = colors.colorConverter.to_rgb(self.props.labelColor)
        self.labelArray[:, :, 0] = r
        self.labelArray[:, :, 1] = g
        self.labelArray[:, :, 2] = b
        self.label.set_array(self.labelArray)

    @staticmethod
    def makeMainMenuItem(fig, labelstr, on_select):
        return MenuItem(fig, labelstr,
                        props = menuItemProps['main'],
                        on_select = on_select)

    @staticmethod
    def makeSignalMenuItem(fig, tempSignal, on_select):
        return MenuItem(fig, tempSignal.name,
                        on_select = on_select,
                        tempSignal=tempSignal)


class Menu(object):
    pady = 10
    signalButtonWidth = 150
    def __init__(self, fig, menuitems, loc):
        self.figure = fig
        fig.suppressComposite = True

        self.menuItems = menuitems
        # location from top left.
        # MPL figure "origin" is bottom left.
        # But loc is distance from TOP left!!
        self.x = max(Menu.pady, loc[0])
        self.y = loc[1]
        self.itemOffset = 0

        # this call must happen in this constructor
        self.resize(self.menuItems)

        fig.canvas.mpl_connect('motion_notify_event', self.on_move)
        fig.canvas.mpl_connect('resize_event', self.onResize)
        fig.canvas.draw()

    def addToMenu(self, newItems):
        assert(isinstance(newItems, list))
        for newItem in newItems:
            self.menuItems.append(newItem)
        self.resize(newItems)


    def resize(self, newItems):
        # Add new MenuItem(s) to the figure
        self.figure.artists.extend(newItems)

        numItems = len(self.menuItems)
        maxw = min(max([item.labelwidth for item in self.menuItems]), Menu.signalButtonWidth)
        maxh = max([item.labelheight for item in self.menuItems])
        totalh = numItems*maxh + (numItems + 1)*2*MenuItem.pady

        self.itemWidth = maxw + 2*MenuItem.padx
        self.itemHeight = maxh + MenuItem.pady

        self.width = self.itemWidth
        self.height = self.placeItems(self.figure.get_figheight()*self.figure.get_dpi() - self.y)

    def placeItems(self, yOffset):
        for item in self.menuItems:
            bottom = yOffset - self.itemHeight - MenuItem.pady
            item.set_extent(self.x, bottom, self.itemWidth, self.itemHeight)
            yOffset -= self.itemHeight + MenuItem.pady
        return yOffset

    def on_move(self, event):
        draw = False
        for item in self.menuItems:
            draw = item.set_hover(event)
            if draw:
                self.figure.canvas.draw()
                break


    def onResize(self, event):
        yOffset = event.height-self.y
        self.placeItems(yOffset)

if __name__ == '__main__':
    fig = plt.figure()
    fig.subplots_adjust(left=0.2, right=0.95)

    def on_select(item):
        print('btn_pressed ' + item.labelstr)
    menuitems =[MenuItem(fig, r'$New$', props=menuItemProps['main'],
                         on_select=on_select),
                MenuItem(fig, r'$Save$', props=menuItemProps['main'],
                         on_select=on_select)]
    menu = Menu(fig, menuitems)
    plt.show()