# Requires python 2.0

# Doesn't yet work for maps where a single wall wraps all around the map
# back to the same point.

# Some parts of this code are really hacky and ugly, they're probably
# difficult to read even in Python. If this was Perl or something I
# probably couldn't read them myself...

removedopts = ['analyticalcollisiondetection', 'ecmsreprogramrobots', 'edgebounce', 'extraborder', 'maxdefensiveitems', 'maxoffensiveitems', 'maxrobots', 'maxshieldedplayerwallbounceangle', 'maxunshieldedplayerwallbounceangle', 'minrobots', 'oneplayeronly', 'reserverobotteam', 'restrictrobots', 'robotfile', 'robotleavelife', 'robotleaveratio', 'robotleavescore', 'robotteam', 'robots', 'robotsleave', 'robotstalk', 'scoretablefilename', 'teamassign', 'usewreckage']
# 'mapdata' has been removed too, but isn't ignored by this script.
# 'numberofrounds', 'numrounds' and 'roundstoplay' have been used for the same
# thing

knownopts = ['allowclusters', 'allowheatseekers', 'allowlasermodifiers', 'allowmodifiers', 'allownukes', 'allowplayerbounces', 'allowplayercrashes', 'allowplayerkilling', 'allowshields', 'allowshipshapes', 'allowsmartmissiles', 'allowtorpedoes', 'allowviewing', 'ballkillscoremult', 'ballswallbounce', 'baseminerange', 'cannonitemprobmult', 'cannonsmartness', 'cannonsuseitems', 'checkpointradius', 'cloakedexhaust', 'cloakedshield', 'clusterkillscoremult', 'contactport', 'crashscoremult', 'debriswallbounce', 'defaultsfilename', 'denyhosts', 'destroyitemincollisionprob', 'detonateitemonkillprob', 'distinguishmissiles', 'dropitemonkillprob', 'dump', 'ecmsreprogrammines', 'edgewrap', 'explosionkillscoremult', 'firerepeatrate', 'framespersecond', 'friction', 'gameduration', 'gravity', 'gravityangle', 'gravityanticlockwise', 'gravityclockwise', 'gravitypoint', 'gravitypointsource', 'gravityvisible', 'heatkillscoremult', 'help', 'identifymines', 'idlerun', 'ignore20maxfps', 'ignore20maxfps', 'initialafterburners', 'initialarmor', 'initialautopilots', 'initialcloaks', 'initialdeflectors', 'initialecms', 'initialemergencyshields', 'initialemergencythrusts', 'initialfuel', 'initialhyperjumps', 'initiallasers', 'initialmines', 'initialmirrors', 'initialmissiles', 'initialphasings', 'initialrearshots', 'initialsensors', 'initialtanks', 'initialtractorbeams', 'initialtransporters', 'initialwideangles', 'itemafterburnerprob', 'itemarmorprob', 'itemautopilotprob', 'itemcloakprob', 'itemconcentratorprob', 'itemconcentratorradius', 'itemconcentratorvisible', 'itemdeflectorprob', 'itemecmprob', 'itememergencyshieldprob', 'itememergencythrustprob', 'itemenergypackprob', 'itemhyperjumpprob', 'itemlaserprob', 'itemmineprob', 'itemmirrorprob', 'itemmissileprob', 'itemphasingprob', 'itemprobmult', 'itemrearshotprob', 'itemsensorprob', 'itemswallbounce', 'itemtankprob', 'itemtractorbeamprob', 'itemtransporterprob', 'itemwideangleprob', 'keepshots', 'laserisstungun', 'laserkillscoremult', 'limitedlives', 'limitedvisibility', 'lockotherteam', 'loseitemdestroys', 'mapauthor', 'mapfilename', 'mapheight', 'mapname', 'mapwidth', 'maxafterburners', 'maxarmor', 'maxautopilots', 'maxcloaks', 'maxdeflectors', 'maxecms', 'maxemergencyshields', 'maxemergencythrusts', 'maxfuel', 'maxhyperjumps', 'maxitemdensity', 'maxlasers', 'maxmines', 'maxminesperpack', 'maxmirrors', 'maxmissiles', 'maxmissilesperpack', 'maxobjectwallbouncespeed', 'maxphasings', 'maxplayershots', 'maxrearshots', 'maxroundtime', 'maxsensors', 'maxshieldedwallbouncespeed', 'maxtanks', 'maxtractorbeams', 'maxtransporters', 'maxunshieldedwallbouncespeed', 'maxvisibilitydistance', 'maxwideangles', 'minefusetime', 'minelife', 'minescoremult', 'minesonradar', 'mineswallbounce', 'minvisibilitydistance', 'missilelife', 'missilesonradar', 'missileswallbounce', 'movingitemprob', 'noquit', 'nukeclusterdamage', 'nukeminmines', 'nukeminsmarts', 'nukesonradar', 'numberofrounds', 'objectwallbouncebrakefactor', 'objectwallbouncelifefactor', 'password', 'playerlimit', 'playersonradar', 'playerstartsshielded', 'playerwallbouncebrakefactor', 'plockserver', 'racelaps', 'recordmode', 'reporttometaserver', 'reset', 'resetonhuman', 'rogueheatprob', 'roguemineprob', 'rounddelay', 'runoverkillscoremult', 'searchdomainforxpilot', 'shieldeditempickup', 'shieldedmining', 'shipmass', 'shotkillscoremult', 'shotlife', 'shotmass', 'shotsgravity', 'shotspeed', 'shotswallbounce', 'shovekillscoremult', 'smartkillscoremult', 'sparkswallbounce', 'tankkillscoremult', 'targetkillteam', 'targetsync', 'targetteamcollision', 'teamcannons', 'teamfuel', 'teamimmunity', 'teamplay', 'timerresolution', 'timing', 'torpedokillscoremult', 'treasurecollisiondestroys', 'treasurecollisionmaykill', 'treasurekillteam', 'treasuresonradar', 'version', 'wallbouncedestroyitemprob', 'wallbouncefueldrainmult', 'worldlives', 'wormholevisible', 'wormtime', 'wreckagecollisionmaykill', 'mapdata']

