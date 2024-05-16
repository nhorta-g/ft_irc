#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "Client.hpp"
# include "Server.hpp"

/*class Channel;
class Client;
class Server;*/

class Command {
private:
	Command();
	~Command();
public:

	void Server::execCmd(Client *clt, std::string cmd)
};

#endif