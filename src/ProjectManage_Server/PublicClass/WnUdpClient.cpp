#include "WnUdpClient.h"

WnUdpClient::WnUdpClient()
{
}

WnUdpClient::~WnUdpClient()
{
	CloseUdp();
}

SOCKET WnUdpClient::CreateUdp(SOCKADDR_IN addr)
{
	m_siRemote = addr;

	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	m_udpSocket = client;
	return m_udpSocket;
}

void WnUdpClient::CloseUdp()
{
	if (m_udpSocket != INVALID_SOCKET)
	{
		closesocket(m_udpSocket);
		m_udpSocket = INVALID_SOCKET;
	}
}

bool WnUdpClient::SendCmd(char *cmd, int cmdsize)
{
	int nSend = sendto(m_udpSocket, cmd, cmdsize, 0,
					   reinterpret_cast<sockaddr *>(&m_siRemote),
					   sizeof(m_siRemote));
	return nSend >= 0;
}

bool WnUdpClient::SendCmdAndRecv(char *cmd, int cmdsize, char *retbuf, int bufsize, int tmout)
{
	(void)tmout;
	int nSend = sendto(m_udpSocket, cmd, cmdsize, 0,
					   reinterpret_cast<sockaddr *>(&m_siRemote),
					   sizeof(m_siRemote));
	if (nSend < 0)
		return false;

	SOCKADDR_IN addr{};
	int addrLen = sizeof(addr);

	int nRecv = recvfrom(m_udpSocket, retbuf, bufsize, 0,
						 reinterpret_cast<sockaddr *>(&addr), &addrLen);

	return nRecv > 0;
}