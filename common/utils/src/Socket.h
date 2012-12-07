/**
 *  Copyright 2012, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef MARS_SOCKET_H
#define MARS_SOCKET_H

#include <cstring> // for size_t
#include <string>
#include <stdint.h>

namespace mars {
  namespace utils {

    // Forward declarations

    class TCPBaseSocket;
    class TCPServer;
    class TCPConnection;


    // consts and enums

    const int INVALID_SOCKET = -1;
    const double defaultTimeout = -1.;

    enum SocketFlags {
      // default is TCP in BLOCKING mode
      SOCKET_FLAG_UDP           = 0x01,
      SOCKET_FLAG_NONBLOCKING   = 0x02,
      SOCKET_FLAG_REUSEADDR     = 0x04,
      SOCKET_FLAG_NODELAY       = 0x08,
      SOCKET_FLAG_BROADCAST     = 0x10,
      SOCKET_FLAG_KEEPALIVE     = 0x20,
    };


    enum SocketError {
      SOCKET_SUCCESS            = 0,
      SOCKET_HOST_NOT_FOUND     = -1,
      SOCKET_CREATE_FAILED      = -2,
      SOCKET_BIND_FAILED        = -3,
      SOCKET_LISTEN_FAILED      = -4,
      SOCKET_ACCEPT_FAILED      = -5,
      SOCKET_SELECT_FAILED      = -6,
      SOCKET_TIMEOUT            = -7,
      SOCKET_CONNECTION_FAILED  = -8,
      SOCKET_CONNECTION_BROKEN  = -9,
      SOCKET_CONNECTION_CLOSED  = -10,
      SOCKET_INVALID_SOCKET     = -11,
      SOCKET_ALREADY_CONNECTED  = -12,
      SOCKET_WOULD_BLOCK        = -126,
      SOCKET_UNSPECIFIED_ERROR  = -127
    };

    // helper functions

    bool isBigEndian();
    template<typename T>
    T ntoh(const T &val);
    template<typename T>
    T hton(const T &val);


    // class declarations

    class TCPServer {
    public:
      TCPServer();
      ~TCPServer();
      SocketError open(unsigned short port);
      SocketError hasClient() const;
      SocketError acceptConnection(TCPConnection *c) const;
      void close();
      bool isOpen();
      SocketError reopen();
    private:
      TCPServer(const TCPServer &other);
      TCPServer& operator=(const TCPServer &other);

      TCPBaseSocket *s;
    }; // end of class TCPServer

    class TCPConnection {
    public:
      TCPConnection();
      ~TCPConnection();
      SocketError connectToTCPServer(const std::string &host,
                                     unsigned short port);
      void close();
      bool isConnected() const;
      SocketError sendAll(const std::string &data);
      SocketError sendAll(const char *data, size_t len);
      size_t send(const char *data, size_t len);
      SocketError recvAll(char *data, size_t len);
      size_t recv(char *data, size_t max);
      SocketError sendBinary(const void *data, size_t len);
      SocketError recvBinary(void *data, size_t len);

      void setBlocking(bool blocking);
      bool isBlocking() const;
      void setTimeout(double timeout);
      double getTimeout() const;

      inline SocketError sendInt32(int32_t val)
      { return sendBinary(&val, 4); }
      inline SocketError recvInt32(int32_t *val)
      { return recvBinary(val, 4); }
      inline SocketError sendUInt32(uint32_t val)
      { return sendBinary(&val, 4); }
      inline SocketError recvUInt32(uint32_t *val)
      { return recvBinary(val, 4); }
      inline SocketError sendFloat(float val)
      { return sendBinary(&val, 4); }
      inline SocketError recvFloat(float *val)
      { return recvBinary(val, 4); }
      inline SocketError sendDouble(double val)
      { return sendBinary(&val, 8); }
      inline SocketError recvDouble(double *val)
      { return recvBinary(val, 8); }


    private:
      TCPConnection(const TCPConnection &other);
      TCPConnection& operator=(const TCPConnection &other);

      TCPBaseSocket *s;
      friend SocketError TCPServer::acceptConnection(TCPConnection *c) const;
    }; // end of class TCPConnection

  } // end of namespace utils
} // end of namespace mars

#endif /* MARS_SOCKET_H */
