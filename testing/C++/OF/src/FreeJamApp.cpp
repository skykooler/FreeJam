#include "FreeJamApp.h"
#include <vector>

const float NOTE_RATIOS[12] = {1,1.059,1.122,1.189,1.26,1.334,1.4142,1.498,1.587,1.682,1.7818,1.887};
bool recording = false;
bool playing = false;
int INDEX = 0;
float SCROLL = 0;
float VIEW_SCALE = 1.0f;
int STEP = 64; // maximum resolution is 64th notes
float SIDE_WIDTH = 150;
Track * ACTIVETRACK;


NotePlayer::NotePlayer(bool cont) {
	continuous = cont;
	for (int i=0; i<88; i++) {
		ofSoundPlayer player;
		notes.push_back(&player);
		// self.notes.append(audio.open_file('instruments/strings/basic strings/c.wav'))
		player.setSpeed(note(i));
		player.setLoop(continuous);
	}
}
void NotePlayer::play_note(int noteval, float volume) {
	(*notes.at(noteval)).stop();
	(*notes.at(noteval)).play();
	// if (fadein) {
	// 	fadein_note(None,self.notes,noteval,self.fadein,volume)
	// } else {
		(*notes.at(noteval)).setVolume(volume);
	// }
}
void NotePlayer::stop_note(int noteval){
	// if (cutoff) {
	// 	cutoff_note(None,self.notes,noteval,self.cutoff)
	// }
}

Subtrack::Subtrack () {
	vector<vector<int> > data;
	printf("Initial size: %i\n", data.size());
}
int Subtrack::size() {
	return data.size();
}
bool Subtrack::play(int index, NotePlayer * player) {
	for (int i=0; i<88; i++) {
		if ( data.at(index)[i] ) {
			float volume = data.at(index)[i+88];
			(*player).play_note(i,volume);
		}
	}
	return (index==data.size()-1);
}
void Subtrack::add_data() {
	vector<int> frame (176,0);
	data.push_back(frame);
}

Track::Track() {
	trackslen.push_back(0); // List of lengths of all subtracks
	tracksindex=0;

	// playing=true;

	// self.definition = xml_to_dict(etree.parse('instruments/strings/basic strings/instrument.xml').getroot())['instrument']
	// try:
	// 	self.player = NotePlayer(True if self.definition['continuous'].lower()=='yes' else False)
	// except:
		player = new NotePlayer(false);
	// try:
	// 	self.player.cutoff = int(self.definition['cutoff'])
	// except:
	// 	self.player.cutoff = 100
	// try:
	// 	if int(self.definition['fadein']):
	// 		self.player.set_fadein()
	// except:
	// 	pass # fadein is already False
	// try:
	// 	self.name = self.definition['name']
	// except:
	// 	self.name = 'Untitled instrument'
	// try:
	// 	self.player.pitchbend = True if self.definition['pitchbend'].lower()=='yes' else False
	// except:
	// 	self.player.pitchbend = False
	// label = "";
	label = "Track 1";
	// int_img = tsa_img;
	// self.cimg = tbg_s_img
	// self.leftimg = tbg_s_l_img
	// self.rightimg = tbg_s_r_img
}

// ofImage * Track::img() {
// 	return this==ACTIVETRACK ? &int_img : &tg_img;
// }
void Track::add(Subtrack subtrack) {
	tracks.push_back(subtrack);
	trackslen.push_back(subtrack.size());
	currenttrack = tracks.size()-1;
}
bool Track::play(int INDEX) {
	int b = 0;
	// for i in self.tracks:
	for(std::vector<Subtrack>::iterator it = tracks.begin(); it != tracks.end(); ++it) {
		if (INDEX-b<((*it).size())) {
			return (*it).play(INDEX-b,player);
		} else {
			b+=(*it).size();
		}
	}
}
void Track::set_text(string text) {
	label = text;
}
void Track::play_note(int noteval, float volume) {
	if (recording) {
		tracks.at(currenttrack).data.at(INDEX)[noteval] = true;
		tracks.at(currenttrack).data.at(INDEX)[noteval+88] = floor(volume*255);
	}
	(*player).play_note(noteval,volume);
}
void Track::stop_note(int noteval){
	if (recording) {
		tracks.at(currenttrack).data.at(INDEX)[noteval] = 255;
	}
}

