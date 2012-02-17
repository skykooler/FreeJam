#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os

import pyglet

from pyglet import clock

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

trackbg_img = pyglet.image.load('resources/image-textures/trackbg.png')

# Colors for fills. OpenGL is overdrawing my stuff if I use primitives.
rgb184184184= pyglet.image.load('resources/image-textures/colors/184-184-184.png')
rgb084084084= pyglet.image.load('resources/image-textures/colors/084-084-084.png')
rgb255000000= pyglet.image.load('resources/image-textures/colors/255-000-000.png')

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
global playing
global recording
vbarloc = 100
dragging_vbar = False
STEP = 64 # maximum res is 64th-notes
VIEW_SCALE = 1.0
SCROLL = 0
INDEX = 0
PLAYBACK_SPEED = 16.0
NOTE_RATIOS = [1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887]
playing = False
recording = False
############################

stagesprite = pyglet.sprite.Sprite(stageimg)

pyglet.resource.path = ['instruments/keyboards/piano1']
pyglet.resource.reindex()

#This label should be part of the track object.
tracklabel1 = pyglet.text.Label('Piano',font_name='Times New Roman',font_size=10)
tracklabel1_shadow = pyglet.text.Label('Piano',
										font_name='Times New Roman',
										font_size=10,
										color=(0,0,0,255))
def draw_track(num,width):
	trackbg_img.blit(0,(vbarloc-barimg.height)-((num+1)*tsa_img.height),width=width)
	for i in xrange(*VIEW_EXTENTS):
		if i%STEP==0:
			rgb084084084.blit((i-SCROLL)*VIEW_SCALE+151,(vbarloc-barimg.height)-((num+1)*tsa_img.height)+4,height=tsa_img.height-4)
	tsa_img.blit(0,(vbarloc-barimg.height)-((num+1)*tsa_img.height),width=150,height=tsa_img.height)
	tracklabel1_shadow.x = 15
	tracklabel1_shadow.y = (vbarloc-barimg.height)-(num*tsa_img.height+tracklabel1.font_size)-2
	tracklabel1.x = 16
	tracklabel1.y = (vbarloc-barimg.height)-(num*tsa_img.height+tracklabel1.font_size)-3
	tracklabel1_shadow.draw()
	tracklabel1.draw()

@window.event
def on_draw():
    window.clear()
    width, height = window.get_size()
    global VIEW_EXTENTS
    VIEW_EXTENTS = (0,width)	# for now, at least
    stagesprite.scale = max(width/(stageimg.width*1.0), height/(stageimg.height*1.0))
    stagesprite.y = vbarloc
    stagesprite.draw()
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
    draw_track(0,width)
    rgb084084084.blit(150,0,height=vbarloc-barimg.height)
    rgb255000000.blit(151+(INDEX-SCROLL)*VIEW_SCALE,0,height=vbarloc-barimg.height)
    #------------------------------#
    
    
@window.event
def on_mouse_press(x, y, button, modifiers):
    width, height = window.get_size()
    # Put other hit tests here first.
    ch = vbarloc-(barimg.height/2+play_img.height/2)
    cx = width/2
    if max(cx-100,width/3)<x<max(cx-100,width/3)+record_img.width and vbarloc-(barimg.height/2+record_img.height/2)<y<vbarloc-(barimg.height/2)+record_img.height/2:
        global playing, recording
        if recording:
            recording = False
        else:
            if not playing:
                playing = True
                pyglet.clock.schedule_interval(play, 1/PLAYBACK_SPEED)
            recording = True
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
            pyglet.clock.unschedule(play)
            return None
        cx+=pause_img.width
    else:
        if cx<x<cx+play_img.width and ch<y<ch+play_img.height:
            global playing
            playing = True
            pyglet.clock.schedule_interval(play, 1/PLAYBACK_SPEED)
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
	if dragging_vbar:
		global vbarloc
		width, height = window.get_size()
		vbarloc=max(min(vbarloc+dy,height),barimg.height)
	
@window.event
def on_mouse_release(x, y, button, modifiers):
	global dragging_vbar
	dragging_vbar = False
    
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
		
class track(object):
	def __init__(self):
		self.tracks=[]
		self.trackslen=[0]
		self.tracksindex=0
		self.playing=True
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
				player.pitch = i
				player.play()
		return (index==len(self.data)-1)
		
		


def log(message):
	pass


track1 = subtrack()
track1.data = [[0.5,1],None,None,[0.561,1.122],[0.63,1.26],
		[0.667,1.334],None,[0.7071,1.4142],[0.749,1.498],None]
track2 = subtrack()
track2.data = [None,None,[1.498,2,2.52,2.996],None,None,None,[2,2.52,2.996]]

TRACKS = [track(),track()]
TRACKS[0].add(track1)
TRACKS[0].add(silence(5))
TRACKS[0].add(track1)
TRACKS[0].add(track1)
TRACKS[1].add(track2)

TRACKSINDEX = [0]


def play(dt):
	global INDEX
	for i in TRACKS:
		i.play(INDEX)
	if INDEX<46:	#TODO: calculate this value
		INDEX+=1


class input(threading.Thread):
	def run(self):
		midiin = rtmidi.RtMidiIn()
		ports = range(midiin.getPortCount())
		if ports:
			for i in ports:
				print i, midiin.getPortName(i)
			midiin.openPort(0)
			while True:
				m = midiin.getMessage(250) # some timeout in ms
				if m != None:
					#print dir(m)
					if m.isNoteOn():
						player = pyglet.media.ManagedSoundPlayer()
						player.queue(c)
						player.pitch = note(m.getNoteNumber())
						player.volume = (m.getVelocity()/128.0)**2
						player.play()
					elif m.isNoteOff():
						pass
			
			
inp = input()

#pyglet.clock.schedule_interval(play, 1/4.0)

inp.start()

pyglet.app.run()
