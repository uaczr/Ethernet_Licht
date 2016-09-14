/*
 * Equipment.cpp
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#include "Equipment.h"

Equipment::Equipment() {
	// TODO Auto-generated constructor stub

	this->equipmentType = None;
	this->setIt = false;
	this->syncIt = false;
}

void Equipment::subscribing(udp::endpoint endPoint, char* buffer,
		size_t length) {
	this->endPoint = endPoint;
	this->subMesg = subscribeMessage(buffer, length);
	this->equipmentType = subMesg.equipmentType;
	this->last_contact = std::chrono::steady_clock::now();
}

void Equipment::set() {
	this->setIt = true;
}

void Equipment::synchronise() {
	this->syncIt = true;
}
void Equipment::now(){
	this->last_contact = std::chrono::steady_clock::now();
}
void Equipment::pushing(char* buffer, size_t length) {
	this->pushMesg = pushingMessage(buffer, length);
	this->last_contact = std::chrono::steady_clock::now();
}

Equipment::~Equipment() {
	// TODO Auto-generated destructor stub
}

