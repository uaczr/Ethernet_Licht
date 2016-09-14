/*
 * Communicator.cpp
 *
 *  Created on: 19.08.2016
 *      Author: christoph
 */

#include "Communicator.h"

Communicator::Communicator(
		int intervall,
		const int port_,
		std::vector<Equipment*>* equipments,
		std::queue<Equipment*>* sendBuffer,
		udp::endpoint localEndpoint)
{


	this->intervall = intervall;
	udpSocketIn = new udp::socket(ioServiceIn_, udp::endpoint(udp::v4(), port_));
	udpSocketOut= new udp::socket(ioServiceOut_, udp::endpoint(udp::v4(), port_+1));

	this->sendBuffer = sendBuffer;
	this->equipments = equipments;
	Equipment* temp = new Equipment;

	runningInIndicator = false;
	runningOutIndicator = false;

	bytesReceived = 0;

	char buff[2];
	buff[0] = '0';
	buff[1] = '1';

	temp->subscribing(localEndpoint, buff, 2);
	equipments->push_back(temp);

	udpSocketIn->set_option(udp::socket::reuse_address(true));
	udpSocketIn->set_option(boost::asio::socket_base::broadcast(true));
	udpSocketOut->set_option(udp::socket::reuse_address(true));
	udpSocketOut->set_option(boost::asio::socket_base::broadcast(true));

	//startReceive();

}

Communicator::~Communicator() {
	// TODO Auto-generated destructor stub
	udpSocketIn->close();
	udpSocketOut->close();
}

std::string Communicator::generateResponse(size_t size) {

	if(subscribing(recvBuffer.elems, size)){
		if(equipments->size() >=1){
			auto iter = std::find_if(equipments->begin(), equipments->end(), std::bind(Equipment::findByEndpoint, std::placeholders::_1, &endPoint));
			if(iter != equipments->end()){
				std::cout << "Subscribe Event:\tExisting Equipment" << std::endl;
				std::cout << "Address:\t" << endPoint.address().to_string() <<  std::endl;
				std::cout << "Type:\t" << recvBuffer[1] << std::endl << std::endl;
				return std::string(std::to_string(StatusMessage) + std::to_string('0'));
			}
		}
		Equipment* temp = new Equipment;
		temp->subscribing(endPoint, recvBuffer.elems, size);
		equipments->push_back(temp);
		std::cout << "Subscribe Event:\tNew Equipment" << std::endl;
		std::cout << "Address:\t" << endPoint.address().to_string() << std::endl;
		std::cout << "Type:\t" << recvBuffer[1] << std::endl << std::endl;
		return std::string(std::to_string(StatusMessage) + std::to_string('0'));
	}

	if(pushing(recvBuffer.elems, size)){
		if(equipments->size() >=1){
			auto iter = std::find_if(equipments->begin(), equipments->end(), std::bind(Equipment::findByEndpoint, std::placeholders::_1, &endPoint));
			if(iter == equipments->end()){
				std::cout << "Push Event:\tNon Exisiting Equipment Update" << std::endl;
				std::cout << "Address:\t" << endPoint.address().to_string() << std::endl << std::endl;
				return std::string(std::to_string(StatusMessage) + std::to_string('1'));
			}
			else{
				(*iter)->pushMesg = pushingMessage(recvBuffer.elems, size);
				std::cout << "Push Event:\tUpdate Data" << std::endl;
				std::cout << "Address:\t" << endPoint.address().to_string() << std::endl << std::endl;
				return std::string(std::to_string(StatusMessage) + std::to_string('0'));
			}
		}
		else{
			std::cout << "Push Event:\tNon Exisiting Equipment Update" << std::endl;
			std::cout << "Address:\t" << endPoint.address().to_string() << std::endl << std::endl;
			return std::string(std::to_string(StatusMessage) + std::to_string('1'));
		}

	}
	if(synchronising(recvBuffer.elems, size)){
			if(equipments->size() >=1 && recvBuffer.elems[1] == '1'){
				std::vector<Equipment*>::iterator iter = std::find_if(equipments->begin(), equipments->end(), std::bind(Equipment::findByEndpoint, std::placeholders::_1, &endPoint));
				if(iter != equipments->end()){
					(*iter)->now();
					std::cout << "Sync Event:\tEquipment Beacon" << std::endl;
					std::cout << "Sync Event:\tEquipment Beacon" << std::endl;
					std::cout << "Address:\t" << endPoint.address().to_string() << std::endl <<  std::endl;
					return std::string(std::to_string(StatusMessage) + std::to_string('0'));
				}
			}
		}
	return std::string(std::to_string(StatusMessage) + std::to_string('1'));
}

