/*
 *   Copyright (C) 2002-2004,2007-2011,2013,2014-2017,2019 by Jonathan Naylor G4KLX
 *   Copyright (C) 1999-2001 by Thomas Sailor HB9JNX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "SerialController.h"
#include "Log.h"

#include <cstring>
#include <cassert>

#include <sys/types.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

CSerialController::CSerialController(const std::string& device, unsigned int speed, bool assertRTS) :
m_device(device),
m_speed(speed),
m_assertRTS(assertRTS),
m_fd(-1)
{
	assert(!device.empty());
}

CSerialController::~CSerialController()
{
}

bool CSerialController::open()
{
	assert(m_fd == -1);

	m_fd = ::open(m_device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY, 0);
	if (m_fd < 0) {
		LogError("Cannot open device - %s", m_device.c_str());
		return false;
	}

	if (::isatty(m_fd)) {
		termios termios;
		if (::tcgetattr(m_fd, &termios) < 0) {
			LogError("Cannot get the attributes for %s", m_device.c_str());
			::close(m_fd);
			return false;
		}

		termios.c_iflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK);
		termios.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL);
		termios.c_iflag &= ~(IXON | IXOFF | IXANY);
		termios.c_oflag &= ~(OPOST);
		termios.c_cflag &= ~(CSIZE | CSTOPB | PARENB | CRTSCTS);
		termios.c_cflag |=  (CS8 | CLOCAL | CREAD);
		termios.c_lflag &= ~(ISIG | ICANON | IEXTEN);
		termios.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
		termios.c_cc[VMIN]  = 0;
		termios.c_cc[VTIME] = 10;

		switch (m_speed) {
			case 1200U:
				::cfsetospeed(&termios, B1200);
				::cfsetispeed(&termios, B1200);
				break;
			case 2400U:
				::cfsetospeed(&termios, B2400);
				::cfsetispeed(&termios, B2400);
				break;
			case 4800U:
				::cfsetospeed(&termios, B4800);
				::cfsetispeed(&termios, B4800);
				break;
			case 9600U:
				::cfsetospeed(&termios, B9600);
				::cfsetispeed(&termios, B9600);
				break;
			case 19200U:
				::cfsetospeed(&termios, B19200);
				::cfsetispeed(&termios, B19200);
				break;
			case 38400U:
				::cfsetospeed(&termios, B38400);
				::cfsetispeed(&termios, B38400);
				break;
			case 115200U:
				::cfsetospeed(&termios, B115200);
				::cfsetispeed(&termios, B115200);
				break;
			case 230400U:
				::cfsetospeed(&termios, B230400);
				::cfsetispeed(&termios, B230400);
				break;
			default:
				LogError("Unsupported serial port speed - %d", int(m_speed));
				::close(m_fd);
				return false;
		}

		if (::tcsetattr(m_fd, TCSANOW, &termios) < 0) {
			LogError("Cannot set the attributes for %s", m_device.c_str());
			::close(m_fd);
			return false;
		}

		if (m_assertRTS) {
			unsigned int y;
			if (::ioctl(m_fd, TIOCMGET, &y) < 0) {
				LogError("Cannot get the control attributes for %s", m_device.c_str());
				::close(m_fd);
				return false;
			}

			y |= TIOCM_RTS;

			if (::ioctl(m_fd, TIOCMSET, &y) < 0) {
				LogError("Cannot set the control attributes for %s", m_device.c_str());
				::close(m_fd);
				return false;
			}
		}
	}

	return true;
}

int CSerialController::read(unsigned char* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(m_fd != -1);

	if (length == 0U)
		return 0;

	unsigned int offset = 0U;

	while (offset < length) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(m_fd, &fds);
		int n;
		if (offset == 0U) {
			struct timeval tv;
			tv.tv_sec  = 0;
			tv.tv_usec = 0;
			n = ::select(m_fd + 1, &fds, NULL, NULL, &tv);
			if (n == 0)
				return 0;
		} else {
			n = ::select(m_fd + 1, &fds, NULL, NULL, NULL);
		}

		if (n < 0) {
			LogError("Error from select(), errno=%d", errno);
			return -1;
		}

		if (n > 0) {
			ssize_t len = ::read(m_fd, buffer + offset, length - offset);
			if (len < 0) {
				if (errno != EAGAIN) {
					LogError("Error from read(), errno=%d", errno);
					return -1;
				}
			}

			if (len > 0)
				offset += len;
		}
	}

	return length;
}

int CSerialController::write(const unsigned char* buffer, unsigned int length)
{
	assert(buffer != NULL);
	assert(m_fd != -1);

	if (length == 0U)
		return 0;

	unsigned int ptr = 0U;
	while (ptr < length) {
		ssize_t n = 0U;
		n = ::write(m_fd, buffer + ptr, length - ptr);
		if (n < 0) {
			if (errno != EAGAIN) {
				LogError("Error returned from write(), errno=%d", errno);
				return -1;
			}
		}

		if (n > 0)
			ptr += n;
	}

	return length;
}

void CSerialController::close()
{
	assert(m_fd != -1);

	::close(m_fd);
	m_fd = -1;
}

