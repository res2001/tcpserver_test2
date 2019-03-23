/*
 * sequences.cpp
 *
 *  Created on: 22 мар. 2019 г.
 *      Author: res
 */
#include <utility>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cinttypes>
#include <cstring>

#include "sequences.h"

#define SENDSOCKET(str)	socket_->sendall(str, std::strlen(str));

const std::string sequences::cmd_seq = "seq";
const std::string sequences::cmd_export = "export seq";
const std::string sequences::cmd_exit = "exit";

sequences::sequences(socket_ptr_type socket, uint32_t number_of_seq)
	: seq_(number_of_seq), socket_(std::move(socket))
{
	if(number_of_seq == 0)
	{
		throw bad_sequence();
	}
}

static bool compare_char_for_equality_icase(const char & c1, const char & c2)
{
	if (c1 == c2)
		return true;
	else if (std::toupper(c1) == std::toupper(c2))
		return true;
	return false;
}

bool sequences::read_commands()
{
	std::string line;
	while(true)
	{
		line = socket_->read_line();

		if(line.empty())
		{
			continue;
		}

		if (line.length() > cmd_seq.length() &&
			std::equal(line.cbegin(), line.cbegin() + cmd_seq.length(),
			cmd_seq.cbegin(), cmd_seq.cend(),
			compare_char_for_equality_icase))
		{
			uint32_t num_seq = 0;
			uint64_t start = 0;
			uint64_t step = 0;
			int sn = std::sscanf(line.c_str() + cmd_seq.length(), "%u%" SCNu64 "%" SCNu64, &num_seq, &start, &step);
			if(sn < 3)
			{
				SENDSOCKET("Bad sequence settings.\n");
				continue;
			}
			if(num_seq > seq_.size())
			{
				SENDSOCKET("Bad sequence number.\n");
				continue;
			}
			--num_seq;
			if(start == 0 || step == 0)
			{
				seq_[num_seq].enable = false;
				SENDSOCKET("Sequence disabled.\n");
			} else {
				seq_[num_seq].cur = seq_[num_seq].start = start;
				seq_[num_seq].step = step;
				seq_[num_seq].enable = true;
				SENDSOCKET("Ok.\n");
			}
		}
		else if(std::equal(line.cbegin(), line.cend(),
				cmd_export.cbegin(), cmd_export.cend(),
				compare_char_for_equality_icase))
		{
			return true;
		}
		else if(std::equal(line.cbegin(), line.cend(),
				cmd_exit.cbegin(), cmd_exit.cend(),
				compare_char_for_equality_icase))
		{
			return false;
		} else {
			SENDSOCKET("Unknown command.\n");
		}
	}
}

void sequences::send_sequences()
{
	while(true)
	{
		std::string line("");
		for(auto seq_it = seq_.begin(); seq_it != seq_.end(); ++seq_it)
		{
			if(seq_it->enable)
			{
				line += std::to_string(seq_it->cur) + "\t";
				const uint64_t cur = seq_it->cur + seq_it->step;
				if(cur < seq_it->cur)
				{
					seq_it->cur = seq_it->start;
				} else {
					seq_it->cur = cur;
				}
			}
		}
		if(line.length() == 0)
		{
			return;
		}
		line.back() = '\n';
		socket_->sendall(line.c_str(), line.length());
	}
}