def checkopts(options):
    dany = 0
    unknown = []
    for opt in options.keys():
	if opt in removedopts:
	    if not dany:
		dany = 1
		print >> sys.stderr, "Removing the following options:"
	    print >>sys.stderr, opt + "  ",
	    del options[opt]
	elif opt not in knownopts:
	    del options[opt]
	    unknown.append(opt)
    if dany:
	print >> sys.stderr
    for opt in unknown:
	print >> sys.stderr, "WARNING did not recognize option %s, removed!" % opt

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
ATTRACT = '$'
BCLICKS = 35 * 64
MAXLEN = 30000

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

	options['mapdata'] = "\n".join(map)+'\n'
    options['mapwidth'] = `width * 35`
    options['mapheight'] = `height * 35`
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
	    if map[loc.u()] in ATTRACT:
		base.dir = 32
	    elif map[loc.d()] in ATTRACT:
		base.dir = 96
	    elif map[loc.r()] in ATTRACT:
		base.dir = 0
	    elif map[loc.l()] in ATTRACT:
		base.dir = 64
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
	    check.loc = loc.copy()
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
    if (options.get('timing') or options.get('race')) in ['yes', 'on', 'true']:
	bases = [(checks[0].loc.dist2(b.loc), b) for b in bases]
	bases.sort()
	bases = [b[1] for b in bases]
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
        direction = polydir(p)
	if direction > 0:
	    poslist.append(p)
	elif direction < 0:
	    neglist.append(p)
        else:
            print >> sys.stderr, "Converting this map isn't supported (yet?)"
            sys.exit(1)
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
	    res[o] = res[o][:]
	    res[o][2] = None
	    res[o + len(r)] = res[o + len(r)][:]
	    res[o + len(r)][2] = None
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

    mapd = options['mapdata']
    del options['mapdata']
    print '<GeneralOptions>'
    for name, value in options.items():
	print '<Option name="%s" value="%s"/>' % (name, encode(value))
    print '</GeneralOptions>'
    print '<Edgestyle id="xpbluehidden" width="-1" color="4E7CFF" style="0"/>'
    print '<Edgestyle id="xpredhidden" width="-1" color="FF3A27" style="0"/>'
    print '<Edgestyle id="yellow" width="2" color="FFFF00" style="0"/>'
    print '<Polystyle id="xpblue" color="4E7CFF" defedge="xpbluehidden" flags="1"/>'
    print '<Polystyle id="xpred" color="FF3A27" defedge="xpredhidden" flags="1"/>'
    print '<Polystyle id="emptyyellow" color="FF" defedge="yellow" flags="0"/>'

    for p in polys2:
	print '<Polygon x="%d" y="%d" style="xpblue">' % tuple(p[-1][:2])
	x = p[-1][0]
	y = p[-1][1]
	h = not p[-1][2]
	curx = 0
	cury = 0
	curh = h
        prevh = 0
        sstr = ''
	for c in p:
	    dx = center(c[0] - x, mxc)
	    dy = center(c[1] - y, myc)
	    if dx * cury != dy * curx or curh != h:
                if curh and not prevh:
		    sstr = ' style="internal"'
		elif prevh and not curh:
                    sstr = ' style="xpbluehidden"'
		for i in range((max(abs(curx), abs(cury)) + MAXLEN - 1) / MAXLEN, 0, -1):
		    print '<Offset x="%d" y="%d"%s/>' % (curx / i, cury / i, sstr)
                    sstr= ''
		    curx -= curx / i
		    cury -= cury / i
		curx = dx
		cury = dy
                prevh = curh
		curh = h
	    else:
		curx += dx
		cury += dy
	    x, y, h = c
	    h = not h
	for i in range((max(abs(curx), abs(cury)) + MAXLEN - 1) / MAXLEN, 0, -1):
	    print '<Offset x="%d" y="%d"%s/>' % (curx / i, cury / i, sstr)
	    curx -= curx / i
	    cury -= cury / i
	print "</Polygon>"
# The styles of these polygons will be changed later...
    for ball in balls:
	print '<Ball team="%d" x="%d" y="%d"/>' % (ball.team, ball.x, ball.y)
	print '<BallArea>'
	print '<Polygon x="%d" y="%d" style="xpred">' % (ball.x - 1120, ball.y - 640)
	print '<Offset x="2240" y="0"/> <Offset x="0" y="2240"/>'
	print '<Offset x="-2240" y="0"/> <Offset x="0" y="-2240"/>'
	print '</Polygon></BallArea>'
	print '<BallTarget team="%d">' % ball.team
	print '<Polygon x="%d" y="%d" style="emptyyellow">' % (ball.x - 480, ball.y)
	print '<Offset x="960" y="0"/> <Offset x="0" y="960"/>'
	print '<Offset x="-960" y="0"/> <Offset x="0" y="-960"/>'
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
    checkopts(options)
    convert(options)
