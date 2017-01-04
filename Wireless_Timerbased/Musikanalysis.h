/*
 * Musikanalysis.h
 *
 *  Created on: 30.05.2015
 *      Author: christoph
 */

#ifndef MUSIKANALYSIS_H_
#define MUSIKANALYSIS_H_
#include <iostream>
#include <marsyas/system/MarSystemManager.h>
#include <marsyas/system/MarSystem.h>
#include <thread>
#include <mutex>
#include "BehaviourGenerator.h"



class Musikanalysis {
private:
	Marsyas::MarSystemManager mng;
	Marsyas::MarSystem* topnet;
	Marsyas::MarSystem* fanout;
	Marsyas::MarSystem* analysis1;
	Marsyas::MarSystem* analysis2;
	BehaviourGenerator* generator;
	std::mutex m;
	bool running;
public:
	Musikanalysis(BehaviourGenerator* generator, double beatperiod);
	Musikanalysis(BehaviourGenerator* generator);
	virtual ~Musikanalysis();
	static void runAnalysis(Musikanalysis *music);
	void start();
	void stop();

};

#endif /* MUSIKANALYSIS_H_ */
