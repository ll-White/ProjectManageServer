#pragma once

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

class WnUdpClient
{
public:
	WnUdpClient();
	~WnUdpClient();

	SOCKET CreateUdp(SOCKADDR_IN addr);
	void CloseUdp();
	bool SendCmd(char *cmd, int cmdsize);
	bool SendCmdAndRecv(char *cmd, int cmdsize, char *retbuf, int bufsize, int tmout);

private:
	SOCKET m_udpSocket = INVALID_SOCKET;
	SOCKADDR_IN m_siRemote{};
};