#!/usr/bin/env python

import sys
import os
import wx
import wx.lib.mixins.listctrl as listmix
import meta

class AutoSizeListCtrl(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin):
	def __init__(self, parent, ID, pos=wx.DefaultPosition,
				 size=wx.DefaultSize, style=wx.LC_REPORT|wx.SUNKEN_BORDER):
		wx.ListCtrl.__init__(self, parent, ID, pos, size, style)
		listmix.ListCtrlAutoWidthMixin.__init__(self)

class Panel(wx.Panel):
	def __init__(self, parent):
		wx.Panel.__init__(self, parent, -1)
		self.serverList = AutoSizeListCtrl(self, -1)
		for label in [ "Server", "Version", "#", "Map" ]:
			self.serverList.InsertColumn(sys.maxint, label)
		self.detailList = AutoSizeListCtrl(self, -1)
		for label in [ "Property", "Value" ]:
			self.detailList.InsertColumn(sys.maxint, label)
		for label in [ "Address", "Port", "Size", "Author", "Status", "Bases",
					   "Teams", "Free bases", "Queue", "FPS", "Sound", "Timing",
					   "Uptime"]:
			self.detailList.InsertStringItem(sys.maxint, label)
		self.detailList.Layout()
		self.playerList = AutoSizeListCtrl(self, -1)
		self.playerList.InsertColumn(0, "Players")
		refresh = wx.Button(self, -1, "Refresh")
		self.Bind(wx.EVT_BUTTON, self.OnRefresh, refresh)
		join = wx.Button(self, -1, "Join")
		self.Bind(wx.EVT_BUTTON, self.OnJoin, join)
		box1 = wx.BoxSizer(wx.HORIZONTAL)
		box1.Add(join, 0, wx.ALL, 5)
		box1.Add((5,0))
		box1.Add(refresh, 0, wx.ALL, 5)
		box2 = wx.BoxSizer(wx.VERTICAL)
		box2.Add(self.detailList, 2, wx.EXPAND)
		box2.Add(self.playerList, 1, wx.EXPAND)
		box2.Add((1, 5))
		box2.Add(box1)
		box3 = wx.BoxSizer(wx.HORIZONTAL)
		box3.Add(self.serverList, wx.EXPAND, wx.EXPAND|wx.ALL, 5)
		box3.Add(box2, 0, wx.EXPAND|wx.ALL, 5)
		self.SetSizer(box3)
		self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.OnSelect, self.serverList)
	
	def RefreshList(self):
		list = self.serverList
		list.DeleteAllItems()
		self.servers = meta.fetch()
		self.servers.sort(lambda x, y: y.count-x.count)
		row = 0
		for server in self.servers:
			list.InsertStringItem(row, server.host)
			list.SetStringItem(row, 1, server.version)
			list.SetStringItem(row, 2, str(server.count))
			list.SetStringItem(row, 3, server.mapname)
			row = row + 1
		for i in range(4):
			list.SetColumnWidth(i, wx.LIST_AUTOSIZE)
		self.serverList.SetItemState(0, wx.LIST_STATE_SELECTED, 
									 wx.LIST_STATE_SELECTED )
		self.Layout()

	def OnSelect(self, evt):
		s = self.servers[evt.GetIndex()]
		self.selected = s
		items = [ s.ip, str(s.port), s.mapsize, s.author, s.status, 
				  str(s.bases), str(s.teambases), str(s.freebases), 
				  str(s.queue), str(s.fps), s.sound, s.timing, 
				  "%.1f days" % (s.uptime/(3600*24.0)) ]
		for i in range(len(items)):
			self.detailList.SetStringItem(i, 1, items[i])
		self.detailList.SetColumnWidth(1, wx.LIST_AUTOSIZE)
		self.playerList.DeleteAllItems()
		if len(s.playlist) > 0:
			for p in s.playlist.split(","):
				self.playerList.InsertStringItem(sys.maxint, p)
			self.playerList.SetColumnWidth(0, wx.LIST_AUTOSIZE)
	
	def OnJoin(self, evt):
		os.system("xpilot-ng-x11 %s -port %d -join"
				  % (self.selected.ip, self.selected.port))
	
	def OnRefresh(self, evt):
		self.RefreshList()
		

class Frame(wx.Frame):
	    def __init__(
			self, parent, ID, title, pos=wx.DefaultPosition,
            size=(800,600), style=wx.DEFAULT_FRAME_STYLE
			):
			wx.Frame.__init__(self, parent, ID, title, pos, size, style)
			panel = Panel(self)
			panel.RefreshList()
			box = wx.BoxSizer(wx.HORIZONTAL)
			box.Add(panel, wx.EXPAND, wx.EXPAND, 0, 0)
			self.SetSizer(box)

class App(wx.App):
	def OnInit(self):
		frame = Frame(None, -1, "XPilot Meta Interface")
		self.SetTopWindow(frame)
		frame.Show(True)
		return True

def main():
	app = App(0)
	app.MainLoop()

if __name__ == '__main__':
	main()
