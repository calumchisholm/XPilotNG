from MapObject import MapObject


class MapCheckPoint(MapObject):
    def __init__(self, x = 0, y = 0, bmstore = None):
        MapObject.__init__(self, x, y, 35 * 64, 35 * 64,
                           bmstore["checkpoint.gif"])

    def printXml(self, file):
        b = self.getBounds()
        file.write('<Check x="%d" y="%d"/>\n' % (b.x + b.width / 2,
                                               b.y + b.height / 2))
