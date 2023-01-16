// ConsoleApplication2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "pch.h"
#include <iostream>
#include <sstream>
#include <iterator>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include "RSJparser.h"

#pragma comment (lib, "crypt32")

using namespace std;
int runServer() {
	ix::WebSocketServer server(8080, "0.0.0.0", ix::SocketServer::kDefaultTcpBacklog, 10000);

	server.setOnClientMessageCallback([&server](std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
		if (msg->type == ix::WebSocketMessageType::Open)
		{
			std::cerr << "New connection" << std::endl;

			//				std::cerr << "id: " << connectionState->getId() << std::endl;
			//				std::cerr << "Uri: " << msg->openInfo.uri << std::endl;

			std::cerr << "Headers:" << std::endl;
			for (auto it : msg->openInfo.headers)
			{
				std::cerr << it.first << ": " << it.second << std::endl;
			}

			webSocket.setUrl(msg->openInfo.headers["channels"]);
		}
		else if (msg->type == ix::WebSocketMessageType::Message) {

			std::cerr << msg->str << std::endl;

			RSJresource msg_json(msg->str);
			string channel(msg_json["channel"].as<string>());

			if (channel.compare("system") == 0) {
				string command(msg_json["command"].as<string>());
				std::vector<string> items;

				if (command.compare("get_online") == 0) {
					std::set<std::shared_ptr<ix::WebSocket>> clients = server.getClients();

					for (auto it : clients) {
						items.push_back(it->getUrl());
					}

					std::ostringstream imploded;
					std::copy(items.begin(), items.end(),
						std::ostream_iterator<std::string>(imploded, ""));

					cerr << imploded.str() << endl;
					webSocket.sendText(imploded.str());
				}
				return;
			}

			std::set<std::shared_ptr<ix::WebSocket>> clients = server.getClients();

			for (auto it : clients) {
				string channels(it->getUrl());
				if (channels.find(channel) != channels.npos) {
					it->sendText(msg->str);
				}
			}

		}
	});
	auto res = server.listen();
	if (!res.first)
	{
		// Error handling
		return 1;
	}

	// Run the server in the background. Server can be stoped by calling server.stop()
	server.start();

	// Block until server.stop() is called.
	server.wait();

	getchar();

	server.stop();

	return 0;
}

int main(int argc, char *argv[])
{
	ix::initNetSystem();

	runServer();
}
