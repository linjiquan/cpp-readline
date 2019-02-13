#pragma once

class StartConnect: public Msg
{
public:
	StartConnect(): Msg("StartConnect") {}

	void Handler() override 
	{
		TcpClientProcess *proc = dynamic_cast<TcpClientProcess *>(Thread::current_);
		proc->GetClientService()->StartClient(address, port);
	}
	
	std::string address;
	uint16_t port;
};

class StopConnect: public Msg
{
public:
	StopConnect(): Msg("StopConnect") {}

	void Handler() override 
	{
		TcpClientProcess *proc = dynamic_cast<TcpClientProcess *>(Thread::current_);
		proc->GetClientService()->StopClient();
	}
};

