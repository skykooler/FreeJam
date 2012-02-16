#!/usr/bin/env python
# -*- coding:utf-8 -*-

import pyglet

stageimg = pyglet.image.load('resources/image-textures/floor.png')
barimg = pyglet.image.load('resources/image-textures/bar.png')
record_img = pyglet.image.load('resources/image-textures/record_b.png')
rw_beg_img = pyglet.image.load('resources/image-textures/rewind_to_beginning.png')
rw_img = pyglet.image.load('resources/image-textures/rewind.png')
play_img = pyglet.image.load('resources/image-textures/play.png')
ff_img = pyglet.image.load('resources/image-textures/fastforward.png')
ff_end_img = pyglet.image.load('resources/image-textures/forward_to_end.png')

window = pyglet.window.Window(resizable=True)

window.maximize()

#### Global variables ######
vbarloc = 100
dragging_vbar = False
############################

stagesprite = pyglet.sprite.Sprite(stageimg)
#barsprite = pyglet.sprite.Sprite(barimg)

@window.event
def on_draw():
    window.clear()
    width, height = window.get_size()
    stagesprite.scale = max(width/(stageimg.width*1.0), height/(stageimg.height*1.0))
    stagesprite.y = vbarloc
    stagesprite.draw()
    barimg.blit(0,vbarloc-barimg.height,width=width,height=barimg.height)
    controlheight = vbarloc-(barimg.height/2+play_img.height/2)
    controlsx = width/2
    record_img.blit(max(controlsx-100,width/3),vbarloc-(barimg.height/2+record_img.height/2))
    rw_beg_img.blit(controlsx,controlheight)
    controlsx+=rw_beg_img.width
    rw_img.blit(controlsx,controlheight)
    controlsx+=rw_img.width
    play_img.blit(controlsx,controlheight)
    controlsx+=play_img.width
    ff_img.blit(controlsx,controlheight)
    controlsx+=ff_img.width
    ff_end_img.blit(controlsx,controlheight)
    
@window.event
def on_mouse_press(x, y, button, modifiers):
	# Put other hit tests here first.
	if vbarloc-barimg.height<y<vbarloc:
		global dragging_vbar
		dragging_vbar = True
	else:
		print "no"

@window.event
def on_mouse_drag(x, y, dx, dy, buttons, modifiers):
	if dragging_vbar:
		global vbarloc
		vbarloc+=dy
	
@window.event
def on_mouse_release(x, y, button, modifiers):
	global dragging_vbar
	dragging_vbar = False
    
pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA, pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)
pyglet.app.run()