import os
import sys

meta = ('meta.xpilot.org', 4401)
client = 'xpilot-ng-x11'
server = 'xpilot-ng-server'
if (sys.platform == "win32"):
	xpilotrc = 'C:\\Program Files\\XPilotNG-SDL\\xpilotrc.txt'
else:
	xpilotrc = os.environ['HOME'] + '/.xpilotrc'
record_url = 'http://xpilot.sourceforge.net/ballruns/'
xpreplay = 'xpilot-ng-replay'
jxpmap_url = 'http://xpilot.sf.net/jxpmap/jxpmap.jnlp'
javaws = 'javaws'
mapedit = 'xpilot-ng-xp-mapedit'
irc_server = 'irc.freenode.net'
irc_channel = '#xpilot'
if (sys.platform == "win32"):
	mapdir = 'C:\\Program Files\\XPilotNG-SDL\\lib\\maps'
else:
	mapdir = '/usr/local/share/xpilot-ng/maps'
