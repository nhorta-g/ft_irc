//#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

# include "Client.hpp"

#include <iostream>
#include <vector> //-> for vector
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <arpa/inet.h> //-> for inet_ntoa()
#include <poll.h> //-> for poll()
#include <csignal> //-> for signal()
#include <stdio.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <map>
//-------------------------------------------------------//
#define RED "\e[1;31m"
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class Client;
class Channel;

class Server
{
private:
	int _Port; //-> server port
	int _SerSocketFd; //-> server fd
	static bool Signal;
	std::vector<Client> _clients;
	std::map<std::string, Channel*> _channels;
	std::vector<struct pollfd> _fds;

	std::string serverpass;
public:
	Server(int port, char *pass){
		_SerSocketFd = -1;
		_Port = port;
		std::string tmp(pass);
		serverpass = tmp;
	}
	void ServerInit();
	void SerSocket();
	void AcceptNewClient();
	void ReceiveNewData(int fd, Client&);
	static void SignalHandler(int signum);
	void CloseFds();
	void ClearClients(int fd);

	Client& get_client(int, std::vector<Client>&);
	void registration(std::string, Client&);

	void validate_cli(Client&);

	void client_sender(int fd, std::string str){
	if(send(fd, str.c_str(), str.size(), 0) == -1)
		std::cerr << "not able to send data" << std::endl;
	}
	std::vector<std::string> tokenit_please(std::string str);
	// Channel NUNO
	void addChannel(std::string name);
};




# endif