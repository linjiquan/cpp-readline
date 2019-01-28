// C/C++
#include <iostream>
#include <string>
#include <thread>

// Linux
#include <sys/types.h>
#include <sys/socket.h>         // SOCK_STREAM, AF_INET, AF_INET6, AF_UNSPEC, getsockname(), send((), recv(), SO_REUSEADDR, SOL_SOCKET, setsockopt(), getsockopt(), SO_ERROR
#include <netinet/in.h>         // sockaddr_in, htons(), ntohs(), IN6ADDR_ANY_INIT, SOL_IPV6, IPV6_V6ONLY
#include <netinet/ip.h>
#include <arpa/inet.h>          // inet_pton(), inet_ntoa()
#include <netdb.h>                      // AI_PASSIVE, NI_NUMERICHOST, getnameinfo(), getaddrinfo(), getprotobynumber()


// Internal
#include "../src/xma_shell.h"
#include "../src/xma_application.h"
#include "../src/xma_process.h"
#include "../src/xma_service.h"
#include "../src/xma_internal.h"

using namespace xma;

class TcpEchoListener: public EpollListener
{
public:
	TcpEchoListener(Process *context, int fd, std::string name);
	bool DoHandle(void * data) override;
private:
	Process *context_;
};


class TcpEchoServer: public Service
{
public:
	TcpEchoServer(std::string name, std::string address, uint16_t port) : Service(name), address_(address), port_(port) {
		XMA_DEBUG("Create tcp echo server[%s]: %s:%d", Name().c_str(), address_.c_str(), port_);
	}

	void OnInit() {
		//create local tcp echo server
		
	}

private:
	TcpEchoListener *listener_;
	std::string address_;
	uint16_t port_;
};
