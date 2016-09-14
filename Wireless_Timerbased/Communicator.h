/*
 * Communicator.h
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#ifndef COMMUNICATOR_H_
#define COMMUNICATOR_H_
#include "Equipment.h"
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <functional>
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>


#include "Protocol.h"


using boost::asio::ip::udp;
using boost::asio::io_service;


class Equipment;
class Communicator {
private:
	const char ack = '\x06';
	const char nak = '\x15';
	const char stx = '\x02';
	const char etx = '\x03';

	bool runningInIndicator;
	std::mutex mRunningInIndicator;
	size_t bytesReceived;

	bool runningOutIndicator;
	std::mutex mRunningOutIndicator;

	std::vector<Equipment*>* equipments;
	std::queue<Equipment*>* sendBuffer;
	boost::system::error_code error;


	udp::endpoint endPoint;
	boost::array<char, 16> recvBuffer;

	io_service ioServiceIn_;
	io_service ioServiceOut_;

	int intervall;

public:
	udp::socket* udpSocketIn;
	udp::socket* udpSocketOut;
	Communicator(
			int intervall,
			const int port_,
			std::vector<Equipment*>* equipments,
			std::queue<Equipment*>* sendBuffer,
			boost::asio::ip::udp::endpoint localEndpoint
			);

	virtual ~Communicator();

	void startIn();
	void stopIn();
	void loopIn();

	void setInRunning();
	void unsetInRunning();
	bool checkInRunning();

	void startOut();
	void stopOut();
	void loopOut();

	void setOutRunning();
	void unsetOutRunning();
	bool checkOutRunning();
	static void commInLoop(Communicator* com);
	static void commOutLoop(Communicator* com);

private:

	std::string generateResponse(size_t size);




};

#endif /* COMMUNICATOR_H_ */
