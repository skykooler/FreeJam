#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os

import pyglet


from pyglet import clock

from pyglet.window import key

import threading

import math

import rtmidi


stageimg = pyglet.image.load('resources/image-textures/floor.png')
barimg = pyglet.image.load('resources/image-textures/bar.png')
record_img = pyglet.image.load('resources/image-textures/record_b.png')
recording_img = pyglet.image.load('resources/image-textures/record_active.png')
rw_beg_img = pyglet.image.load('resources/image-textures/rewind_to_beginning.png')
rw_img = pyglet.image.load('resources/image-textures/rewind.png')
play_img = pyglet.image.load('resources/image-textures/play.png')
pause_img = pyglet.image.load('resources/image-textures/pause.png')
ff_img = pyglet.image.load('resources/image-textures/fastforward.png')
ff_end_img = pyglet.image.load('resources/image-textures/forward_to_end.png')


tsa_img = pyglet.image.load('resources/image-textures/track_software_active.png')
tra_img = pyglet.image.load('resources/image-textures/track_real_active.png')
tla_img = pyglet.image.load('resources/image-textures/track_loop_active.png')
toa_img = pyglet.image.load('resources/image-textures/track_other_active.png')
tg_img = pyglet.image.load('resources/image-textures/track_generic.png')

tbg_s_img = pyglet.image.load('resources/image-textures/track_software_bg.png')
tbg_s_l_img = pyglet.image.load('resources/image-textures/track_software_bg_left.png')
tbg_s_r_img = pyglet.image.load('resources/image-textures/track_software_bg_right.png')
tbg_r_img = pyglet.image.load('resources/image-textures/track_real_bg.png')
tbg_r_l_img = pyglet.image.load('resources/image-textures/track_real_bg_left.png')
tbg_r_r_img = pyglet.image.load('resources/image-textures/track_real_bg_right.png')
tbg_l_img = pyglet.image.load('resources/image-textures/track_loop_bg.png')
tbg_l_l_img = pyglet.image.load('resources/image-textures/track_loop_bg_left.png')
tbg_l_r_img = pyglet.image.load('resources/image-textures/track_loop_bg_right.png')
tbg_o_img = pyglet.image.load('resources/image-textures/track_other_bg.png')
tbg_o_l_img = pyglet.image.load('resources/image-textures/track_other_bg_left.png')
tbg_o_r_img = pyglet.image.load('resources/image-textures/track_other_bg_right.png')

trackbg_img = pyglet.image.load('resources/image-textures/trackbg.png')

# Colors for fills. OpenGL is overdrawing my stuff if I use primitives.
rgb184184184= pyglet.image.load('resources/image-textures/colors/184-184-184.png')
rgb084084084= pyglet.image.load('resources/image-textures/colors/084-084-084.png')
rgb255000000= pyglet.image.load('resources/image-textures/colors/255-000-000.png')

key_cf_img = pyglet.image.load('resources/image-textures/key_cf.png')
key_dga_img = pyglet.image.load('resources/image-textures/key_dga.png')
key_eb_img = pyglet.image.load('resources/image-textures/key_eb.png')
key_black_img = pyglet.image.load('resources/image-textures/key_black.png')
key_cf_pressed_img = pyglet.image.load('resources/image-textures/key_cf_pressed.png')
key_dga_pressed_img = pyglet.image.load('resources/image-textures/key_dga_pressed.png')
key_eb_pressed_img = pyglet.image.load('resources/image-textures/key_eb_pressed.png')
key_black_pressed_img = pyglet.image.load('resources/image-textures/key_black_pressed.png')
key_cf = pyglet.sprite.Sprite(key_cf_img)
key_dga = pyglet.sprite.Sprite(key_dga_img)
key_eb = pyglet.sprite.Sprite(key_eb_img)
key_black = pyglet.sprite.Sprite(key_black_img)