float note(float val){
	val-=36;
	if (0<=val and val<12) {
		return NOTE_RATIOS[(int)(floor(val))];
	} else {
		return (pow(2,(floor(val/12)))) * NOTE_RATIOS[(int)(floor(val))%12];
	}
}

//--------------------------------------------------------------
void FreeJamApp::setup(){
	synth.loadSound("sounds/synth.wav");
	beats.loadSound("sounds/1085.mp3");
	//vocals.loadSound("sounds/Violet.mp3");
	vocals.loadSound("sounds/c.wav");
	synth.setVolume(0.75f);
	beats.setVolume(0.75f);
	vocals.setVolume(0.5f);
	// font.loadFont("Sudbury_Basin_3D.ttf", 32);
	font.loadFont("FreeSerif.ttf", 10);
	beats.setMultiPlay(false);
	vocals.setMultiPlay(true);
	playMode = KEYBOARD;
	// playMode = STAGE;
	vbarloc = 256;
	// vbarloc = 512;
	dragging_vbar = false;
	vbar_offset_y = 0;

	//----------------------------------------------------------
	stageimg.loadImage("resources/image-textures/floor.png");
	barimg.loadImage("resources/image-textures/bar.png");
	record_img.loadImage("resources/image-textures/record_b.png");
	recording_img.loadImage("resources/image-textures/record_active.png");
	rw_beg_img.loadImage("resources/image-textures/rewind_to_beginning.png");
	rw_img.loadImage("resources/image-textures/rewind.png");
	play_img.loadImage("resources/image-textures/play.png");
	pause_img.loadImage("resources/image-textures/pause.png");
	ff_img.loadImage("resources/image-textures/fastforward.png");
	ff_end_img.loadImage("resources/image-textures/forward_to_end.png");
	slider_img.loadImage("resources/image-textures/slider.png");

	trackbg_img.loadImage("resources/image-textures/trackbg.png");
	tsa_img.loadImage("resources/image-textures/track_software_active.png");
	tra_img.loadImage("resources/image-textures/track_real_active.png");
	tla_img.loadImage("resources/image-textures/track_loop_active.png");
	toa_img.loadImage("resources/image-textures/track_other_active.png");
	tg_img.loadImage("resources/image-textures/track_generic.png");

	tbg_s_img.loadImage("resources/image-textures/track_software_bg.png");
	tbg_s_l_img.loadImage("resources/image-textures/track_software_bg_left.png");
	tbg_s_r_img.loadImage("resources/image-textures/track_software_bg_right.png");
	tbg_r_img.loadImage("resources/image-textures/track_real_bg.png");
	tbg_r_l_img.loadImage("resources/image-textures/track_real_bg_left.png");
	tbg_r_r_img.loadImage("resources/image-textures/track_real_bg_right.png");
	tbg_l_img.loadImage("resources/image-textures/track_loop_bg.png");
	tbg_l_l_img.loadImage("resources/image-textures/track_loop_bg_left.png");
	tbg_l_r_img.loadImage("resources/image-textures/track_loop_bg_right.png");
	tbg_o_img.loadImage("resources/image-textures/track_other_bg.png");
	tbg_o_l_img.loadImage("resources/image-textures/track_other_bg_left.png");
	tbg_o_r_img.loadImage("resources/image-textures/track_other_bg_right.png");

	key_cf.loadImage("resources/image-textures/key_cf.png");
	key_dga.loadImage("resources/image-textures/key_dga.png");
	key_eb.loadImage("resources/image-textures/key_eb.png");
	key_black.loadImage("resources/image-textures/key_black.png");
	key_cf_pressed.loadImage("resources/image-textures/key_cf_pressed.png");
	key_dga_pressed.loadImage("resources/image-textures/key_dga_pressed.png");
	key_eb_pressed.loadImage("resources/image-textures/key_eb_pressed.png");
	key_black_pressed.loadImage("resources/image-textures/key_black_pressed.png");


	Track * t = new Track();
	TRACKS.push_back(t);
	ACTIVETRACK = TRACKS[0];
	Subtrack subtrack = Subtrack();
	(*ACTIVETRACK).add(subtrack);
		ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).add_data();
	printf("bla %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).size());
	printf("%i\n",(*ACTIVETRACK).tracks.size());
	printf("ACTIVETRACK POINTER: %i\n", ACTIVETRACK);
	// printf("CURRENTTRACK POINTER: %i\n", (*ACTIVETRACK).currenttrack);
}

