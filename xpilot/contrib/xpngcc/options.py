import re
import os
import wx
import wx.lib.mixins.listctrl as listmix

class OptionListCtrl(wx.ListCtrl, listmix.ListCtrlAutoWidthMixin):
	def __init__(self, parent, ID, pos=wx.DefaultPosition,
				 size=wx.DefaultSize, style=wx.LC_REPORT|wx.SUNKEN_BORDER):
		wx.ListCtrl.__init__(self, parent, ID, pos, size, style)
		listmix.ListCtrlAutoWidthMixin.__init__(self)
		self.InsertColumn(0, 'Name')
		self.InsertColumn(1, 'Value')
		self.InsertColumn(2, 'Description')
	def set_value(self, row, newval):
		if newval: self.SetStringItem(row, 1, newval)
		else: self.SetStringItem(row, 1, '<default>')
	def set_options(self, opts):
		self.DeleteAllItems()
		row = 0
		for opt in opts:
			self.InsertStringItem(row, opt.names[0])
			if opt.value: self.SetStringItem(row, 1, opt.value)
			else: self.SetStringItem(row, 1, '<default>')
			self.SetStringItem(row, 2, opt.desc)
			row += 1
		if row:
			self.SetColumnWidth(0, wx.LIST_AUTOSIZE)
			self.resizeLastColumn(100)

