//
//
//
//void Ircserv::commandJoin(const std::string &channel)
//{
//	if (channel.empty() || channel[0] != '#')
//	{
//		std::string errMsg = ":ircserver 461 * JOIN: Invalid Channel!\r\n";
//		send(_clientFd, errMsg.c_str(), errMsg.size(), 0);
//		return ;
//	}
//
//	//Se nao existir, cria um novo canal
//	if (_channels.find(channel) == _channels.end())
//	{
//		std::cout << green << "Nao existe channel, criado um novo" << channel << "\n" << reset;
//		_channels.insert(std::make_pair(channel, std::vector<Client>()));
//		_channels[channel].push_back(returnClientStruct(_clientFd));
//
//		std::string testeMsg = ":" + _clientsMap[_clientFd]._nickName + "!" + _clientsMap[_clientFd]._userName + "@localhost JOIN " + channel + "\r\n";
//		send(_clientFd, testeMsg.c_str(), testeMsg.size(), 0);
//
//		std::string msgTopic = ":ircserver 332 " + _clientsMap[_clientFd]._nickName + " " + channel + " :My cool server yay!\r\n";
//		send(_clientFd, msgTopic.c_str(), msgTopic.size(), 0);
//
//		_channelTopics.insert(std::make_pair(channel ,"My cool server yay!"));
//		return ;
//	}
//	else
//	{
//		//Verifica se ja existe o clientFd igual no canal, caso nao tenha, coloque na lista
//		if (!checkIfClientInChannel(_channels, channel, _clientFd))
//		{
//			_channels[channel].push_back(returnClientStruct(_clientFd));
//		}
//	}
//
//	//Mensagem de confirmacao para dar JOIN
//	std::string testeMsg = ":" + _clientsMap[_clientFd]._nickName + "!" + _clientsMap[_clientFd]._userName + "@localhost JOIN " + channel + "\r\n";
//	send(_clientFd, testeMsg.c_str(), testeMsg.size(), 0);
//
//
//	std::string msgTopic = ":ircserver 332 " + _clientsMap[_clientFd]._nickName + " " + channel + " :" + _getChannelTopic(channel) + "\r\n";
//	send(_clientFd, msgTopic.c_str(), msgTopic.size(), 0);
//
//	makeUserList(channel);
//
//	return ;
//}