from javastuff import *

class PolygonStyle:
    FILL_NONE = 0
    FILL_COLOR = 1
    FILL_TEXTURED = 2

    def __init__(self):
        self.visible = 1
        self.visibleInRadar = 1
        self.color = None
        self.texture = None
        self.fillStyle = PolygonStyle.FILL_NONE
        self.defEdgeStyle = None
        id = ""

    def isVisible(self):
        return self.visible

    def setVisible(self, v):
        self.visible = v

    def isVisibleInRadar(self):
        return self.visibleInRadar

    def setVisibleInRadar(self, v):
        self.visibleInRadar = v

    def getColor(self):
        return self.color

    def setColor(self, c):
        self.color = c

    def getTexture(self):
        return self.texture

    def setTexture(self, t):
        self.texture = t

    def getFillStyle(self):
        return self.fillStyle

    def setFillStyle(self, f):
        self.fillStyle = f

    def getDefaultEdgeStyle(self):
        return self.defEdgeStyle

    def setDefaultEdgeStyle(self, l):
        self.defEdgeStyle = l

    def getId(self):
        return self.id

    def setId(self, id):
        self.id = id

    def computeFlags(self):
        flags = 0
        if self.fillStyle != self.FILL_NONE:
            if self.color != None:
                flags |= 1
            if self.texture != None:
                flags |= 2
        if not self.isVisible():
            flags |= 4
        if not self.isVisibleInRadar():
            flags |= 8
        return flags

    def printXml(self, file):
        file.write('<Polystyle id="%s"' % self.id)
        if self.color is not None:
            file.write(' color="%s"' % toRGB(self.color))
        elif self.fillStyle == self.FILL_TEXTURED and self.texture != None:
            file.write(' texture="%s"' % self.texture.getFileName)
        file.write(' defedge="%s" flags="%s"/>\n' % (self.defEdgeStyle.getId(),
                                                   self.computeFlags()))
