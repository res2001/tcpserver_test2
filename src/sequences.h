/*
 * test2.h
 *
 *  Created on: 22 мар. 2019 г.
 *      Author: res
 */

#ifndef SEQUENCES_H_
#define SEQUENCES_H_

#include <cstddef>
#include <vector>
#include <string>

#include "tcpsocket.h"

struct sequence_item
{
	uint64_t start = 0;
	uint64_t step = 0;
	uint64_t cur = 0;
	bool enable = false;
};

#define NUMBER_SEQUENCE			3

class sequences
{
private:
	using seq_type = std::vector<sequence_item>;
	using socket_ptr_type = std::unique_ptr<tcp_socket>;
	static const std::string cmd_seq;
	static const std::string cmd_export;
	static const std::string cmd_exit;
	seq_type seq_;
	socket_ptr_type socket_;
public:
	class bad_sequence {};		// exception class
	sequences() = delete;
	sequences(const sequences &) = delete;
	sequences(const sequences &&) = delete;
	sequences & operator=(const sequences &) = delete;
	sequences & operator=(sequences &&) = delete;

	sequences(socket_ptr_type socket, uint32_t number_of_seq = NUMBER_SEQUENCE);
	bool read_commands();
	void send_sequences();
};

#endif /* SEQUENCES_H_ */
