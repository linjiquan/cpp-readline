#pragma once

// C/C++
#include <string>

namespace xma {

// Copied from linux kernel
#define barrier() __asm__ __volatile__("" ::: "memory")

class Fifo
{
public:
  Fifo(uint32_t size): size_(size) 
  {
	if ((size_) & ((size_) - 1)) {
		throw std::runtime_error("Invalid fifo length");
	}
	
	data_ = new (sizeof(void *) * size_);
  }
  ~Fifo() {
	  delete data_;
  }

	uint32_t GetLen() 
	{
		uint32_t out = out_;

		barrier();

		return in_ - out;
	}


	int TryPut(void * data)
	{
		if ((size_ - in_ + out_) == 0) 
		{
			fails_++;
			return -1;
		}

		barrier ();

		data_[in_ & (size_ - 1)] = data;

		barrier ();

		in_++;
		puts_++;

		return 0;
	}

	//no fail
	int Put(void * data)
	{
		while ((size_ - in_ + out_) == 0) {
			barrier ();
		}

		fifo->pkts[fifo->in & (fifo->size - 1)] = msg;

		barrier ();

		in_++;
		puts_++;
	}


	uint32_t Get(void * buffer, uint32_t len)
	{
		uint32_t l;

		len = std::min (len, in_ - out_);

		barrier ();

		l = std::min (len, size_ - (out_ & (size_ - 1)));

		memcpy (buffer, data_ + (out_ & (size_ - 1)), l * sizeof (void *));

		memcpy (buffer + l, data_, (len - l) * sizeof (void *));


		/*
		 * Ensure that we remove the bytes from the fifo -before-
		 * we update the fifo->out index.
		 */
		barrier ();


		out_ += len;
		gets += len;

		return len;
	}

private:
	std::string name_;
	uint32_t size_;
	uint32_t in_;
	uint32_t out_;

	void *data_;

	uint64_t fails_;
	uint64_t puts_;
	uint64_t gets_;
};

}