key_cf_pressed = pyglet.sprite.Sprite(key_cf_pressed_img)
key_dga_pressed = pyglet.sprite.Sprite(key_dga_pressed_img)
key_eb_pressed = pyglet.sprite.Sprite(key_eb_pressed_img)
key_black_pressed = pyglet.sprite.Sprite(key_black_pressed_img)


window = pyglet.window.Window(resizable=True)
window.maximize()

#### Global variables ######
global vbarloc
global dragging_vbar
global STEP
global VIEW_SCALE
global SCROLL 
global INDEX
global PLAYBACK_SPEED
global PLAYMODE
global KEYPRESS_MASK
global playing
global recording
vbarloc = 100
dragging_vbar = False
STEP = 64 # maximum res is 64th-notes
VIEW_SCALE = 10.0
SCROLL = 0
INDEX = 0
PLAYBACK_SPEED = 16.0
PLAYMODE = 'keyboard'
KEYPRESS_MASK = []
NOTE_RATIOS = [1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887]
KEYMAP = {key.A:36,key.W:37,key.S:38,key.E:39,key.D:40,key.F:41,key.T:42,
			key.G:43,key.Y:44,key.H:45,key.U:46,key.J:47,key.K:48,
			key.O:49,key.L:50,key.P:51,key.SEMICOLON:52,key.APOSTROPHE:53}
playing = False
recording = False
############################

stagesprite = pyglet.sprite.Sprite(stageimg)

pyglet.resource.path = ['instruments/keyboards/piano1']
pyglet.resource.reindex()

def draw(*args):
	#Dummy function to redraw the window
	pass
	
def play_note(noteval, velocity):
	player = pyglet.media.ManagedSoundPlayer()
	player.queue(c)
	player.pitch = note(noteval)
	player.volume = velocity
	player.play()
	if recording:
		ACTIVETRACK.tracks[-1].data[-1].append(noteval)

