/*
 * tcpsocket.cpp
 *
 *  Created on: 20 мар. 2019 г.
 *      Author: res
 */
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <iostream>
#include <cassert>

#include "tcpsocket.h"

void tcp_server::open(const char *host, const char *serv, int backlog)
{
	struct addrinfo *ressave = nullptr;
	struct addrinfo	*res;
	struct addrinfo	hints;
	int ret;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_flags		= AI_PASSIVE;
	hints.ai_family		= AF_UNSPEC;
	hints.ai_socktype	= SOCK_STREAM;

	if ( (ret = getaddrinfo(host, serv, &hints, &res)) != 0)
	{
		throw socket_exception(ret, gai_strerror(ret));
	}

	ressave = res;

	do {
		if(listenfd_ != -1)
		{
			close(listenfd_);
		}

		if( (listenfd_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
		{
			ret = errno;
			continue;
		}

		const int on = 1;
		setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

		if(bind(listenfd_, res->ai_addr, res->ai_addrlen) < 0)
		{
			ret = errno;
			continue;
		}

		assert(res->ai_addrlen > 0 && res->ai_addrlen <= sizeof(struct sockaddr_in6));

		if(listen(listenfd_, backlog) == 0)
		{
			break;
		}

		ret = errno;

	} while ( (res = res->ai_next) != nullptr);

	freeaddrinfo(ressave);

	if (res == nullptr)
	{
		if(listenfd_ != -1)
		{
			close(listenfd_);
		}
		throw socket_exception(ret, strerror(ret));
	}
}

tcp_server::~tcp_server()
{
	if(listenfd_ != -1)
	{
		close(listenfd_);
	}
}

std::unique_ptr<tcp_socket> tcp_server::accept()
{
	assert(listenfd_ != -1);
	struct sockaddr_in6	cliaddr;
	socklen_t cliaddrlen;
	int connfd;
	std::memset(& cliaddr, 0, sizeof(cliaddr));
	do {
		cliaddrlen = sizeof(cliaddr);
		if( (connfd = ::accept(listenfd_, reinterpret_cast<struct sockaddr*>(& cliaddr), & cliaddrlen)) < 0)
		{
			if(errno == ECONNABORTED)
			{
				continue;
			}
			throw socket_exception(errno, strerror(errno));
		}
	} while(connfd < 0);
	return std::make_unique<tcp_socket>(connfd, reinterpret_cast<struct sockaddr *>(& cliaddr), cliaddrlen);
}

tcp_socket::tcp_socket(int fd, struct sockaddr * addr, socklen_t addrlen)
//	: recvbuf_(RECVBUF_SIZE, 0)
{
	if(fd <= 0 || addr == nullptr || addrlen == 0 || addrlen > sizeof(struct sockaddr_in6))
		throw socket_exception(SE_BADSOCKET, socket_exception_str(SE_BADSOCKET));
//	const int on = 1;
//	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));

	fd_ = fd;
	addrlen_ = addrlen;
	std::memcpy(&addr_, addr, addrlen);
}

tcp_socket::~tcp_socket()
{
	shutdown(fd_, SHUT_RDWR);
	close(fd_);
}

void tcp_socket::sendall(const void * data, size_t len)
{
	size_t nleft = len;
	ssize_t nwritten;
	const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data);

	while (nleft > 0)
	{
		nwritten = ::send(fd_, ptr, nleft, MSG_NOSIGNAL);

		if (nwritten < 0 && errno == EINTR)
		{
			nwritten = 0;		/* and call write() again */
		} else if(nwritten < 0) {
			throw socket_exception(errno, strerror(errno));
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
}

void tcp_socket::recvbuf_remove_n(size_t n) noexcept
{
	assert(n <= recvbuf_cnt_);
	recvbuf_cnt_ -= n;
	if(recvbuf_cnt_ == 0)
	{
		recvbuf_idx_ = 0;
	} else {
		recvbuf_idx_ += n;
	}
}

size_t tcp_socket::recv(void * buf, size_t len)
{
	if(!recvbuf_is_empty())
	{
		assert(recvbuf_.data() != buf);
		size_t copy_cnt = recvbuf_cnt_ < len ? recvbuf_cnt_ : len;
		std::memcpy(buf, recvbuf_get_ptr(), copy_cnt);
		recvbuf_remove_n(copy_cnt);
		return copy_cnt;
	}
	while(1)
	{
		ssize_t recv_cnt = ::recv(fd_, buf, len, 0);
		if(recv_cnt < 0)
		{
			if(errno == EINTR)
			{
				continue;
			}
			throw socket_exception(errno, strerror(errno));
		} else if(recv_cnt == 0) {
			throw socket_exception(SE_REMOTE_SHUTDOWN, socket_exception_str(SE_REMOTE_SHUTDOWN));
		}
		return (size_t)recv_cnt;
	}
}

std::string tcp_socket::read_line()
{
	std::string str = "";
	size_t copy_cnt;
	bool is_ret = false;
	str.reserve(READ_LINE_RESERV_STRING);
	while(true)
	{
		if(recvbuf_is_empty())
		{
			recvbuf_cnt_ = recv(recvbuf_.data(), recvbuf_.size());
		}
		recvbuf_type::value_type *recvdata_ptr = recvbuf_get_ptr();
//		std::clog << "\nrecv bufer data:\n" << std::string(recvdata_ptr, recvbuf_cnt_) << std::endl;
		for(copy_cnt = 0; copy_cnt < recvbuf_cnt_; ++copy_cnt)
		{
			if(recvdata_ptr[copy_cnt] == '\n')
			{
				++copy_cnt;
				is_ret = true;
				break;
			}
		}
//		std::clog << "read cnt: " << recvbuf_cnt_ << "\tcopy cnt: " << copy_cnt << std::endl;
		if(copy_cnt)
		{
			str += std::string(recvdata_ptr, copy_cnt);
			recvbuf_remove_n(copy_cnt);
		}
		if(is_ret)
		{
			str.pop_back();
			if(str.back() == '\r')
			{
				str.pop_back();
			}
//			std::clog << "read str: " << str << std::endl << std::endl;
			return str;
		}
	}
}
