# Requires python 2.0

removedopts = ['extraborder', 'edgebounce', "maxshieldedplayerwallbounceangle",
"maxshieldedbounceangle", "maxunshieldedplayerwallbounceangle",
"maxunshieldedbounceangle", "scoretablefilename", "scoretable", 'teamassign',
'oneplayeronly', "maxrobots", "robots", "minrobots", "robotfile", "robotstalk",
"robotsleave", "robotleavelife", "robotleavescore", "robotleaveratio",
"robotteam", "restrictrobots", "reserverobotteam", "ecmsreprogramrobots"]
# 'mapdata' has been removed too, but isn't ignored by this script.


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

def polydir(poly):
    xd, yd = poly[-1][2]
    wind = 0
    for point in poly:
	if (yd >= 0) ^ (point[2][1] >= 0):
	    side = xd * point[2][1] - point[2][0] * yd
	    if side != 0:
		if side > 0:
		    wind += 1
		else:
		    wind -= 1
	xd, yd = point[2]
    return wind / 2

def dist2(x1, y1, x2, y2, width, height):
    x = (x1 - x2 + (width >> 1)) % width - (width >> 1)
    y = (y1 - y2 + (height >> 1)) % height - (height >> 1)
    return 1. * x * x + 1. * y * y

# Only for polygons converted from blocks and going in the right direction.
def inside(x, y, poly, width, height):
    lastdir = poly[-1][2]
    mindir1 = None
    mindist = width
    for p in poly:
	if p[1] == y and (p[0] - x) % width < mindist:
	    mindir1 = lastdir
	    mindir2 = p[2]
	    mindist = (p[0] - x) % width
	lastdir = p[2]
    if not mindir1:
	return 0
    elif mindir1[0] + mindir2[0] <= 0:
	return mindir2[1] > 0
    else:
	return mindir1[1] > 0

def closestteam(loc, bases):
    maxd = 30000 * 30000
    for bs in bases:
	if loc.dist2(bs.loc) < maxd:
	    maxd = loc.dist2(bs.loc)
	    ans = bs.team
    return ans