//--------------------------------------------------------------
void FreeJamApp::update(){
	printf("1Update data size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data.size());

	ofBackground(255,255,255);
	printf("2Update data size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data.size());

	// update the sound playing system:
	ofSoundUpdate();
	printf("3Update data size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data.size());


	// TODO: proper timing
	play();

}

//--------------------------------------------------------------
void FreeJamApp::draw(){
	printf("4Update data size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data.size());
	keyscaley = max((ofGetHeight()-vbarloc),128.0f);
	keyscalex = keyscaley*key_cf.width/key_cf.height;
	keyscalexb = 0.5*keyscalex;
	keyscaleyb = 0.5*keyscaley;
	float y = min(0.0f,ofGetHeight()-(vbarloc+128));

	//----------------------------------------------------------
	// draw the background colors:
	float widthDiv = ofGetWidth() / 3.0f;
	ofSetHexColor(0xeeeeee);
	ofRect(0,0,widthDiv,ofGetHeight());
	ofSetHexColor(0xffffff);
	ofRect(widthDiv,0,widthDiv,ofGetHeight());
	ofSetHexColor(0xdddddd);
	ofRect(widthDiv*2,0,widthDiv,ofGetHeight());


	//---------------------------------- synth:
	if (synth.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("synth !!", 50,50);

	ofSetHexColor(0x000000);
	string tempStr = "click to play\npct done: "+ofToString(synth.getPosition())+"\nspeed: " + ofToString(synth.getSpeed()) + "\npan: " + ofToString(synth.getPan()) ;
	ofDrawBitmapString(tempStr, 50,ofGetHeight()-50);



	//---------------------------------- beats:
	if (beats.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("beats !!", widthDiv+50,50);

	ofSetHexColor(0x000000);
	tempStr = "click and drag\npct done: "+ofToString(beats.getPosition())+"\nspeed: " +ofToString(beats.getSpeed());
	ofDrawBitmapString(tempStr, widthDiv+50,ofGetHeight()-50);

	//---------------------------------- vocals:
	if (vocals.getIsPlaying()) ofSetHexColor(0xFF0000);
	else ofSetHexColor(0x000000);
	font.drawString("vocals !!", widthDiv*2+50,50);

	ofSetHexColor(0x000000);
	tempStr = "click to play (multiplay)\npct done: "+ofToString(vocals.getPosition())+"\nspeed: " + ofToString(vocals.getSpeed());
	ofDrawBitmapString(tempStr, widthDiv*2+50,ofGetHeight()-50);

	//-------------------------------------------
	ofSetColor(255);
	//ofEnableAlphaBlending();
	if (playMode==KEYBOARD) {
		//int index = 0;
		// while (index*keyscalex<ofGetWidth()) {
		for (int index=0; index<ofGetWidth()/(keyscalex);) {
			key_cf.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_eb.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_cf.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_dga.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
			key_eb.draw(index*keyscalex,y,keyscalex,keyscaley);
			index+=1;
		}
		// int index = 0;
		// while (index*keyscalexb<ofGetWidth()) {
		for (int index=0; index<ofGetWidth()/(keyscalex);) {
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			key_black.draw((index*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
			index+=1;
			index+=1;
		}
		map<int,bool>::iterator iter;
		for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
			if (iter->second) {
				switch (iter->first%12) {
					case 1:
					case 3:
					case 6:
					case 8:
					case 10:
						key_black_pressed.draw((floor((iter->first-41)*7/12.0+3)*128+96)*keyscalex/128,y,keyscalexb,keyscaleyb);
						break;
					case 0:
					case 5:
						key_cf_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
					case 2:
					case 7:
					case 9:
						key_dga_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
					default:
						key_eb_pressed.draw(floor((iter->first-41)*7/12.0+3)*keyscalex,y,keyscalex,keyscaley);
						break;
				}
			}
		}
	} else {
		float stagescale = max(ofGetWidth()/(stageimg.width), ofGetHeight()/(stageimg.height));
		stageimg.draw(0,ofGetHeight()-(vbarloc+stagescale*stageimg.height),stagescale*stageimg.width,stagescale*stageimg.height);
	}
	//---------------------------------------------- Control Bar
	barimg.draw(0,ofGetHeight()-vbarloc,ofGetWidth(),barimg.height);
	float controlheight = ofGetHeight()+barimg.height/2-(vbarloc+play_img.height/2);
	float controlsx = ofGetWidth()/2;
	if (recording) {
		recording_img.draw(max(controlsx-100,(float)ofGetWidth()/3),ofGetHeight()+barimg.height/2-(vbarloc+record_img.height/2));
	} else {
		record_img.draw(max(controlsx-100,(float)ofGetWidth()/3),ofGetHeight()+barimg.height/2-(vbarloc+record_img.height/2));
	}
	rw_beg_img.draw(controlsx,controlheight);

	controlsx+=rw_beg_img.width;
	rw_img.draw(controlsx,controlheight);
	controlsx+=rw_img.width;
	if (playing) {
		pause_img.draw(controlsx,controlheight);
		controlsx+=pause_img.width;
	} else {
		play_img.draw(controlsx,controlheight);
		controlsx+=play_img.width;
	}
	ff_img.draw(controlsx,controlheight);
	controlsx+=ff_img.width;
	ff_end_img.draw(controlsx,controlheight);
	//------------------------------------------

	//----------- Tracks ---------------------//
	// Track background
	ofSetColor(184,184,184);
	ofFill();
	ofRect(0,ofGetHeight()+barimg.height-vbarloc,ofGetWidth(),vbarloc-barimg.height);
	for (int i=0; i<TRACKS.size();i++) {
		draw_track(TRACKS[i],i,ofGetWidth());
	}
	ofSetColor(84,84,84);
	ofLine(150,ofGetHeight()+barimg.height-vbarloc,150,ofGetHeight());
	// Scrub bar
	ofSetColor(255,0,0);
	ofLine(151+(INDEX-SCROLL)*VIEW_SCALE,ofGetHeight(),151+(INDEX-SCROLL)*VIEW_SCALE,ofGetHeight()+barimg.height-vbarloc);
	// Slider background
	ofSetColor(237,237,237);
	ofRect(151,ofGetHeight()-16,ofGetWidth()-150,16);
	ofSetColor(84,84,84);
	// Slider track
	ofLine(155,ofGetHeight()-8,ofGetWidth()-4,ofGetHeight()-8);
	ofSetColor(255,255,255);
	slider_img.draw(min(VIEW_SCALE*64+154,(float)ofGetWidth()-8),ofGetHeight()-13);

	printf("5Update data size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data.size());
}

//--------------------------------------------------------------
void FreeJamApp::keyPressed  (int key){
}

//--------------------------------------------------------------
void FreeJamApp::keyReleased(int key){

}

//--------------------------------------------------------------
void FreeJamApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void FreeJamApp::mouseDragged(int x, int y, int button){
	// continuously control the speed of the beat sample via drag,
	// when in the "beat" region:
	if (dragging_vbar) {
		// vbarloc=ofGetHeight()-max(min(vbar_offset_y+y,(float)ofGetHeight()),(float)barimg.height);
		vbarloc=max(barimg.height,ofGetHeight()-max(y+(int)vbar_offset_y,0));
		printf("Vbarloc: %f\n", vbarloc);
	// } else if {
	} else if (y<(ofGetHeight()-vbarloc) and playMode==KEYBOARD){
		int noteval = floor(x/(keyscalex)*12/7.0)+36;
		if (y>keyscaleyb) {
			if (noteval%12==1 or noteval%12==3 or noteval%12==6 or noteval%12==8 or noteval%12==10){
				noteval-=floor((float)(floor((x+31)/(keyscalex)*12/7.0)+36==noteval))*2-1;
			}
		}
		map<int,bool>::iterator iter = KEYPRESS_MASK.find(noteval);
		if (iter==KEYPRESS_MASK.end() or iter->second==false) {
			KEYPRESS_MASK[noteval] = true;
			// if not noteval in KEYPRESS_MASK:
			// 	KEYPRESS_MASK.append(noteval)

			// ACTIVETRACK.play_note(noteval,1)
			vocals.play();
			vocals.setSpeed(note(noteval));
		}
		for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
			if (iter->first!=noteval) {
				iter->second = false;
			}
		}
		return;
	}
}

//--------------------------------------------------------------
void FreeJamApp::mousePressed(int x, int y, int button){
	/*float widthStep = ofGetWidth() / 3.0f;
	if (x < widthStep){
		synth.play();
		synth.setSpeed( 0.1f + ((float)(ofGetHeight() - y) / (float)ofGetHeight())*10);
		synth.setPan(ofMap(x, 0, widthStep, -1, 1, true));
	} else if (x >= widthStep && x < widthStep*2){
		beats.play();
	} else {
		vocals.play();
		vocals.setSpeed( 0.1f + ((float)(ofGetHeight() - y) / (float)ofGetHeight())*3);
		//map x within the last third of window to -1 to 1 ( for left / right panning )
		vocals.setPan( ofMap(x, widthStep*2, widthStep*3, -1, 1, true) );
	}*/

	float ch = ofGetHeight()+(barimg.height/2-play_img.height/2)-vbarloc;
	float cx = ofGetWidth()/2;

	if (y<(ofGetHeight()-vbarloc) and playMode==KEYBOARD){
		//keyscale = max((height-vbarloc)/(key_cf_img.height*1.0),0.25)
		int noteval = floor(x/(keyscalex)*12/7.0)+36;
		if (y>keyscaleyb) {
			if (noteval%12==1 or noteval%12==3 or noteval%12==6 or noteval%12==8 or noteval%12==10){
				noteval-=floor((float)(floor((x+31)/(keyscalex)*12/7.0)+36==noteval))*2-1;
			}
		}
		KEYPRESS_MASK[noteval] = true;
		// if not noteval in KEYPRESS_MASK:
		// 	KEYPRESS_MASK.append(noteval)

		// ACTIVETRACK.play_note(noteval,1)
		vocals.play();
		vocals.setSpeed(note(noteval));
		return;
	} else if (max(cx-100,(float)ofGetWidth()/3)<x and x<max(cx-100,(float)ofGetWidth()/3)+record_img.width and (float)ofGetHeight()+(barimg.height/2-record_img.height/2)-vbarloc<y and y<(float)ofGetHeight()+(barimg.height/2)+record_img.height/2-vbarloc) {
		if (recording) {
			recording = false;
		} else {
			if (not playing) {
				playing = true;
			}
			recording = true;
		printf("1 ACTIVETRACK POINTER: %i\n", ACTIVETRACK);
		printf("1 CURRENTTRACK POINTER: %i\n", ACTIVETRACK->currenttrack);
			Subtrack subtrack = Subtrack();
			(*ACTIVETRACK).add(subtrack);
		printf("2 ACTIVETRACK POINTER: %i\n", ACTIVETRACK);
		printf("2 CURRENTTRACK POINTER: %i\n", ACTIVETRACK->currenttrack);
		printf("Data pointer: %i\n", &ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).data);
		printf("Currenttrack size: %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).size());
		}
		return;
	} else if (cx<x and x<cx+rw_beg_img.width and ch<y and y<ch+rw_beg_img.height) {
		INDEX = 0;
		return;
	}
	cx+=rw_beg_img.width;
	if (cx<x and x<cx+rw_img.width and ch<y and y<ch+rw_img.height) {
		INDEX = (INDEX-1);
		INDEX = INDEX-INDEX%STEP;
		return;
	}
	cx+=rw_img.width;
	if (playing) {
		if (cx<x and x<cx+pause_img.width and ch<y and y<ch+pause_img.height) {
			playing = false;
			recording = false;
			return;
		}
		cx+=pause_img.width;
	} else {
		if (cx<x and x<cx+play_img.width and ch<y and y<ch+play_img.height) {
			playing = true;
			return;
		}
		cx+=play_img.width;
	}
	if (cx<x and x<cx+ff_img.width and ch<y and y<ch+ff_img.height) {
		INDEX = INDEX-INDEX%STEP+STEP;
		return;
	}
	cx+=ff_img.width;
	if (cx<x and x<cx+ff_end_img.width and ch<y and y<ch+ff_end_img.height) {
		// fast_forward_end();
		return;
	}

	if (ofGetHeight()-vbarloc<y and y<ofGetHeight()+barimg.height-vbarloc) {
		dragging_vbar = true;
		vbar_offset_y = max(barimg.height,ofGetHeight()-max(y,0))-vbarloc;
	}
}

//--------------------------------------------------------------
void FreeJamApp::mouseReleased(int x, int y, int button){
	dragging_vbar = false;
	map<int,bool>::iterator iter;
	for (iter = KEYPRESS_MASK.begin(); iter != KEYPRESS_MASK.end(); ++iter) {
		// ACTIVETRACK.stop_note(iter->first);
		iter->second = false;
	}
	vocals.stop();
	// KEYPRESS_MASK = []
	// dragging_scale = false;

}

//--------------------------------------------------------------
void FreeJamApp::windowResized(int w, int h){
}

//--------------------------------------------------------------
void FreeJamApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void FreeJamApp::dragEvent(ofDragInfo dragInfo){

}

void FreeJamApp::draw_track(Track * track,int num,float width) {
	float TOP_HEIGHT=(ofGetHeight()+barimg.height)-vbarloc+((num)*(tsa_img).height);

	ofSetColor(255,255,255);
	trackbg_img.draw(0,TOP_HEIGHT,ofGetWidth(),trackbg_img.height);

	for (int i=0; i<ofGetWidth(); i++) {
		if (i%STEP==0) {
			ofSetColor(84,84,84);
			ofLine((i-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT,(i-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT+tsa_img.height-3);
		}
	}
	ofSetColor(255,255,255);
	int track_index = 0;
	int time_index = 0;
	for ( int i=0; i<(*track).tracks.size(); i++) {
	// 	if not i.is_silence():
	// 		pass
	// 		#print time_index
			// track.leftimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT+4);
			tbg_s_l_img.draw((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT);
	// 		track.cimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+5,TOP_HEIGHT+4,width=len(i)*VIEW_SCALE-8)
			tbg_s_img.draw((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+5,TOP_HEIGHT,max((*track).tracks[i].size()*VIEW_SCALE-8,0.0f),tbg_s_img.height);
	// 		track.rightimg.blit((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+len(i)*VIEW_SCALE-4,TOP_HEIGHT+4)
			tbg_s_r_img.draw((time_index-SCROLL)*VIEW_SCALE+SIDE_WIDTH+(*track).tracks[i].size()*VIEW_SCALE-4,TOP_HEIGHT);
	// 		for j in xrange(len(i.data)):
	// 			if i.data[j]:
	// 				#for [k,l,m] in i.data[j]:
	// 				#	rgb084084084.blit((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT+8+k%28,width=m*VIEW_SCALE)
	// 				for r in xrange(len(i.data[j])):
	// 					#1 indicates note-on, 255 indicates note-off
	// 					if i.data[j][r]==1 :
	// 						length = 0;
	// 						for k in xrange(j, len(i.data)):
	// 							if i.data[k][r]==255:
	// 								break
	// 							length+=1
	// 						rgb084084084.blit((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT+8+r%28,width=length*VIEW_SCALE)
	//
	//		#draw_rect((time_index+j-SCROLL)*VIEW_SCALE+SIDE_WIDTH+1,TOP_HEIGHT+8+r%28,length*VIEW_SCALE,1,0.33,0.33,0.33);
	// 	time_index+=len(i)
	// 	track_index+=1
	}
	// TODO: swap out tsa_img depending on track type
	tsa_img.draw(0,TOP_HEIGHT,SIDE_WIDTH,tsa_img.height);
	ofSetColor(0,0,0);
	font.drawString((*track).label, 15, TOP_HEIGHT+tsa_img.height/2 + font.getLineHeight()/2-1);
	ofSetColor(255,255,255);
	font.drawString((*track).label, 16, TOP_HEIGHT+tsa_img.height/2 + font.getLineHeight()/2);
}

void FreeJamApp::play() {
	printf("A %i\n", ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).size());
	if (playing) {
		for (int i=0; i<TRACKS.size(); i++) {
			(*TRACKS[i]).play(INDEX);
		}
		if (INDEX<256) {	//TODO: calculate this value
			INDEX+=1;
		} else {
			playing = false;
			recording = false;
		}
	}
	if (recording) {
		ACTIVETRACK->tracks.at(ACTIVETRACK->currenttrack).add_data();
		ofExit();
		printf("IOJJOJJOIOJIJOOJOIIJJOJI\n");
	}
}
