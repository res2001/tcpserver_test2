/*
 * tcpserver_test2.cpp
 *
 *  Created on: 20 мар. 2019 г.
 *      Author: res
 */
#include <iostream>
#include <thread>
#include <cstring>

#include "tcpsocket.h"
#include "sequences.h"

#define DEFAULT_PORT			"8080"
#define DEFAULT_HOST			nullptr

void usage(const char* progname)
{
	std::clog	<< "Usage: " << progname << " [<host IP>] [<port>]" << std::endl
				<< "Default value:" << std::endl
				<< "\thost IP\t- all" << std::endl
				<< "\tport - " DEFAULT_PORT << std::endl;
}

void test2_thread_func(std::unique_ptr<tcp_socket> socket)
{
	try
	{
		sequences seq(std::move(socket));
		if(!seq.read_commands())
		{
			return;
		}
		seq.send_sequences();
	}
	catch(socket_exception & se)
	{
		std::clog << "Error: " << se.nerr_ << " - " << se.serr_ << std::endl;
	}
	catch (const std::bad_alloc& e)
	{
		std::clog << "Allocation failed in socket thread: " << e.what() << std::endl;
	}
}

int main(int argc, const char **argv)
{
	const char *host = nullptr;
	const char *svc = nullptr;
	if(argc == 1)
	{
		host = DEFAULT_HOST;
		svc = DEFAULT_PORT;
	} else if(argc == 2) {
		if(std::strcmp(argv[1], "-h") == 0)
		{
			usage(argv[0]);
			return 0;
		}
		svc = argv[1];
	} else if(argc == 3) {
		host = argv[1];
		svc = argv[2];
	} else {
		usage(argv[0]);
		return 1;
	}

	try
	{
		tcp_server srv(host, svc);
		while(true)
		{
			std::unique_ptr<tcp_socket> socket= srv.accept();
			std::thread th(test2_thread_func, std::move(socket));
			th.detach();
		}
	}
	catch(socket_exception &se)
	{
		std::clog << "Socket error: " << se.nerr_ << " : " << se.serr_ << std::endl;
	}
	catch (const std::bad_alloc& e)
	{
		std::clog << "Allocation failed: " << e.what() << std::endl;
	}
	return 0;
}
