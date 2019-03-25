/*
 * socket_exception.h
 *
 *  Created on: 20 мар. 2019 г.
 *      Author: res
 */

#ifndef SRC_SOCKET_EXCEPTION_H_
#define SRC_SOCKET_EXCEPTION_H_

#include <exception>

#define SOCKET_EXCEPTION_MAP(XX)								\
	XX(1, BADSOCKET, "Bad socket.")								\
	XX(2, REMOTE_SHUTDOWN, "Remote host closed connection.")	\
	XX(3, BUFFER_OVERFLOW, "Buffer overflow.")

#define XX(num, name, helpstr)		SE_ ## name = num,
enum socket_exception_e {
	SOCKET_EXCEPTION_MAP(XX)
};
#undef XX

extern const char * socket_exception_str(enum socket_exception_e sock_excp_id);

class socket_exception : public std::exception {
public:
	int nerr_;
	const char * serr_;

	socket_exception() = delete;
	socket_exception(int nerr, const char * serr) noexcept
		: nerr_(nerr), serr_(serr) {};
	socket_exception(const socket_exception& se) noexcept
		: nerr_(se.nerr_), serr_(se.serr_) {};
	socket_exception& operator=(const socket_exception& se) noexcept
	{
		nerr_ = se.nerr_;
		serr_ = se.serr_;
		return *this;
	}
    virtual ~socket_exception() = default;
    virtual const char* what() const noexcept
    {
    	return serr_;
    }
};

#endif /* SRC_SOCKET_EXCEPTION_H_ */