SIDE_WIDTH = 150
def draw_track(track,num,width):
	BASE_HEIGHT=(vbarloc-barimg.height)-((num+1)*track.img().height)
	trackbg_img.blit(0,BASE_HEIGHT,width=width)
	for i in xrange(*VIEW_EXTENTS):
		if i%STEP==0:
			rgb084084084.blit((i-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+4,height=tsa_img.height-4)
	track_index = 0
	time_index = 0
	for i in track.tracks:
		if not i.is_silence():
			pass
			#print time_index
			track.leftimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+4)
			track.cimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+5,BASE_HEIGHT+4,width=len(i)*VIEW_SCALE-8)
			track.rightimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+len(i)*VIEW_SCALE-4,BASE_HEIGHT+4)
			for j in xrange(len(i.data)):
				if i.data[j]:
					for k in i.data[j]:
						rgb084084084.blit((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,BASE_HEIGHT+8+k%28,width=10)
		time_index+=len(i)
		track_index+=1
	track.img().blit(0,BASE_HEIGHT,width=SIDE_WIDTH,height=track.img().height)
	track.labelshadow.x = 15
	track.labelshadow.y = (vbarloc-barimg.height)-(num*track.img().height+track.label.font_size)-2
	track.label.x = 16
	track.label.y = (vbarloc-barimg.height)-(num*track.img().height+track.label.font_size)-3
	track.labelshadow.draw()
	track.label.draw()

@window.event
def on_draw():
	window.clear()
	width, height = window.get_size()
	global VIEW_EXTENTS
	VIEW_EXTENTS = (0,width)	# for now, at least
	if PLAYMODE=='stage':
		stagesprite.scale = max(width/(stageimg.width*1.0), height/(stageimg.height*1.0))
		stagesprite.y = vbarloc
		stagesprite.draw()
	elif PLAYMODE=='keyboard':
		keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.125)
		index = 0
		key_cf.scale = keyscale
		key_dga.scale = keyscale
		key_eb.scale = keyscale
		key_cf.y = vbarloc
		key_dga.y = vbarloc
		key_eb.y = vbarloc
		key_black.scale = keyscale
		key_black.y = vbarloc+256*keyscale
		key_cf_pressed.scale = keyscale
		key_dga_pressed.scale = keyscale
		key_eb_pressed.scale = keyscale
		key_cf_pressed.y = vbarloc
		key_dga_pressed.y = vbarloc
		key_eb_pressed.y = vbarloc
		key_black_pressed.scale = keyscale
		key_black_pressed.y = vbarloc+256*keyscale
		while index*128*keyscale<width:
			key_cf.x = index*128*keyscale; index+=1
			key_cf.draw()
			key_dga.x = index*128*keyscale; index+=1
			key_dga.draw()
			key_eb.x = index*128*keyscale; index+=1
			key_eb.draw()
			key_cf.x = index*128*keyscale; index+=1
			key_cf.draw()
			key_dga.x = index*128*keyscale; index+=1
			key_dga.draw()
			key_dga.x = index*128*keyscale; index+=1
			key_dga.draw()
			key_eb.x = index*128*keyscale; index+=1
			key_eb.draw()
		index = 0
		while index*128*keyscale<width:
			key_black.x = (index*128+96)*keyscale
			key_black.draw()
			index+=1
			key_black.x = (index*128+96)*keyscale
			key_black.draw()
			index+=1
			index+=1
			key_black.x = (index*128+96)*keyscale
			key_black.draw()
			index+=1
			key_black.x = (index*128+96)*keyscale
			key_black.draw()
			index+=1
			key_black.x = (index*128+96)*keyscale
			key_black.draw()
			index+=1
			index+=1
		for i in KEYPRESS_MASK:
			if i%12 in (1,3,6,8,10):
				x = (int((i-41)*7/12.0+3)*128+96)*keyscale
				key_black_pressed.x=x
				key_black_pressed.draw()
			else:
				x = int((i-41)*7/12.0+3)*128*keyscale
				if i%12 in (0,5):
					key_cf_pressed.x = x
					key_cf_pressed.draw()
				elif i%12 in (2,7,9):
					key_dga_pressed.x = x
					key_dga_pressed.draw()
				else:
					key_eb_pressed.x = x
					key_eb_pressed.draw()
	#------  Control Bar ----------#
	barimg.blit(0,vbarloc-barimg.height,width=width,height=barimg.height)
	controlheight = vbarloc-(barimg.height/2+play_img.height/2)
	controlsx = width/2
	if recording:
		recording_img.blit(max(controlsx-100,width/3),vbarloc-(barimg.height/2+record_img.height/2))
	else:
		record_img.blit(max(controlsx-100,width/3),vbarloc-(barimg.height/2+record_img.height/2))
	rw_beg_img.blit(controlsx,controlheight)
	controlsx+=rw_beg_img.width
	rw_img.blit(controlsx,controlheight)
	controlsx+=rw_img.width
	if playing:
		pause_img.blit(controlsx,controlheight)
		controlsx+=pause_img.width
	else:
		play_img.blit(controlsx,controlheight)
		controlsx+=play_img.width
	ff_img.blit(controlsx,controlheight)
	controlsx+=ff_img.width
	ff_end_img.blit(controlsx,controlheight)
	#------------------------------#
	
	#---------- Tracks ------------#
	rgb184184184.blit(0,0,width=width,height=vbarloc-barimg.height)
	for i in xrange(len(TRACKS)):
		draw_track(TRACKS[i],i,width)
	rgb084084084.blit(150,0,height=vbarloc-barimg.height)
	rgb255000000.blit(151+(INDEX-SCROLL)*VIEW_SCALE,0,height=vbarloc-barimg.height)
	#------------------------------#
	
	