void Communicator::startIn() {
	if(!checkInRunning()){
		setInRunning();
	}
}

void Communicator::stopIn() {
	if(checkInRunning()){
		unsetInRunning();
	}
}

void Communicator::loopIn() {
		bytesReceived = udpSocketIn->receive_from(boost::asio::buffer(recvBuffer), endPoint, 0 ,error);
		if (error && error != boost::asio::error::message_size)
		        throw boost::system::system_error(error);
		std::string message = generateResponse(bytesReceived);
		udpSocketIn->send_to(boost::asio::buffer(message),
				endPoint, 0, error);
}

void Communicator::setInRunning() {
	this->mRunningInIndicator.lock();
	runningInIndicator = true;
	this->mRunningInIndicator.unlock();
}

void Communicator::unsetInRunning() {
	this->mRunningInIndicator.lock();
	runningInIndicator = false;
	this->mRunningInIndicator.unlock();
}

bool Communicator::checkInRunning() {
	return runningInIndicator;
}

void Communicator::startOut() {
	if(!checkOutRunning()){
		setOutRunning();
	}
}

void Communicator::stopOut() {
	if(checkOutRunning()){
		unsetOutRunning();
	}
}

void Communicator::loopOut() {

	if (!sendBuffer->empty()) {
		Equipment* first;
		first = sendBuffer->front();
		if (first->setIt) {
			udpSocketOut->send_to(boost::asio::buffer(first->setMesg.buffer),
					first->endPoint);
			std::cout << "Send Event:\t Setting Message sent" << std::endl;
			std::cout << "Port:\t" << first->endPoint.port() << std::endl;
			std::cout << "Target:\t" << first->endPoint.address().to_string()
					<< std::endl;
			std::cout << "MessageType:\t" << first->setMesg.buffer << std::endl
					<< std::endl;
			first->setIt = false;
		}
		if (first->syncIt) {
			udpSocketOut->send_to(boost::asio::buffer(first->syncMesg.buffer),
					first->endPoint);
			std::cout << "Send Event:\t Syncing Message sent" << std::endl;
			std::cout << "Port:\t" << first->endPoint.port() << std::endl;
			std::cout << "Target:\t" << first->endPoint.address().to_string()
					<< std::endl;
			std::cout << "MessageType:\t" << first->syncMesg.messageType << std::endl;
			std::cout << "BeatPeriod:\t" << first->syncMesg.beat_period_millis << std::endl;
			std::cout << "BeatDist:\t" << (int) first->syncMesg.beat_distinctivness << std::endl
					<< std::endl;
			first->syncIt = false;
		}
		if (!first->syncIt && !first->setIt) {
			sendBuffer->pop();
		}
	}
}

void Communicator::setOutRunning() {
	this->mRunningOutIndicator.lock();
	runningOutIndicator = true;
	this->mRunningOutIndicator.unlock();
}

void Communicator::unsetOutRunning() {
	this->mRunningOutIndicator.lock();
	runningOutIndicator = false;
	this->mRunningOutIndicator.unlock();
}

bool Communicator::checkOutRunning() {
	return runningOutIndicator;
}

void Communicator::commInLoop(Communicator* com) {
	while (com->runningInIndicator) {

		com->loopIn();
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(com->intervall));
	}
}
void Communicator::commOutLoop(Communicator* com) {
	while (com->runningInIndicator) {

		com->loopOut();
		std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(com->intervall));
	}
}
