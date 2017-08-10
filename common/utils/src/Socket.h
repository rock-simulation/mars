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
      SOCKET_FLAG_NOSIGNAL      = 0x40,
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
    unsigned short ntoh(const unsigned short &val);
    unsigned long ntoh(const unsigned long &val);
    unsigned short hton(const unsigned short &val);
    unsigned long hton(const unsigned long &val);


    // class declarations

    class TCPServer {
    public:
      TCPServer();
      ~TCPServer();

      /**
       * \brief Open the server on the given port
       */
      SocketError open(unsigned short port);

      /** \brief Not Implemented yet */
      SocketError hasClient() const;

      /**
       * \brief Accepts an incoming connection.
       * \param c An unconnected TCPConnection which will get connected to the
       *          remote client by this call.
       */
      SocketError acceptConnection(TCPConnection *c) const;

      /**
       * \brief closes the server.
       *        It will no longer listen for incoming connections.
       */
      void close();

      /**
       * \brief Queries whether the server has been opened.
       */
      bool isOpen();

      /** Not Implemented yet */
      SocketError reopen();
      /** Not Implemented Yet */
      void setTimeout(double timeout);
      /** Not Implemented Yet */
      double getTimeout() const;

    private:
      TCPServer(const TCPServer &other);
      TCPServer& operator=(const TCPServer &other);

      TCPBaseSocket *s;
    }; // end of class TCPServer


    class TCPConnection {
    public:
      TCPConnection();
      ~TCPConnection();
      /**
       * \brief Connects to a given server on the given \c port.
       */
      SocketError connectToTCPServer(const std::string &host,
                                     unsigned short port);
      /**
       * \brief Closes the connection.
       * It is safe to call this even if there is no connection established.
       */
      void close();

      /**
       * \brief Queries whether there is an established connection.
       */
      bool isConnected() const;

      /**
       * \brief Sends the entire string.
       * Under the hood this repeatedly calls send until the entire string
       * is transferred.
       * \sa recvAll, send, sendBinary
       */
      SocketError sendAll(const std::string &data);
      /** \copydoc sendAll */
      SocketError sendAll(const char *data, size_t len);

      /**
       * \brief Send the \c data buffer over the connection.
       * This does not guarantee that the entire \c data buffer is transferred.
       * \param data The data buffer to be sent over the connection.
       * \param len The length of the data buffer.
       * \return The number of bytes actually trnsferred.
       * \sa recv, sendAll, sendBinary
       */
      size_t send(const char *data, size_t len);

      /**
       * \brief Receives exactly len bytes and stores them in data.
       * Under the hood this repeatedly calls recv until len bytes are received.
       * \param data A pointer to a buffer that can hold at least len bytes.
       * \param len The number of bytes to receive before returning.
       * \sa sendAll, recv, recvBinary
       */
      SocketError recvAll(char *data, size_t len);

      /**
       * \brief Receive at most max bytes and store them in data
       * This does not guarantee that actually max bytes are received.
       * \param data A pointer to a buffer that can hold at least max bytes.
       * \param max The maximum of bytes to receive before returning.
       * \sa send, recvAll, recvBinary
       */
      size_t recv(char *data, size_t max);

      /**
       * \brief Convenience function to send len bytes of binary data.
       * This will transmit the len bytes pointed to by data in
       * Network-Byte-Order.
       * \sa recvBinary, send, sendAll
       */
      SocketError sendBinary(const void *data, size_t len);

      /**
       * \brief Convenience function to receive len bytes of binary data.
       * This will receive len bytes convert them from Network-Byte-Order
       * to Host-Byte-Order and store them in the buffer pointed to by data.
       * \sa sendBinary, recv, recvAll
       */
      SocketError recvBinary(void *data, size_t len);

      /** Not Implemented Yet */
      void setBlocking(bool blocking);
      /** Not Implemented Yet */
      bool isBlocking() const;
      /** Not Implemented Yet */
      void setTimeout(double timeout);
      /** Not Implemented Yet */
      double getTimeout() const;

      /**
       * \brief Convenience function to send a 32-bit signed integer
       *        in a endian aware manner.
       * \sa sendBinary, recvBinary, sendInt32, recvInt32,
       *     sendUInt32, recvUInt32, sendFloat, recvFloat,
       *     sendDouble, recvDouble
       */
      inline SocketError sendInt32(int32_t val)
      { return sendBinary(&val, 4); }
      /** 
       * \brief Convenience function to receive a 32-bit signed integer
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError recvInt32(int32_t *val)
      { return recvBinary(val, 4); }
      /** 
       * \brief Convenience function to send a 32-bit unsigned integer
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError sendUInt32(uint32_t val)
      { return sendBinary(&val, 4); }
      /** 
       * \brief Convenience function to receive a 32-bit unsigned integer
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError recvUInt32(uint32_t *val)
      { return recvBinary(val, 4); }
      /** 
       * \brief Convenience function to send a single precision float
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError sendFloat(float val)
      { return sendBinary(&val, 4); }
      /** 
       * \brief Convenience function to receive a single precision float
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError recvFloat(float *val)
      { return recvBinary(val, 4); }
      /** 
       * \brief Convenience function to send a double precision float
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
      inline SocketError sendDouble(double val)
      { return sendBinary(&val, 8); }
      /** 
       * \brief Convenience function to receive a double precision float
       *        in a endian aware manner.
       * \details \copydetails sendInt32
       */
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
