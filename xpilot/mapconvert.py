removedopts = ['extraborder', 'edgebounce', "maxshieldedplayerwallbounceangle",
"maxshieldedbounceangle", "maxunshieldedplayerwallbounceangle",
"maxunshieldedbounceangle", "scoretablefilename", "scoretable", 'teamassign',
'oneplayeronly', "maxrobots", "robots", "minrobots", "robotfile", "robotstalk",
"robotsleave", "robotleavelife", "robotleavescore", "robotleaveratio",
"robotteam", "restrictrobots", "reserverobotteam", "ecmsreprogramrobots"]

def removeopts(options):
    for opt in options.keys():
	if opt in removedopts:
	    del options[opt]

def parse(lines):
    options = {}
    ln = 0
    while ln < len(lines):
	line = lines[ln].split("#")[0].strip() # comments & s/e whitespace
	ln += 1
	if line == "":
	    continue
	option, value = line.split(':', 1)
	option = option.strip().lower()
	value = value.strip()
	if value[:11] == "\multiline:":
	    delim = value[11:].strip() + "\n"
	    value = ""
	    while ln < len(lines) and lines[ln] != delim:
		value += lines[ln]
		ln += 1
	    ln += 1
	options[option] = value
    return options

repl = (('&', '&amp;'), ('"', '&quot;'), ("'", '&apos;'),
	    ('<', '&lt;'), ('>', '&gt;'))

def encode(text):
    for t, r in repl:
	text = text.replace(t, r)
    return text

def center(coord, size):
    return ((coord + size / 2) % size - size / 2)

dirs = ((1, 0), (1, -1), (0, -1), (-1, -1), (-1, 0), (-1, 1), (0, 1), (1, 1))
SPACE = ' .'  # More need to be added here for the algorithm to work correctly
FILLED = 'x#' # Consider fuel stations as a filled block
REC_LU = 's'
REC_RU = 'a'
REC_LD = 'w'
REC_RD = 'q'
BCLICKS = 35 * 64

class Wrapcoords:
    def __init__(self, width, height, x = 0, y = 0):
	self.width = width
	self.height = height
	self.x = x % width
	self.y = y % height
    def __cmp__(self, other):
	return self.x != other.x or self.y != other.y
    def u(self):
	return Wrapcoords(self.width, self.height, self.x, self.y - 1)
    def d(self):
	return Wrapcoords(self.width, self.height, self.x, self.y + 1)
    def l(self):
	return Wrapcoords(self.width, self.height, self.x - 1, self.y)
    def r(self):
	return Wrapcoords(self.width, self.height, self.x + 1, self.y)
    def ur(self):
	return self.r().u()
    def ul(self):
	return self.l().u()
    def dl(self):
	return self.l().d()
    def dr(self):
	return self.r().d()
	self.x = (self.x + 1) % self.width
    def copy(self):
	return Wrapcoords(self.width, self.height, self.x, self.y)
    def godir(self, dir):
	self.x += dirs[dir][0]
	self.y += dirs[dir][1]
	self.x %= self.width
	self.y %= self.height
    def dist2(self, other):
	x = (self.x - other.x + self.width / 2) % self.width - self.width / 2
	y = (self.y - other.y + self.height /2) % self.height - self.height / 2
	return x * x + y * y

class Map:
    def __init__(self, data, width, height):
	self.data = data
	self.width = width
	self.height = height
    def __getitem__(self, coords):
	return self.data[coords.y][coords.x]
    def __setitem__(self, coords, value):
	self.data[coords.y][coords.x] = value
    def d(self, loc, d):
	if d == 0 :
	    return self[loc] in FILLED + REC_LU + REC_RU
	elif d == 1:
	    return self[loc.u()] in FILLED + REC_LD + REC_RD
	elif d == 2:
	    return self[loc.u()] in FILLED + REC_LU + REC_LD
	elif d == 3:
	    return self[loc.ul()] in FILLED + REC_RU + REC_RD
	elif d == 4:
	    return self[loc.ul()] in FILLED + REC_LD + REC_RD
	elif d == 5:
	    return self[loc.l()] in FILLED + REC_RU + REC_LU
	elif d == 6:
	    return self[loc.l()] in FILLED + REC_RD + REC_RU
	elif d == 7:
	    return self[loc] in FILLED + REC_LU + REC_LU
    def coords(self, x = 0, y = 0):
	return Wrapcoords(self.width, self.height, x, y)

class Struct:
    pass

