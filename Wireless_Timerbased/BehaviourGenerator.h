/*
 * BehaviourGenerator.h
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#ifndef BEHAVIOURGENERATOR_H_
#define BEHAVIOURGENERATOR_H_

#include <vector>
#include <queue>
#include <mutex>
#include <functional>
#include <chrono>
#include <thread>
#include "Communicator.h"
#include "Equipment.h"
#include "Protocol.h"

class BehaviourGenerator {
	std::vector<Equipment*>* equipments;
	std::queue<Equipment*>* sendBuffer;

	Equipment* ptrControl;
	Equipment* ptrAllSender;
	bool beatIndicator;
	bool runningIndicator;

	double beat_period_millis;
	double beat_distinctivness;
	std::mutex mBeatIndicator;
	std::mutex mRunningIndicator;

	std::chrono::duration<double, std::milli> equipmentIntervall;
	std::chrono::duration<double, std::milli> signalIntervall;

	std::chrono::duration<double, std::milli> contactIntervall;
public:
	BehaviourGenerator(std::vector<Equipment*>* equipments, std::queue<Equipment*>* sendBuffer);
	virtual ~BehaviourGenerator();

	void start();
	void stop();

	void loop();

	void setBeat();
	void unsetBeat();
	bool checkBeat();

	void setRunning();
	void unsetRunning();
	bool checkRunning();

	void addToQueue(Equipment* equipment);

	Equipment* getAllSender();

	void sortEquipment();
	void signal();

	static void equipmentDiscovery(BehaviourGenerator* gen);
	static void signalLoop(BehaviourGenerator* gen);

	static void beat(BehaviourGenerator* gen, double data1, double data2);
};

#endif /* BEHAVIOURGENERATOR_H_ */