@window.event
def on_mouse_press(x, y, button, modifiers):
	width, height = window.get_size()
	# Put other hit tests here first.
	ch = vbarloc-(barimg.height/2+play_img.height/2)
	cx = width/2
	if y>vbarloc and PLAYMODE=='keyboard':
		keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.125)
		noteval = int(x/(key_cf_img.width*keyscale)*12/7.0)+36
		if y<vbarloc+256*keyscale:
			if noteval%12 in (1,3,6,8,10):
				noteval-=int(int((x+31)/(key_cf_img.width*keyscale)*12/7.0)+36==noteval)*2-1
		if not noteval in KEYPRESS_MASK:
			KEYPRESS_MASK.append(noteval)
		player = pyglet.media.ManagedSoundPlayer()
		player.queue(c)
		player.pitch = note(noteval)
		player.volume = 127
		player.play()
		return None
	if max(cx-100,width/3)<x<max(cx-100,width/3)+record_img.width and vbarloc-(barimg.height/2+record_img.height/2)<y<vbarloc-(barimg.height/2)+record_img.height/2:
		global playing, recording
		if recording:
			recording = False
		else:
			if not playing:
				playing = True
				#pyglet.clock.schedule_interval(play, 1/PLAYBACK_SPEED)
			recording = True
			ACTIVETRACK.add(subtrack())
		return None
	if cx<x<cx+rw_beg_img.width and ch<y<ch+rw_beg_img.height:
		global INDEX
		INDEX = 0
		return None
	cx+=rw_beg_img.width
	if cx<x<cx+rw_img.width and ch<y<ch+rw_img.height:
		global INDEX
		INDEX = (INDEX-1)
		INDEX = INDEX-INDEX%STEP
		return None
	cx+=rw_img.width
	if playing:
		if cx<x<cx+pause_img.width and ch<y<ch+pause_img.height:
			global playing
			playing = False
			recording = False
			#pyglet.clock.unschedule(play)
			return None
		cx+=pause_img.width
	else:
		if cx<x<cx+play_img.width and ch<y<ch+play_img.height:
			global playing
			playing = True
			#pyglet.clock.schedule_interval(play, 1/PLAYBACK_SPEED)
			return None
		cx+=play_img.width
	if cx<x<cx+ff_img.width and ch<y<ch+ff_img.height:
		global INDEX
		INDEX = INDEX-INDEX%STEP+STEP
		return None
	cx+=ff_img.width
	if cx<x<cx+ff_end_img.width and ch<y<ch+ff_end_img.height:
		fast_forward_end()
		return None
	if vbarloc-barimg.height<y<vbarloc:
		global dragging_vbar
		dragging_vbar = True
	else:
		print "no"

@window.event
def on_mouse_drag(x, y, dx, dy, buttons, modifiers):
	width, height = window.get_size()
	if dragging_vbar:
		global vbarloc
		vbarloc=max(min(vbarloc+dy,height),barimg.height)
	elif y>vbarloc and PLAYMODE=='keyboard':
		global KEYPRESS_MASK
		keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.125)
		noteval = int(x/(key_cf_img.width*keyscale)*12/7.0)+36
		if y<vbarloc+256*keyscale:
			if noteval%12 in (1,3,6,8,10):
				noteval-=int(int((x+31)/(key_cf_img.width*keyscale)*12/7.0)+36==noteval)*2-1
		if not noteval in KEYPRESS_MASK:
			KEYPRESS_MASK = []
			KEYPRESS_MASK.append(noteval)
			play_note(noteval,127)
		return None
	
@window.event
def on_mouse_release(x, y, button, modifiers):
	global dragging_vbar
	dragging_vbar = False
	global KEYPRESS_MASK
	KEYPRESS_MASK = []
	
@window.event
def on_key_press(symbol, modifiers):
	if symbol in KEYMAP:
		noteval = KEYMAP[symbol]
		if not noteval in KEYPRESS_MASK:
			KEYPRESS_MASK.append(noteval)
		play_note(noteval,127)
	
@window.event
def on_key_release(symbol,modifiers):
	if symbol in KEYMAP:
		if KEYMAP[symbol] in KEYPRESS_MASK:
			del KEYPRESS_MASK[KEYPRESS_MASK.index(KEYMAP[symbol])]
	
@window.event
def on_close():
	inp.stop = True
	
pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA, pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)

clock.set_fps_limit(128)

#clock.schedule_interval_soft(draw,0.0625)

