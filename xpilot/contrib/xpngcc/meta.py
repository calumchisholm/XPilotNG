import socket
import StringIO
from string import *

def fetch():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		s.connect(("meta.xpilot.org", 4401))
		data = ""
		while True:
			buf = s.recv(1024)
			if not buf:	return parse(data)
			data += buf
	finally:
		s.close()
		
def parse(data):
	lines = StringIO.StringIO(data).readlines()
	servers = []
	for line in lines: servers.append(Server(line))
	return servers

class Server:
	def __init__(self, line):
		self.line = strip(line)
		fields = split(self.line, ":")
		self.version = fields[0]
		self.host = fields[1]
		self.port = atoi(fields[2])
		self.count = atoi(fields[3])
		self.mapname = fields[4]
		self.mapsize = fields[5]
		self.author = fields[6]
		self.status = fields[7]
		self.bases = atoi(fields[8])
		self.fps = atoi(fields[9])
		self.playlist = fields[10]
		self.sound = fields[11]
		self.teambases = atoi(fields[13])
		self.uptime = atoi(fields[12])
		self.timing = fields[14]
		self.ip = fields[15]
		self.freebases = fields[16]
		self.queue = atoi(fields[17])
        
	def __str__(self):
		return "Server:" + self.line
        
            
