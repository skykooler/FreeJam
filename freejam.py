#!/usr/bin/env python
# -*- coding:utf-8 -*-

import os

import pyglet

from pyglet import clock

import threading

import math

import MIDI

pyglet.resource.path = ['instruments/keyboards/piano1']
pyglet.resource.reindex()
NOTE_RATIOS = [1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887]
INDEX = 0		# Position in project

def draw (dummy):
	pass

clock.set_fps_limit(128)

clock.schedule_interval_soft(draw,0.0625)

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

'''def sysexcb(bs): print 'sysex: ', map(ord, bs)

m = MIDI.In(sysexcb)

midifile = os.open('/dev/midi1', os.O_RDONLY)

def testfunc(*args):
	print args

m['note-on'] = testfunc'''

def play(dt):
	global INDEX
	print INDEX,"\t",
	for i in TRACKS:
		i.play(INDEX)
	if INDEX<46:	#TODO: calculate this value
		INDEX+=1
		
import MIDI

def sysex(bs): print 'sysex', map(ord, bs)

m = MIDI.In(sysex)

def foo(msg, *args):
	if msg=="note-on":
		print msg, "(", args[0], args[1], args[2], ")"
		if args[2]>1:
			player = pyglet.media.ManagedSoundPlayer()
			player.queue(c)
			player.pitch = note(args[1])
			player.volume = (args[2]/128.0)**2
			player.play()
	else:
		log(msg)

m[...] = foo

#import os

midifile = os.open('/dev/midi1', os.O_RDONLY)


class input(threading.Thread):
	def run(self):
		while 1:
			bs = os.read(midifile, 1)
			if not bs: break
			m.inbytes(bs)
	
inp = input()

pyglet.clock.schedule_interval(play, 1/4.0)

inp.start()

pyglet.app.run()