class OptionViewPanel(wx.Panel):
	def __init__(self, parent):
		wx.Panel.__init__(self, parent, -1)
		self.option = None
		sz = wx.BoxSizer(wx.HORIZONTAL)
		self.SetSizer(sz)
		p = wx.Panel(self)
		ps = wx.FlexGridSizer(2,2)
		ps.AddGrowableCol(1)
		p.SetSizer(ps)
		l = wx.StaticText(p, -1, "Name:")
		l.SetForegroundColour(wx.Color(255,255,255))
		ps.Add(l, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		self.name = wx.TextCtrl(p, style=wx.TE_READONLY)
		ps.Add(self.name, 0, 
			   wx.EXPAND|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		l = wx.StaticText(p, -1, "Value:")
		l.SetForegroundColour(wx.Color(255,255,255))
		ps.Add(l, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		self.value = wx.TextCtrl(p)
		ps.Add(self.value, 0, 
			   wx.EXPAND|wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		sz.Add(p, wx.EXPAND, wx.EXPAND, 0)
		self.desc = wx.TextCtrl(self, -1, 
								style=wx.TE_READONLY|wx.TE_MULTILINE)
		sz.Add(self.desc, wx.EXPAND, wx.EXPAND|wx.ALL, 5)
		p = wx.Panel(self)
		ps = wx.BoxSizer(wx.VERTICAL)
		p.SetSizer(ps)
		b = wx.Button(p, -1, "Apply")
		self.Bind(wx.EVT_BUTTON, self.on_apply, b)
		self.apply = b
		ps.Add(b, 0, wx.BOTTOM, 2)
		b = wx.Button(p, -1, "Default")
		self.Bind(wx.EVT_BUTTON, self.on_default, b)
		self.default = b
		ps.Add(b, 0, wx.TOP, 2)
		sz.Add(p, 0, wx.ALL, 5)
		self.clear()
	def clear(self):
		self.option = None
		self.name.SetValue('')
		self.value.SetValue('')
		self.desc.SetValue('')
		self.apply.Disable()
		self.default.Disable()
	def show_option(self, opt):
		self.apply.Enable(True)
		self.default.Enable(True)
		self.option = opt
		self.name.SetValue(str(opt.names)[1:-1])
		if opt.value: self.value.SetValue(opt.value)
		else: self.value.SetValue('')
		self.desc.SetValue(opt.desc)
	def on_apply(self, evt):
		self.option.value = self.value.GetValue()
		self.GetParent().apply()
	def on_default(self, evt):
		self.option.value = None
		self.value.SetValue('')
		self.GetParent().apply()

class FilterPanel(wx.Panel):
	def __init__(self, parent, action):
		wx.Panel.__init__(self, parent, -1)
		self.action = action
		sz = wx.BoxSizer(wx.HORIZONTAL)
		self.SetSizer(sz)
		l = wx.StaticText(self, -1, "Filter:")
		l.SetForegroundColour(wx.Color(255,255,255))
		sz.Add(l, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		self.text = wx.TextCtrl(self, -1, '')
		sz.Add(self.text, wx.EXPAND, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)
		b = wx.Button(self, -1, "Filter")
		self.Bind(wx.EVT_BUTTON, self.on_filter, b)
		sz.Add(b, 0, wx.ALL, 5)
		b = wx.Button(self, -1, "Show All")
		self.Bind(wx.EVT_BUTTON, self.on_show_all, b)
		sz.Add(b, 0, wx.ALL, 5)
		b = wx.Button(self, -1, action[0])
		self.Bind(wx.EVT_BUTTON, self.on_action, b)
		sz.Add(b, 0, wx.ALL, 5)
	def on_filter(self, evt):
		self.GetParent().filter(self.text.GetValue())
	def on_show_all(self, evt):
		self.text.SetValue('')
		self.GetParent().filter(None)
	def on_action(self, evt):
		self.action[1]()
	def set_filter(self, str):
		if str:	self.text.SetValue(str)

class OptionsPanel(wx.Panel):
	def __init__(self, parent, options, action):
		wx.Panel.__init__(self, parent, -1)
		self.options = options
		self.list = OptionListCtrl(self, -1)
		self.Bind(wx.EVT_LIST_ITEM_SELECTED, self.on_select, self.list)
		sz = wx.BoxSizer(wx.VERTICAL)
		self.filter_panel = FilterPanel(self, action)
		sz.Add(self.filter_panel, 0, wx.EXPAND, 0)
		sz.Add(self.list, wx.EXPAND, wx.EXPAND|wx.ALL, 5)
		self.view = OptionViewPanel(self)
		sz.Add(self.view, 0, wx.EXPAND, 0)
		self.SetSizer(sz)
	def apply(self):
		self.list.set_value(self.row, self.active.value)
	def filter(self, str):
		self.filter_panel.set_filter(str)
		self.view.clear()
		if str:
			self.active_options = [o for o in self.options if o.matches(str) ]
		else: self.active_options = self.options
		self.list.set_options(self.active_options)
	def on_select(self, evt):
		self.row = evt.GetIndex()
		self.active = self.active_options[self.row]
		self.view.show_option(self.active)

class ClientOptionsPanel(OptionsPanel):
	def __init__(self, parent, client, xpilotrc):
		self.xpilotrc = xpilotrc
		opts = parse_options([client, '-help'])
		if os.path.exists(xpilotrc):
			vals = parse_xpilotrc(xpilotrc)
			join_options_with_values(opts, vals)
		sort_options(opts)
		OptionsPanel.__init__(self, parent, opts, ('Save', self.save))
		self.filter(None)
	def save(self):
		if wx.MessageDialog(self, 
							"Save current settings to %s?" % self.xpilotrc,
							"Are you sure?").ShowModal() == wx.ID_OK:
			f = file(self.xpilotrc,'w')
			for o in self.options:
				if o.value:
					o.write(f)
			f.close()

class ServerOptionsPanel(OptionsPanel):
	def __init__(self, parent, server):
		self.server = server
		opts = parse_options([server, '-help'])
		sort_options(opts)
		OptionsPanel.__init__(self, parent, opts, ('Start', self.start))
		self.filter('[ Flags: command,')
	def start(self):
		opts = [self.server]
		for o in self.options:
			if o.value:
				if o.type == '<bool>':
					if o.value == 'true':
						opts.append('-' + o.names[0])
					else:
						opts.append('+' + o.names[0])
				else:
					opts.append('-' + o.names[0])
					opts.append(o.value)
		os.system(reduce(lambda x,y: x + ' ' + y, opts) + '&')

class Option:
	def __init__(self, names, type, desc, value):
		self.names = names
		self.type = type
		self.desc = desc
		self.value = value
	def __str__(self):
		return str((self.names, self.type, self.desc, self.value))
	def matches(self, query):
		for name in self.names:
			if name.find(query) != -1: return True
		if self.desc.find(query) != -1: return True
		return False
	def write(self, f):
		f.write('xpilot.' + self.names[0] + ': ' + self.value + '\n')

class Popen:
	def __init__(self, cmd):
		self.stdout = os.popen2(cmd)[1]
	def wait(self):
		return 0

def parse_options(cmd):
	opts = []
	r = re.compile('    -(/\+)?([^<]*)(<\w*>)?')
	p = Popen(cmd)
	for line in p.stdout:
		m = r.match(line)
		if not m: continue
		if m.group(1): type = '<bool>'
		elif m.group(3): type = m.group(3)
		else: continue
		names = [x for x in m.group(2).split() if x != 'or']
		desc = ''
		while 1:
			line = p.stdout.next().strip()
			if not line: break
			desc += ' ' + line
		opts.append(Option(names, type, desc.lstrip(), None))
	p.wait()
	return opts

def parse_xpilotrc(fn):
	vals = {}
	r = re.compile('xpilot\.(\w*)[\s:=]*(.*)')
	f = file(fn)
	try:
		for line in f:
			line = line.strip()
			m = r.match(line)
			if not m: continue
			vals[m.group(1)] = m.group(2)
		return vals
	finally:
		f.close()

def join_options_with_values(opts, vals):
	for opt in opts:
		for name in opt.names:
			if name not in vals:
				continue
			opt.value = vals[name]
			break

def sort_options(opts):
	opts.sort(lambda o1,o2: cmp(o1.names[0], o2.names[0]))


class Frame(wx.Frame):
	    def __init__(
			self, parent, ID, title, pos=wx.DefaultPosition,
            size=(800,600), style=wx.DEFAULT_FRAME_STYLE
			):
			wx.Frame.__init__(self, parent, ID, title, pos, size, style)
			box = wx.BoxSizer(wx.VERTICAL)
			box.Add(ClientOptionsPanel(
					self, 'xpilot-ng-sdl', os.environ['HOME'] + '/.xpilotrc'),
					wx.EXPAND, wx.EXPAND, 0, 0)
			self.SetSizer(box)

class App(wx.App):
	def OnInit(self):
		frame = Frame(None, -1, "")
		self.SetTopWindow(frame)
		frame.Show(True)
		return True

if __name__ == '__main__':
	app = App(0)
	app.MainLoop()
