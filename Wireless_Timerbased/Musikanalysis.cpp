/*
 * Musikanalysis.cpp
 *
 *  Created on: 30.05.2015
 *      Author: christoph
 */

#include "Musikanalysis.h"
#include <iostream>
#include "BeatByNiko.h"
#include "BeatRec.h"
#include "Spectrum.h"
#include "BeatStat.h"
using namespace Marsyas;
Musikanalysis::Musikanalysis(BehaviourGenerator* generator) {
	//register custom MarSystems
	std::cout << "Setup" << std::endl;
	MarSystem* beat = new BeatRec("Beat");
	MarSystem* beat2 = new BeatByNiko("BeatNiko");
	MarSystem* spectrum = new Spectrum("spec");
	MarSystem* beatstat = new BeatStat("beatstat");
	mng.registerPrototype("Beat", beat);
	mng.registerPrototype("BeatNiko", beat2);
	mng.registerPrototype("Spectrum", spectrum);
	mng.registerPrototype("BeatStat", beatstat);
	std::cout << "Point1" << std::endl;



	//create top level series and set datarate
	topnet = mng.create("Series", "topnet");
	topnet->updControl("mrs_real/israte", 48000.0);
	fanout = mng.create("Fanout", "fanout");
	analysis1 = mng.create("Series", "analysis1");
	analysis2 = mng.create("Series", "analysis2");

	//create audio input
	topnet->addMarSystem(mng.create("AudioSourceBlocking", "audioIn"));

	//topnet->updControl("AudioSource/audioIn/mrs_bool/realtime", false);

	// create onepole

	//topnet->addMarSystem(mng.create("OnePole", "pfilter"));
	//topnet->updControl("OnePole/pfilter/mrs_real/alpha",0.9);

	//create highpass filter to cancel distortion in bass
	topnet->addMarSystem(mng.create("Biquad", "undistort"));
	topnet->updControl("Biquad/undistort/mrs_string/type","highpass");
	topnet->updControl("Biquad/undistort/mrs_real/resonance",5.0);
	topnet->updControl("Biquad/undistort/mrs_real/frequency", 20.0);



	//CONFIGURE FANOUT SUBSYSTEMS

	//configure analysis1

	//add Windowing
	analysis1->addMarSystem(mng.create("Windowing", "an1win"));
	analysis1->updControl("Windowing/an1win/mrs_string/type", "Hanning");
	analysis1->updControl("Windowing/an1win/mrs_natural/size", 2048);

	//add FFT-Spectrum
	 analysis1->addMarSystem(mng.create("Spectrum", "an1Spec"));

	 //add to decibel conversion
	 analysis1->addMarSystem(mng.create("PowerSpectrum", "an1PSpec"));
	 analysis1->updControl("PowerSpectrum/an1PSpec/mrs_string/spectrumType", "decibels");

	 //add BeatbyNiko
	 analysis1->addMarSystem(mng.create("BeatNiko", "an1BeatNiko"));
	 analysis1->updControl("BeatNiko/an1BeatNiko/mrs_real/threshold", 1.0);
	 analysis1->updControl("BeatNiko/an1BeatNiko/mrs_natural/meanSamples", 30);
	 analysis1->updControl("BeatNiko/an1BeatNiko/mrs_real/lowercutoff", 20.0);
	 analysis1->updControl("BeatNiko/an1BeatNiko/mrs_real/uppercutoff", 90.0);

	 //add Beat
	 analysis1->addMarSystem(mng.create("Beat", "an1Beat"));
	 analysis1->updControl("Beat/an1Beat/mrs_real/threshold", 30.0);
	 analysis1->updControl("Beat/an1Beat/mrs_natural/bufferSize", 48000);


	 //configure analysis2
	 analysis2->addMarSystem(mng.create("Biquad", "bandpass"));
	 analysis2->addMarSystem(mng.create("Gain", "gain"));
	 analysis2->addMarSystem(mng.create("FullWaveRectifier", "rectifier"));
	 analysis2->addMarSystem(mng.create("ShiftInput", "win2"));
	 //analysis2->addMarSystem(mng.create("Square", "square"));
	 analysis2->addMarSystem(mng.create("AbsMax", "max"));
	 analysis2->addMarSystem(mng.create("BeatStat", "beatstat"));
	 analysis2->addMarSystem(mng.create("Beat", "Beat"));
	 //analysis2->addMarSystem(mng.create("FlowThru", "thru"));
	 analysis2->updControl("Gain/gain/mrs_real/gain",3.0);
	 analysis2->updControl("Biquad/bandpass/mrs_string/type","bandpass");
	 analysis2->updControl("Biquad/bandpass/mrs_real/resonance",3.0);
	 analysis2->updControl("Biquad/bandpass/mrs_real/frequency", 80.0);
	 analysis2->updControl("ShiftInput/win2/mrs_natural/winSize", 64);



	 ///FANOUT
	 //fanout->addMarSystem(analysis1);

	 topnet->addMarSystem(analysis2);
	 BeatRec *mar = dynamic_cast<BeatRec*>(analysis2->getChildMarSystem("Beat/Beat"));
	 mar->initSignal(generator);
	 topnet->updControl("AudioSourceBlocking/audioIn/mrs_natural/bufferSize", 64);
	 topnet->updControl("mrs_natural/inSamples", 64);
	 topnet->update();
	 std::cout << topnet->getControl("AudioSourceBlocking/audioIn/mrs_natural/onSamples") << std::endl;
	 m.lock();
	 running = false;
	 m.unlock();
}

Musikanalysis::~Musikanalysis() {
	// TODO Auto-generated destructor stub
}

void Musikanalysis::runAnalysis(Musikanalysis *music){
	while(true){
		music->m.try_lock();
		if(!music->running)
			break;
		music->m.unlock();
		//std::cout << "tick" <<std::endl;
		music->topnet->tick();
	}
}

void Musikanalysis::start(){

	m.lock();
	running = true;
	m.unlock();
	topnet->updControl("AudioSourceBlocking/audioIn/mrs_bool/initAudio", true);

	std::thread musikanal(Musikanalysis::runAnalysis, this);
	musikanal.join();
}

void Musikanalysis::stop(){
	topnet->updControl("AudioSourceBlocking/audioIn/mrs_bool/initAudio", false);
	m.lock();
	running = false;
	m.unlock();
}