c = pyglet.resource.media('c.wav', streaming=False)

def note(val):
	val-=36
	if 0<=val<12:
		return NOTE_RATIOS[val]
	else:
		return (2**(int(val)/12))*NOTE_RATIOS[val%12]

class silence(object):
	def __getitem__(self,item):
		return None
	def __len__(self):
		return self.n
	def __init__(self,n):
		self.n = n
	def play(self,index):
		return index==self.n-1
	def is_silence(self):
		return True
		
class track(object):
	def __init__(self):
		self.tracks=[]
		self.trackslen=[0]
		self.tracksindex=0
		self.playing=True
		self.label = pyglet.text.Label('Piano',font_name='Times New Roman',font_size=10)
		self.labelshadow = pyglet.text.Label('Piano', font_name='Times New Roman', font_size=10, color=(0,0,0,255))
		self.int_img = tsa_img
		self.cimg = tbg_s_img
		self.leftimg = tbg_s_l_img
		self.rightimg = tbg_s_r_img
	def img(self):
		return self.int_img if self==ACTIVETRACK else tg_img
	def add(self,track):
		self.tracks.append(track)
		self.trackslen.append(len(track))
	def play(self,INDEX):
		b = 0
		for i in self.tracks:
			if INDEX-b<len(i):
				return i.play(INDEX-b)
			else:
				b+=len(i)
	def set_text(self,text):
		self.label.text = str(text)
		self.labelshadow.text = str(text)

		
class subtrack(object):
	def __len__(self):
		return len(self.data)
	def __init__(self):
		self.data = []
	def play(self,index):
		if self.data[index]:
			for i in self.data[index]:
				player = pyglet.media.ManagedSoundPlayer()
				player.queue(c)
				player.pitch = note(i)
				player.play()
		return (index==len(self.data)-1)
	def is_silence(self):
		return False
		
		


def log(message):
	pass


track1 = subtrack()
track1.data = [[0.5,1],None,None,[0.561,1.122],[0.63,1.26],
		[0.667,1.334],None,[0.7071,1.4142],[0.749,1.498],None]
track2 = subtrack()
track2.data = [None,None,[1.498,2,2.52,2.996],None,None,None,[2,2.52,2.996]]

TRACKS = [track(),track()]
ACTIVETRACK = TRACKS[0]
#TRACKS[0].add(track1)
#TRACKS[0].add(silence(5))
#TRACKS[0].add(track1)
#TRACKS[0].add(track1)
#TRACKS[1].add(track2)

TRACKSINDEX = [0]


def play(dt):
	global INDEX
	if playing:
		for i in TRACKS:
			i.play(INDEX)
		if INDEX<46:	#TODO: calculate this value
			INDEX+=1
		else:
			global playing, recording
			playing = False
			recording = False
	if recording:
		ACTIVETRACK.tracks[-1].data.append([])


class input(threading.Thread):
	def run(self):
		self.stop = False
		midiin = rtmidi.RtMidiIn()
		ports = range(midiin.getPortCount())
		if ports:
			for i in ports:
				print "Found port", i, midiin.getPortName(i)
			midiin.openPort(int(raw_input("Enter MIDI port to use: ")))
			while not self.stop:
				m = midiin.getMessage(25) # some timeout in ms
				if m != None:
					#print dir(m)
					if m.isNoteOn():
						if not m.getNoteNumber() in KEYPRESS_MASK:
							KEYPRESS_MASK.append(m.getNoteNumber())
						play_note(m.getNoteNumber(),(m.getVelocity()/128.0)**2)
						#window.dispatch_events()
					elif m.isNoteOff():
						if m.getNoteNumber() in KEYPRESS_MASK:
							del KEYPRESS_MASK[KEYPRESS_MASK.index(m.getNoteNumber())]
					pyglet.clock.schedule_once(draw,0.01)
			
			
inp = input()

pyglet.clock.schedule_interval(play, 1/PLAYBACK_SPEED)

inp.start()

pyglet.app.run()
