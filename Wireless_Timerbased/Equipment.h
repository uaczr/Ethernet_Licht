/*
 * Equipment.h
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#ifndef EQUIPMENT_H_
#define EQUIPMENT_H_
#include <vector>
#include <functional>
#include <chrono>
#include <boost/asio.hpp>
#include "Protocol.h"
#include "Communicator.h"
using boost::asio::ip::udp;

class Equipment {
public:
	udp::endpoint endPoint;
	unsigned int equipmentType;
	std::chrono::steady_clock::time_point last_contact;
	subscribeMessage subMesg;
	settingMessage setMesg;
	synchronisingMessage syncMesg;
	pushingMessage pushMesg;

	bool setIt;
	bool syncIt;

	void subscribing(udp::endpoint endPoint, char* buffer, size_t length);
	void set();
	void synchronise();
	void pushing(char* buffer, size_t length);
	void now();

	Equipment();
	virtual ~Equipment();

	static bool findByEndpoint(Equipment* equipment, udp::endpoint* endPoint){
		if(equipment->endPoint.address().to_string().compare(endPoint->address().to_string()) == 0){
			return true;
		}
		return false;
	};
	static bool findByType(Equipment* equipment, const unsigned int Type){
		if(equipment != 0x0){
			if(equipment->equipmentType == Type){
				return true;
			}
		}
		return false;
	};
};

#endif /* EQUIPMENT_H_ */