def convert(options):
    height = int(options['mapheight'])
    width = int(options['mapwidth'])
    map = options['mapdata'].splitlines()
    if options.get('edgewrap') not in ['yes', 'on', 'true']: #default off
	height += 2
	width += 2
	map = [' ' + line + ' ' for line in map]
	map = [' ' * width] + map + [' ' * width]
	options['mapwidth'] = `width`
	options['mapheight'] = `height`
	options['mapdata'] = "\n".join(map)+'\n'
    map = Map(map, width, height)
    done = Map([[0] * width for i in range(height)], width, height)
    bases = []
    balls = []
    fuels = []
    checks = [None] * 27  # 1 extra so it always ends with None
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
	    ball.y = (height - loc.y - 1) % height * BCLICKS + 640
	    ball.loc = loc.copy()
	    balls.append(ball)
	if map[loc] == '#':
	    fuel = Struct()
	    fuel.x = loc.x * BCLICKS + BCLICKS / 2
	    fuel.y = (height - loc.y - 1) % height * BCLICKS + BCLICKS / 2
	    fuel.loc = loc.copy()
	    fuels.append(fuel)
	if map[loc] in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
	    check = Struct()
	    check.x = loc.x * BCLICKS + BCLICKS / 2
	    check.y = (height - loc.y - 1) % height * BCLICKS + BCLICKS / 2
	    checks[ord(map[loc]) - ord('A')] = check
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
	startdir = dir
	poly = []
	while 1:
	    poly.append((l.x, l.y, dirs[dir]))
	    if dir in [6, 7]:
		done[l] = 1
	    elif dir == 5:
		done[l.l()] = 1
	    l.godir(dir)
	    dir = (dir + 3) % 8
	    while map.d(l, dir):
		dir = (dir - 1) % 8
	    if l == startloc and dir == startdir:
		break
	polys.append(poly)
    mxc = BCLICKS * width
    myc = BCLICKS * height
    for ball in balls:
	ball.team = closestteam(ball.loc, bases)
    if options.get('teamfuel') in ['yes', 'on', 'true']: #default off
	for fuel in fuels:
	    fuel.team = closestteam(fuel.loc, bases)
    else:
	for fuel in fuels:
	    fuel.team = -1
    polys2 = []
    for p in polys:
	polys2.append([[q[0] * BCLICKS, q[1] * BCLICKS, q[2]] for q in p])
    # Invert the map vertically.
    for p in polys2:
	for p2 in p:
	    if p2[1]:  # Keep 0 at 0, don't move to maxheight + 1 !
		p2[1] = height * BCLICKS - p2[1]
	    p2[2] = (p2[2][0], -p2[2][1])
    neglist = []
    poslist = [] 
    for p in polys2:
	if polydir(p) > 0:
	    poslist.append(p)
	else:
	    neglist.append(p)
    totlist = neglist + poslist
    negcount = len(neglist)
    poscount = len(poslist)
    totcount = len(totlist)
    cdict = [{} for i in range(totcount)]
    clist = [[] for i in range(totcount)]
    for i in range(negcount):
	closest = 0
	for j in range(negcount, negcount + poscount):
	    if closest and cdict[j].has_key(closest):
		continue
	    if inside(neglist[i][0][0], neglist[i][0][1], totlist[j], mxc, myc):
		if not closest or cdict[closest].has_key(j):
		    closest = j
		else:
		    if inside(totlist[closest][0][0], totlist[closest][0][1], totlist[j], mxc, myc):
			cdict[j][closest] = 1
		    else:
			cdict[closest][j] = 1
			closest = j
	if closest:
	    clist[closest].append(i)
    reslist = []
    for i in range(negcount, totcount):
	if not clist[i]:
	    reslist.append(totlist[i])
	    continue
	dist = [(1e98, 0, 0)] * len(clist[i])
	count = len(clist[i])
	res = totlist[i]
	offset = 0
	last = i
	while count > 0:
	    for j in range(len(clist[i])):
		if clist[i][j] < 0:
		    continue
		r = totlist[clist[i][j]]
		mindist = dist[j][0]
		for k in range(len(totlist[last])):
		    x = totlist[last][k][0]
		    y = totlist[last][k][1]
		    for l in range(len(r)):
			d = dist2(r[l][0], r[l][1], x, y, mxc, myc)
			if d < mindist:
			    mindist = d
			    dist[j] = (mindist, k + offset, l)
	    mindist = 1e99
	    mink = -2.1
	    for k in range(len(clist[i])):
		if clist[i][k] < 0:
		    continue
		if dist[k][0] < mindist:
		    mink = k
		    mindist = dist[k][0]
	    r = totlist[clist[i][mink]]
	    r = r[dist[mink][2]:] + r[:dist[mink][2]+1]
	    totlist[clist[i][mink]] = r
	    o = dist[mink][1]
	    offset = o + 1
	    res = res[:o + 1] + r + res[o:]
	    last = clist[i][mink]
	    clist[i][mink] = -1
	    count -= 1
	    for k in range(len(clist[i])):
		if clist[i][k] < 0:
		    continue
		if dist[k][1] > o:
		    dist[k] = (dist[k][0], dist[k][1] + len(r) + 1, dist[k][2])
	reslist.append(res)
    polys2 = reslist
		    
    # Turn this on if you want to randomize the map edges
    if 0:
	from math import sin
	for p in polys2:
	    for l in p:
		l[0] = int(l[0] + sin(1. * l[0] * l[1] / 2786) * (2240 / 3)) % mxc
		l[1] = int(l[1] + sin(1. * l[0] * l[1] / 1523) * (2240 / 3)) % myc

    print "<XPilotMap>"

    print '<Featurecount bases="%d" balls="%d" fuels="%d" checks="%d"/>' % (
	len(bases), len(balls), len(fuels), checks.index(None))
    mapd = options['mapdata']
    del options['mapdata']
    print '<GeneralOptions>'
    for name, value in options.items():
	print '<Option name="%s" value="%s"/>' % (name, encode(value))
    print '</GeneralOptions>'

    for p in polys2:
	print '<Polygon x="%d" y="%d">' % tuple(p[0][:2])
	x = p[0][0]
	y = p[0][1]
	for c in p[1:]:
	    print '<Offset x="%d" y="%d"/>' % (center(c[0] - x, mxc), center(c[1] - y, myc))
	    x, y = c[:2]
	print "</Polygon>"
    for ball in balls:
	print '<Ball team="%d" x="%d" y="%d"/>' % (ball.team, ball.x, ball.y)
	print '<BallArea team="%d">' % ball.team
	print '<Polygon x="%d" y="%d">' % (ball.x - 1120, ball.y - 640)
	print '<Offset x="2240" y="0"/> <Offset x="0" y="2240"/>'
	print '<Offset x="-2240" y="0"/>'
	print '</Polygon></BallArea>'
	print '<BallTarget team="%d">' % ball.team
	print '<Polygon x="%d" y="%d">' % (ball.x - 480, ball.y)
	print '<Offset x="960" y="0"/> <Offset x="0" y="960"/>'
	print '<Offset x="-960" y="0"/>'
	print '</Polygon></BallTarget>'
    for base in bases:
	print '<Base team="%d" x="%d" y="%d" dir = "%d"/>' % \
	      (base.team, base.x, base.y, base.dir)
    for fuel in fuels:
	print ('<Fuel x="%d" y="%d"' % (fuel.x, fuel.y)),
	if fuel.team != -1:
	    print (' team="%d"' % fuel.team),
	print '/>'
    for check in checks:
	if not check:
	    break
	print '<Check x="%d" y="%d"/>' % (check.x, check.y)
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