def convert(options):
    height = int(options['mapheight'])
    width = int(options['mapwidth'])
    map = Map(options['mapdata'].splitlines(), width, height)
    done = Map([[0] * width for i in range(height)], width, height)
    bases = []
    balls = []
    polys = []
    for loc in [map.coords(x, y) for x in range(width) for y in range(height)]:
	if map[loc] in '_0123456789':
	    base = Struct()
	    base.x = loc.x * BCLICKS + BCLICKS / 2
	    base.y = (height - loc.y - 1) % height * BCLICKS + BCLICKS / 2
	    if map[loc] == '_':
		base.team = 0
	    else:
		base.team = ord(map[loc]) - ord('0')
	    base.dir = 32
	    base.loc = loc.copy()
	    bases.append(base)
	if map[loc] == '*':
	    ball = Struct()
	    ball.x = loc.x * BCLICKS + BCLICKS / 2
	    ball.y = (height - loc.y - 1) % height * BCLICKS + 640;
	    ball.loc = loc.copy()
	    balls.append(ball)
	if done[loc]:
	    continue
	if map[loc] in FILLED and map[loc.l()] not in FILLED + REC_RD + REC_RU:
	    startloc = loc
	    dir = 6
	elif map[loc] in REC_RU:
	    startloc = loc
	    dir = 7
	elif map[loc] in REC_RD:
	    startloc = loc.r()
	    dir = 5
	else:
	    continue
	l = startloc.copy()
	poly = []
	while 1:
	    poly.append((l.x, l.y, dir))
	    if dir in [6, 7]:
		done[l] = 1
	    elif dir == 5:
		done[l.l()] =1
	    l.godir(dir)
	    if l == startloc:
		break
	    dir = (dir + 3) % 8
	    while map.d(l, dir):
		dir = (dir - 1) % 8
	polys.append(poly)
    for bl in balls:
	maxd = 30000 * 30000
	for bs in bases:
	    if bl.loc.dist2(bs.loc) < maxd:
		maxd = bl.loc.dist2(bs.loc)
		bl.team = bs.team
    polys2 = []
    for p in polys:
	polys2.append([[q[0] * BCLICKS, q[1] * BCLICKS] for q in p])

    # Invert the map vertically.
    for p in polys2:
	for p2 in p:
	    if p2[1]:  # Keep 0 at 0, don't move to maxheight + 1 !
		p2[1] = height * BCLICKS - p2[1]

    mxc = BCLICKS * width
    myc = BCLICKS * height
    # This part just for Bloods
    from math import sin
    for p in polys2:
	for l in p:
	    l[0] = int(l[0] + sin(1. * l[0] * l[1] / 2786) * (2240 / 3)) % mxc
	    l[1] = int(l[1] + sin(1. * l[0] * l[1] / 1523) * (2240 / 3)) % myc

    print "<XPilotMap>"

    print '<Featurecount bases="%d" balls="%d"/>' % (len(bases), len(balls))
    mapd = options['mapdata']
    del options['mapdata']
    md = '<Mapdata x="%d" y="%d"' % (width, height)
    tofind = '*_0123456789'
    attribnum = 65
    for pos in range(len(mapd)):
	if mapd[pos] in tofind:
	    md += ' foo%s="%s%d"' % (chr(attribnum), mapd[pos], pos)
	    attribnum += 1
    md += '/>'
    print md
    print '<GeneralOptions>'
    for name, value in options.items():
	print '<Option name="%s" value="%s"/>' % (name, encode(value))
    print '</GeneralOptions>'

    for p in polys2:
	print '<Polygon x="%d" y="%d">' % tuple(p[0])
	x = p[0][0]
	y = p[0][1]
	for c in p[1:]:
	    print '<Offset x="%d" y="%d"/>' % (center(c[0] - x, mxc), center(c[1] - y, myc))
	    x, y = c
	print "</Polygon>"
    for ball in balls:
	print '<Ball team="%d" x="%d" y="%d"/>' % (ball.team, ball.x, ball.y)
	print '<BallArea team="%d">' % ball.team
	print '<Polygon x="%d" y="%d">' % (ball.x - 1120, ball.y - 640)
	print '<Offset x="2240" y="0"/> <Offset x="0" y="2240"/>'
	print '<Offset x="-2240" y="0"/> <Offset x="0" y="-2240"/>'
	print '</Polygon></BallArea>'
	print '<BallTarget team="%d">' % ball.team
	print '<Polygon x="%d" y="%d">' % (ball.x - 480, ball.y)
	print '<Offset x="960" y="0"/> <Offset x="0" y="960"/>'
	print '<Offset x="-960" y="0"/> <Offset x="0" y="-960"/>'
	print '</Polygon></BallTarget>'
    for base in bases:
	print '<Base team="%d" x="%d" y="%d" dir = "%d"/>' % \
	      (base.team, base.x, base.y, base.dir)
    print "</XPilotMap>"

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
	print "Give 1 argument, the map file name"
	sys.exit(1)
    file = open(sys.argv[1])
    lines = file.readlines()
    file.close()
    options = parse(lines)
    removeopts(options)
    convert(options)
