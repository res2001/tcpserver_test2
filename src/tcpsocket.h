/*
 * tcpsocket.h
 *
 *  Created on: 20 мар. 2019 г.
 *      Author: res
 */

#ifndef SRC_TCPSOCKET_H_
#define SRC_TCPSOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstddef>
#include <cassert>
#include <vector>
#include <string>
#include <memory>

#include "socket_exception.h"

#define BACKLOG			5
#define RECVBUF_SIZE	4096
#define READ_LINE_RESERV_STRING	512
#define READ_LINE_MAX_STRING	4096

class tcp_socket
{
private:
	using recvbuf_type = std::vector<char>;
	int fd_ = -1;
	socklen_t addrlen_ = 0;
	struct sockaddr_in6 addr_ = { 0 };
	recvbuf_type recvbuf_ = recvbuf_type(RECVBUF_SIZE, 0);
	size_t recvbuf_cnt_ = 0;
	size_t recvbuf_idx_ = 0;
public:
	tcp_socket() = delete;
	tcp_socket(const tcp_socket&) = delete;
	tcp_socket & operator=(const tcp_socket &) = delete;
	tcp_socket & operator=(tcp_socket &&) = delete;
	tcp_socket(const tcp_socket &&) = delete;

	~tcp_socket();
	tcp_socket(int fd, struct sockaddr * sock, socklen_t addrlen);
	void sendall(const void * data, size_t len);
	size_t recv(void * buf, size_t len);
	std::string read_line();
private:
	bool recvbuf_is_empty() noexcept
	{
		assert(((recvbuf_cnt_ == 0 && recvbuf_idx_ == 0) || recvbuf_cnt_ > 0) && recvbuf_cnt_ <= RECVBUF_SIZE && recvbuf_idx_ < RECVBUF_SIZE);
		return recvbuf_cnt_ == 0;
	}
	recvbuf_type::value_type* recvbuf_get_ptr() noexcept
	{
		assert(recvbuf_idx_ < RECVBUF_SIZE);
		return recvbuf_.data() + recvbuf_idx_;
	}
	void recvbuf_remove_n(size_t n) noexcept;
};

class tcp_server
{
private:
	int listenfd_ = -1;
public:
	tcp_server(const tcp_server &) = delete;
	tcp_server(const tcp_server &&) = delete;
	tcp_server & operator=(const tcp_server &) = delete;
	tcp_server & operator=(tcp_server &&) = delete;

	~tcp_server();
	tcp_server() {};
	void open(const char *host, const char *serv, int backlog = BACKLOG);
	explicit tcp_server(const char *host, const char *serv, int backlog = BACKLOG)
	{
		open(host, serv, backlog);
	}
	std::unique_ptr<tcp_socket> accept();
};

#endif /* SRC_TCPSOCKET_H_ */
