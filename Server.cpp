# include "Server.hpp"
# include "Client.hpp"

void addChannel(std::string name) {
	
}

void Server::ClearClients(int fd){ //-> clear the clients
	for(size_t i = 0; i < _fds.size(); i++){
		if (_fds[i].fd == fd)
			{_fds.erase(_fds.begin() + i); break;}
	}
	for(size_t i = 0; i < _clients.size(); i++){
		if (_clients[i].GetFd() == fd)
			{_clients.erase(_clients.begin() + i); break;}
	}

}

bool Server::Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received! Ending server now" << std::endl;
	Server::Signal = true;
}

void	Server::CloseFds(){
	for(size_t i = 0; i < _clients.size(); i++){ //-> close all the clients
		std::cout << RED << "Client <" << _clients[i].GetFd() << "> Disconnected" << WHI << std::endl;
		close(_clients[i].GetFd());
	}
	if (_SerSocketFd != -1){ //-> close the server socket
		std::cout << RED << "Server <" << _SerSocketFd << "> Disconnected" << WHI << std::endl;
		close(_SerSocketFd);
	}
}

void Server::ReceiveNewData(int fd, Client &cli)
{

	if (cli.GetFd() == _SerSocketFd)
		std::cout << "deu ruim para localizar o cliente" << std::endl;

	char buff[1024];
	memset(buff, 0, sizeof(buff)); //-> clear the buffer to received data
	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0); //-> receive the data
	std::string in = buff;
	std::cout << "string received in general :" << in << std::endl;
	exit(1);

	if(bytes <= 0){
		std::cout << RED << "Client <" << fd << "> Disconnected" << WHI << std::endl;
		ClearClients(fd);
		close(fd);
	}


	else if (!cli.is_verified()){
			std::cout << YEL << "Client <" << fd << "> Data: " << WHI << in;
			if(in.find("CAP") != std::string::npos || in.find("pass") != std::string::npos || in.find("nick") != std::string::npos || in.find("user") != std::string::npos || in.find("PASS") != std::string::npos){
				registration(in, cli); // ve oque o babaca mandou, passing e set das vars do cliente obj
				validate_cli(cli);
			}
			else{
				std::cout << "you need to verify first Brother" << std::endl;
				client_sender(cli.GetFd(), "You need to verify first Brother\n");
			}
		}



	else if (in.find("\r\n") == std::string::npos){ // se vem do nc commando
			std::cout << YEL << "Client <" << fd << "> Data: " << WHI << in;
			cli.set_buffer(in.substr(0, in.find_first_of('\n'))); //porque ta vindo com \n aqui, manda oque o gajo mandou ate o \n
			client_sender(cli.GetFd(), "messege received not from hexchat, added to buffer: " + in);
		}
	else { // verificar como ta chegando string pelo hexchat
		std::vector<std::string> buffer_token = tokenit_please(cli.get_buffer() + in.substr(0, in.find_first_of('\n')));
		std::cout << "recebido por hex chat" << std::endl;
		if (buffer_token.empty())
			return ; // se ficar apertando enter que nem um retardado
		//check command() function?
	}


}

void Server::AcceptNewClient()
{
	Client cli; //-> create a new client, dava para jogar isso tudo num constructor mas foda-se, norminete é o caralho, constructor = AcceptNewClient()
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(_SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
		{std::cout << "accept() failed" << std::endl; return;}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		{std::cout << "fcntl() failed" << std::endl; return;}

	NewPoll.fd = incofd; //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //pollin = reading data
	NewPoll.revents = 0; //-> set the revents to 0

	cli.SetFd(incofd); //-> set the client file descriptor
	cli.setIpAdd(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	cli.set_verified(0);
	cli.set_user("");
	cli.set_bool_pass(0);
	cli.set_nick("");
	cli.set_buffer("");

	_clients.push_back(cli); //-> add the client to the vector of clients
	_fds.push_back(NewPoll); //-> add the client socket to the pollfd

	std::cout << GRE << "Client <" << incofd << "> Connected" << WHI << std::endl;
}

void Server::SerSocket()
{
	int en = 1;
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET; //->  ipv4
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
	add.sin_port = htons(this->_Port); //-> convert the port to network byte order (big endian)

	_SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if(_SerSocketFd == -1) //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	if(setsockopt(_SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = _SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0; //-> set the revents to 0
	_fds.push_back(NewPoll); //-> add the server socket to the pollfd
}

Client& Server::get_client(int fd, std::vector<Client>& cli){

	for (unsigned int i = 0 ; i < cli.size() ; i++){
		if (fd == cli[i].GetFd())
			return cli[i];
	}
	return cli[0]; // just in case
}

void Server::ServerInit()
{
	std::cout << "Server Password :" << serverpass << std::endl;
	SerSocket(); //-> create the server socket
	std::cout << GRE << "Server <" << _SerSocketFd << "> Connected" << WHI << std::endl;
	std::cout << "Waiting to accept a connection...\n";

	while (Server::Signal == false){

		if((poll(&_fds[0],_fds.size(),-1) == -1) && Server::Signal == false)
			throw(std::runtime_error("poll() fail"));

		for (size_t i = 0; i < _fds.size(); i++){ //-> check all file descriptors
			if (_fds[i].revents & POLLIN){ //-> check if there is data to read
				if (_fds[i].fd == _SerSocketFd) // se alteracao foi no fd do server entao so aceitamos
					AcceptNewClient();
				else
					ReceiveNewData(_fds[i].fd, get_client(_fds[i].fd, _clients)); //get_client devolve o cliente obj para modificarmos
			}
		}
	}
	CloseFds();
}

/*struct pollfd { server
 int     fd; //-> file descriptor
 short   events;//-> requested events
 short   revents; //-> returned events
};*/
