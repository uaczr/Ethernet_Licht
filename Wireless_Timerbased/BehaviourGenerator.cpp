/*
 * BehaviourGenerator.cpp
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#include "BehaviourGenerator.h"

BehaviourGenerator::BehaviourGenerator(std::vector<Equipment*>* equipments, std::queue<Equipment*>* sendBuffer) {
	// TODO Auto-generated constructor stub
	this->sendBuffer = sendBuffer;
	this->equipments = equipments;
	this->beatIndicator = false;
	this->runningIndicator = false;
	this->equipmentIntervall = std::chrono::milliseconds(1000);
	this->signalIntervall = std::chrono::milliseconds(2);
	this->contactIntervall = std::chrono::milliseconds(5000);

	this->beat_period_millis = 500.0;
	this->beat_distinctivness = 1.0;

	this->ptrAllSender = 0;
	this->ptrControl = 0;

}

BehaviourGenerator::~BehaviourGenerator() {
	// TODO Auto-generated destructor stub
}

void BehaviourGenerator::start() {
	if(!checkRunning()){
		setRunning();
	}
}

void BehaviourGenerator::stop() {
	if(checkRunning()){
		unsetRunning();
	}
}

void BehaviourGenerator::setBeat() {
	this->mBeatIndicator.lock();
	beatIndicator = true;
	this->mBeatIndicator.unlock();
}

void BehaviourGenerator::unsetBeat() {
	this->mBeatIndicator.lock();
	beatIndicator = false;
	this->mBeatIndicator.unlock();
}

bool BehaviourGenerator::checkBeat() {
	return beatIndicator;
}


void BehaviourGenerator::setRunning() {
	this->mRunningIndicator.lock();
	runningIndicator = true;
	this->mRunningIndicator.unlock();
}

void BehaviourGenerator::unsetRunning() {
	this->mRunningIndicator.lock();
	runningIndicator = false;
	this->mRunningIndicator.unlock();
}

bool BehaviourGenerator::checkRunning() {
	return runningIndicator;
}



void BehaviourGenerator::loop() {
	std::chrono::milliseconds duration(1000);
	while(runningIndicator){
		if(!equipments->empty()){
			Equipment* temp1 = getAllSender();
			if(!(temp1 == 0)){
				temp1->synchronise();
				addToQueue(temp1);
			}
		}

		std::this_thread::sleep_for(duration);

	}
}

void BehaviourGenerator::addToQueue(Equipment* equipment) {
	if(this->sendBuffer->size() < 10){
		sendBuffer->push(equipment);
	}
}

Equipment* BehaviourGenerator::getAllSender() {

	std::vector<Equipment*>::iterator iter = std::find_if(equipments->begin(), equipments->end(), std::bind(Equipment::findByType, std::placeholders::_1, All));
	if(iter != equipments->end()){
		return (*iter);
	}
	return 0;


}

void BehaviourGenerator::beat(BehaviourGenerator* gen, double data1,
		double data2) {
	gen->beatIndicator = true;
	gen->beat_period_millis = data1;
	gen->beat_distinctivness = data2;
}

void BehaviourGenerator::sortEquipment() {
	if(!equipments->empty()){
		for(std::vector<Equipment*>::iterator it = equipments->begin(); it != equipments->end(); it++){
			Equipment* temp = (*it);
			/*if(std::chrono::steady_clock::now()-temp->last_contact > this->contactIntervall && temp->equipmentType != All){
				equipments->erase(it);
				break;
			}*/
			if(temp->equipmentType == Control){
				this->ptrControl = temp;
			}
			if(temp->equipmentType == All){
				this->ptrAllSender = temp;
			}
		}
	}
}

void BehaviourGenerator::equipmentDiscovery(BehaviourGenerator* gen) {
	while(gen->runningIndicator){
		gen->sortEquipment();
		std::this_thread::sleep_for(gen->equipmentIntervall);
	}
}

void BehaviourGenerator::signal(){
	if(this->ptrAllSender != 0 && beatIndicator){
		beatIndicator = false;
		this->ptrAllSender->syncMesg.beat_distinctivness = (uint8_t)(this->beat_distinctivness*100);
		this->ptrAllSender->syncMesg.beat_period_millis = (uint16_t)(this->beat_period_millis);
		this->ptrAllSender->synchronise();
		addToQueue(this->ptrAllSender);

	}
}

void BehaviourGenerator::signalLoop(BehaviourGenerator* gen) {
	while(gen->runningIndicator){

		gen->signal();
		std::this_thread::sleep_for(gen->signalIntervall);
	}
}
