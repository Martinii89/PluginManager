// plugininstaller.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "SettingsManager.h"
#include <vector>
#include <fstream>
#include <sstream>
#include "easywsclient/easywsclient.hpp"
static inline unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch);

int main(int argc, char *argv[])
{
	RegisterySettingsManager settings;
	std::wstring registryString = settings.GetStringSetting(L"BakkesModPath", RegisterySettingsManager::REGISTRY_DIR_APPPATH);
	if (registryString.empty())
	{
		std::cout << "Could not find BakkesMod path. Are you sure it's installed?" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}
	if (argc < 2)
	{
		std::cout << "Missing args" << std::endl;
		getchar();
		return EXIT_FAILURE;
	}

	int rcon_port = 9002;
	std::string rcon_password = "password";

	std::wstring config_path = registryString + L"cfg/config.cfg";
	if (GetFileAttributesW(config_path.c_str()) != 0xFFFFFFFF) //file exists
	{
		std::ifstream readConfig(config_path);
		std::string line;
		
		while (std::getline(readConfig, line))
		{
			std::istringstream iss(line);
			std::string name, val;
			if (!(iss >> name >> val)) { break; } // error

			if (name.compare("rcon_port") == 0)
			{
				if (val.front() == '"')
				{
					val = val.substr(1);
				}
				if (val.back() == '"')
				{
					val.pop_back();
				}
				rcon_port = std::stoi(val);
			}
			if (name.compare("rcon_password") == 0)
			{
				if (val.front() == '"')
				{
					val = val.substr(1);
				}
				if (val.back() == '"')
				{
					val.pop_back();
				}
				rcon_password = val;
			}
		}
	}

	std::string param = argv[1];
	if (param.find("bakkesmod://") == 0)
	{
		param = param.substr(std::string("bakkesmod://").size());
	}
	/*else
	{
		param = argv[1];
	}*/
	std::vector<std::string> params;
	split(param, params, '/');
	std::string command = params.at(0);
	std::cout << param << ", command: " << command << std::endl;
	if (command.compare("install") == 0) 
	{
		if (params.size() < 2)
		{
			std::cout << "Usage: install/id" << std::endl;
			getchar();
			return EXIT_FAILURE;
		}
		std::vector<std::string> ids;
		split(params.at(1), ids, ',');
		for (std::string id : ids)
		{
#ifdef _WIN32
			INT rc;
			WSADATA wsaData;

			rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if (rc) {
				std::cout << "WSAStartup Failed." << std::endl;
				getchar();
				return 1;
			}
#endif
			bool success = false;
			std::string ws_url = "ws://localhost:" + std::to_string(rcon_port) + "/";
			std::cout << "Trying to connect to " << ws_url << std::endl;
			try {
				std::unique_ptr<easywsclient::WebSocket> ws(easywsclient::WebSocket::from_url(ws_url));
				if (ws.get() != NULL) {
					ws->send("rcon_password " + rcon_password);
					while (ws->getReadyState() != easywsclient::WebSocket::CLOSED) {
						easywsclient::WebSocket::pointer wsp = &*ws;
						ws->poll();
						ws->dispatch([succ = &success, wsp, id](const std::string & message) {
							printf(">>> %s\n", message.c_str());
							if (message == "authyes") {
								wsp->send(std::string("bpm_install ") + id);
								wsp->poll();

								*succ = true;
								std::cout << "Plugin install called through websockets!" << std::endl;
							}
							else if (message == "authyes") {
								*succ = false;
								std::cout << "Plugin install through websockets failed, invalid password!" << std::endl;
							}
							else
							{
								std::cout << "Unknown message from websocket server!" << std::endl;
							}
							wsp->close();
						});
					}
				}
				else
				{
					std::cout << "Websocket server not running. (AKA RL is not running or BM is not injected)" << std::endl;
				}
			}
			catch (...) {
				std::cout << "Websockets failed for some reason (RL not running?), using newfeatures.apply file instead" << std::endl;
			}
#ifdef _WIN32
			WSACleanup();
#endif
			if (!success)
			{
				std::ofstream outfile;

				outfile.open(registryString + L"/data/newfeatures.apply", std::ios_base::app);
				outfile << "bpm_install " << id << std::endl;
				std::cout << "Added line \"bpm_install " << id << "\" to /data/newfeatures.apply" << std::endl;
				std::cout << "Plugin will be installed next time you launch the game!" << std::endl;
			}

		}
	}
	getchar();
	
}



static inline unsigned int split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
	unsigned int pos = txt.find(ch);
	unsigned int initialPos = 0;
	strs.clear();

	// Decompose statement
	while (pos != std::string::npos) {
		strs.push_back(txt.substr(initialPos, pos - initialPos));
		initialPos = pos + 1;

		pos = txt.find(ch, initialPos);
	}

	// Add the last one
	strs.push_back(txt.substr(initialPos, min(pos, txt.size()) - initialPos));
	return strs.size();
}
