//
// Created by sebastian on 31.05.19.
//

#include "SerialImpl.h"

SerialImpl::SerialImpl(std::string device, unsigned int baudrate) : mDevice{ device }, mBaudrate{ baudrate }, mPort{ mIOService } {
	try {
		mPort.open(device);
	}
	catch (std::runtime_error& e) {
		mErrorMessage = e.what();
		return;
	}
	mOpen = true;

	using serial = asio::serial_port_base;
	serial::baud_rate baud(baudrate);
	serial::character_size bitsize(8);
	serial::parity parity(serial::parity::none);
	serial::stop_bits stopbits(serial::stop_bits::one);
	serial::flow_control flowcontrol(serial::flow_control::none);

    mPort.set_option(baud);
    mPort.set_option(bitsize);
    mPort.set_option(parity);
    mPort.set_option(stopbits);
	mPort.set_option(flowcontrol);
}

SerialImpl::~SerialImpl() {
    mIOService.stop();
	if (mOpen) {
		mPort.cancel();
		mPort.close();
	}
}

void SerialImpl::writeData(std::byte data) {
	if (mOpen) {
		asio::write(mPort, asio::buffer({ data }, 1));
		mIOService.poll();
	}
}

void SerialImpl::writeData(std::vector<std::byte> data) {
	if (mOpen) {
		asio::write(mPort, asio::buffer(data, data.size()));
		mIOService.poll();
	}
}

std::optional<std::string> SerialImpl::reciveByte() {
	if (mOpen) {
		mIOService.poll();
		asio::streambuf readbuf;
		asio::read(mPort, readbuf, asio::transfer_exactly(1));
		return std::string{ asio::buffers_begin(readbuf.data()), asio::buffers_end(readbuf.data()) };
	}
	return std::nullopt;
}

std::vector<std::byte> SerialImpl::reciveBytes() {
	if (mOpen) {
		mIOService.poll();
		return std::vector<std::byte>();
	}
	return {};
}

bool SerialImpl::isOpen() const
{
	return mOpen;
}

std::optional<std::string> const& SerialImpl::errorMessage() const
{
	return mErrorMessage;
}


