#pragma once

#include "ofMain.h"
#include <algorithm>
#include <vector>
#include <map>
#include <math.h>
#include <string>

using namespace std;

extern const float NOTE_RATIOS[12];
extern bool recording;
extern int INDEX;

float note(float val);

class FreeJamApp: public ofBaseApp{

	public:
		void setPlayMode(int mode);
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		
		ofSoundPlayer  beats;
		ofSoundPlayer  synth;
		ofSoundPlayer  vocals;

		ofTrueTypeFont	font;
		float 			synthPosition;

		ofImage stageimg;
		ofImage barimg;
		
		ofImage key_cf;
		ofImage key_dga;
		ofImage key_eb;
		ofImage key_black;
		ofImage key_cf_pressed;
		ofImage key_dga_pressed;
		ofImage key_eb_pressed;
		ofImage key_black_pressed;

		int				playMode;
		float			keyscalex;
		float			keyscaley;
		float			keyscalexb;
		float			keyscaleyb;
		float			vbarloc;
		bool			dragging_vbar;
		float			vbar_offset_y;

		// vector<int> KEYPRESS_MASK;
		map<int, bool> KEYPRESS_MASK;
		
};

class NotePlayer {
	public:
		NotePlayer (bool cont);
		void play_note(int noteval, float volume);
		void stop_note(int noteval);
		bool continuous;
		bool fadein;
	private:
		vector<ofSoundPlayer *> notes;
};

class Subtrack {
	public:
		int size();
		bool play(int index, NotePlayer * player);
		vector<vector<int> > data; // Inside one is 176 ints long
};

class Track {
	void init();
	void add(Subtrack subtrack);
	bool play(int INDEX);
	void set_text(string text);
	void play_note(int noteval, float volume);
	void stop_note(int noteval);

	vector<Subtrack> tracks;
	vector<float> trackslen;
	int tracksindex;
	bool playing;
	string label;
	Subtrack * currenttrack;
	NotePlayer * player;
};

enum {
	STAGE,
	KEYBOARD
};