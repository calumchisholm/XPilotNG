from MapObject import MapObject

class MapBall(MapObject):
    def __init__(self, x = 0, y = 0, team = 1, bmstore = None):
        MapObject.__init__(self, x, y, 21 * 64, 21 * 64, bmstore["ball.gif"])
        self.team = team

    def getTeam(self):
        return self.team

    def setTeam(self, team):
        if team < 1 or team > 10:
            raise ValueError("illegal team: %d", team)
        self.team = team

    def printXml(self, file):
        file.write('<Ball x="%d" y="%d" team="%d"/>\n' % (
            self.getBounds().x + self.getBounds().width / 2,
            self.getBounds().y + self.getBounds().height / 2,
            self.getTeam()))
