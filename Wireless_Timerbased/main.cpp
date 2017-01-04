/*
 * main.cpp
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */
#include <ctime>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "Musikanalysis.h"
#include "Equipment.h"
#include "Communicator.h"
#include "BehaviourGenerator.h"



std::vector<Equipment*> equipments;
std::queue<Equipment*> sendBuffer;
boost::asio::ip::udp::endpoint localEndpoint(boost::asio::ip::udp::v4(), 1103);



Communicator* server;
Musikanalysis* analyzer;
BehaviourGenerator* generator;

void startNetworkIn(){

	std::thread commIn(std::bind(Communicator::commInLoop, server));
	commIn.join();
}

void startNetworkOut(){

}

void startBehaviour(){
	generator->start();
	std::thread discoverThread(std::bind(BehaviourGenerator::equipmentDiscovery, generator));
	std::thread signalThread(std::bind(BehaviourGenerator::signalLoop, generator));

	discoverThread.join();
	signalThread.join();
}


int main(int argc, char* argv[])
{
	double beatperiod = 0;
	if(argc == 2){
		beatperiod = (double)atoi(argv[1]);
	}
	io_service io;
	localEndpoint.address(boost::asio::ip::address_v4::from_string("192.168.0.255"));
	server = new Communicator(2, 1103, &equipments, &sendBuffer, localEndpoint);
	generator = new BehaviourGenerator(&equipments, &sendBuffer);
	if(beatperiod == 0){
		analyzer = new Musikanalysis(generator);
	}
	else{
		analyzer = new Musikanalysis(generator, beatperiod);
	}


	server->startIn();
	server->startOut();
	generator->start();

	std::thread discoverThread(std::bind(BehaviourGenerator::equipmentDiscovery, generator));
	std::thread signalThread(std::bind(BehaviourGenerator::signalLoop, generator));
	std::thread commIn(std::bind(Communicator::commInLoop, server));
	std::thread commOut(std::bind(Communicator::commOutLoop, server));

	analyzer->start();

	discoverThread.join();
	signalThread.join();
	commIn.join();
	commOut.join();

	return 0;
}


