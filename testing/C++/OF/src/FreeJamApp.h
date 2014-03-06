#pragma once

#include "ofMain.h"
#include <algorithm>
#include <vector>
#include <map>
#include <math.h>
#include <string>

using namespace std;


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
public:
	Track ();
	void add(Subtrack subtrack);
	bool play(int INDEX);
	void set_text(string text);
	void play_note(int noteval, float volume);
	void stop_note(int noteval);
	// ofImage * img();

	vector<Subtrack> tracks;
	vector<float> trackslen;
	int tracksindex;
	bool playing;
	string label;
	Subtrack * currenttrack;
	NotePlayer * player;
	ofImage int_img;
};

// extern 
extern Track * ACTIVETRACK;
extern const float NOTE_RATIOS[12];
extern bool recording;
extern bool playing;
extern int INDEX;
extern float SCROLL;
extern float VIEW_SCALE;
extern int STEP;
extern float SIDE_WIDTH;

float note(float val);

class FreeJamApp: public ofBaseApp{

	public:
		void setPlayMode(int mode);
		void setup();
		void update();
		void draw();
		void draw_track(Track * track, int num, float width);

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
		ofImage record_img;
		ofImage recording_img;
		ofImage rw_beg_img;
		ofImage rw_img;
		ofImage play_img;
		ofImage pause_img;
		ofImage ff_img;
		ofImage ff_end_img;
		ofImage slider_img;

		ofImage trackbg_img;
		ofImage tsa_img;
		ofImage tra_img;
		ofImage tla_img;
		ofImage toa_img;
		ofImage tg_img;

		ofImage tbg_s_img;
		ofImage tbg_s_l_img;
		ofImage tbg_s_r_img;
		ofImage tbg_r_img;
		ofImage tbg_r_l_img;
		ofImage tbg_r_r_img;
		ofImage tbg_l_img;
		ofImage tbg_l_l_img;
		ofImage tbg_l_r_img;
		ofImage tbg_o_img;
		ofImage tbg_o_l_img;
		ofImage tbg_o_r_img;

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

		std::vector<Track *> TRACKS;
		
};

enum {
	STAGE,
	KEYBOARD
};