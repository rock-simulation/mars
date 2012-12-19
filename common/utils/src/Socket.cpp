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

#include "Socket.h"

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <netdb.h>
#  include <sys/socket.h>
#endif

#ifndef __linux
#  define MSG_NOSIGNAL 0
#endif

#ifdef WIN32
#  ifndef WSAEAGAIN
#    define WSAEAGAIN WSAEWOULDBLOCK
#  endif
#  define CHECK_ERRNO(expected)                 \
  (WSAGetLastError() == WSA ## expected)
#else
#  define CHECK_ERRNO(expected)                 \
  (errno == expected)
#endif


#include <cassert>
#include <cstdio>
#include <errno.h>

#include "misc.h"

namespace mars {
  namespace utils {


    /********************************
     * helper function implementation
     ********************************/
    unsigned short ntoh(const unsigned short &val)
    { return ::ntohs(val); }
    unsigned long ntoh(const unsigned long &val)
    { return ::ntohl(val); }
    unsigned short hton(const unsigned short &val)
    { return ::htons(val); }
    unsigned long hton(const unsigned long &val)
    { return ::htonl(val); }

    bool isBigEndian() {
      union {
        uint32_t i;
        char c[4];
      } test = {0x01020304};
      return test.c[0] == 1;
    }


    /********************************
     * TCPBaseSocket declaration
     ********************************/

    class TCPBaseSocket {
    public:
      TCPBaseSocket();
      ~TCPBaseSocket();
      SocketError init();
      void close();
      SocketError bind(const std::string &addr, unsigned short port);
      SocketError listen(int queueSize);
      SocketError connect(const std::string &host, unsigned short port);
      TCPBaseSocket* accept();
      TCPBaseSocket* connect();
      size_t send(const char *data, size_t len) const;
      size_t recv(char *data, size_t maxLen);
      void setFlags(unsigned int flags);
      int getSocketFamily() const;
      int getSocketType() const;
      int getSocketProtocol() const;
      bool isConnected() const;
      double getTimeout() const;
      void setTimeout(double timeout);

    private:
      unsigned int flags;
      int sock_fd;
      double sock_timeout;

      int internal_select(bool writing, double timeout);
    }; // end of class TCPBaseSocket



    /********************************
     * TCPServer implementation
     ********************************/

    TCPServer::TCPServer()
      : s(new TCPBaseSocket()) {
      s->init();
    }

    TCPServer::~TCPServer() {
      if(s) {
        s->close();
        delete s;
      }
    }

    SocketError TCPServer::open(unsigned short port) {
      SocketError ret;
      if((ret = s->init()) == SOCKET_SUCCESS) {
        if((ret = s->bind("localhost", port)) == SOCKET_SUCCESS) {
          ret = s->listen(3);
        }
      }
      return ret;
    }

    SocketError TCPServer::hasClient() const
    {assert(false);}

    SocketError TCPServer::acceptConnection(TCPConnection *c) const {
      if(c->s) {
        if(c->s->isConnected()) {
          fprintf(stderr,
                  "TCPServer::acceptConnection: client already connected\n");
          return SOCKET_ALREADY_CONNECTED;
        }
        delete c->s;
      }
      c->s = s->accept();
      return (c->s ? SOCKET_SUCCESS : SOCKET_ACCEPT_FAILED);
    }

    void TCPServer::close()
    { if(s) s->close(); }

    bool TCPServer::isOpen()
    { return (s && s->isConnected()); }

    SocketError TCPServer::reopen()
    {assert(false);}

    /* disallow copying */
    TCPServer::TCPServer(const TCPServer &other)
    {assert(false);}

    /* disallow assignment */
    TCPServer& TCPServer::operator=(const TCPServer &other)
    {assert(false);}



    /********************************
     * TCPConnection implementation
     ********************************/

    TCPConnection::TCPConnection()
      : s(NULL) {
    }

    TCPConnection::~TCPConnection() {
      if(s) {
        s->close();
        delete s;
      }
    }

    SocketError TCPConnection::connectToTCPServer(const std::string &host,
                                                  unsigned short port) {
      if(s) {
        delete s;
      }
      s = new TCPBaseSocket;
      SocketError ret;
      if((ret = s->init()) == SOCKET_SUCCESS) {
        ret = s->connect(host, port);
      }
      return ret;
    }

    void TCPConnection::close()
    { if(s) s->close(); }

    bool TCPConnection::isConnected() const
    { return s && s->isConnected(); }

    SocketError TCPConnection::sendAll(const std::string &data)
    { return sendAll(data.data(), data.size()); }

    SocketError TCPConnection::sendAll(const char *data, size_t len) {
      if(!s)
        return SOCKET_INVALID_SOCKET;
      size_t offset = 0;
      while(len > 0) {
        size_t bytesSent = send(data + offset, len);
        if(!bytesSent)
          return SOCKET_CONNECTION_BROKEN;
        offset += bytesSent;
        len -= bytesSent;
      }
      return SOCKET_SUCCESS;
    }

    size_t TCPConnection::send(const char *data, size_t len) {
      if(s) {
        return s->send(data, len);
      }
      return 0;
    }

    SocketError TCPConnection::recvAll(char *data, size_t len) {
      if(!s)
        return SOCKET_INVALID_SOCKET;
      while(len > 0) {
        size_t bytesRead = s->recv(data, len);
        if(!bytesRead)
          return SOCKET_CONNECTION_BROKEN;
        data += bytesRead;
        len -= bytesRead;
      }
      return SOCKET_SUCCESS;
    }

    size_t TCPConnection::recv(char *data, size_t max) {
      if(s)
        return s->recv(data, max);
      return 0;
    }

    SocketError TCPConnection::sendBinary(const void *data, size_t len) {
      if(isBigEndian()) {
        return sendAll(static_cast<const char*>(data), len);
      } else {
        char *buffer = new char[len];
        for(size_t i=0, j=len-1; i < len; ++i, --j) {
          buffer[i] = static_cast<const char*>(data)[j];
        }
        SocketError ret = sendAll(buffer, len);
        delete[] buffer;
        return ret;
      }
    }

    SocketError TCPConnection::recvBinary(void *data, size_t len) {
      if(isBigEndian()) {
        return recvAll(static_cast<char*>(data), len);
      } else {
        char *buffer = new char[len];
        SocketError ret = recvAll(buffer, len);
        for(size_t i=0, j=len-1; i < len; ++i, --j) {
          ((unsigned char*)data)[i] = ((unsigned char*)buffer)[j];
        }
        delete[] buffer;
        return ret;
      }
    }

    void TCPConnection::setBlocking(bool blocking)
    {assert(false);}
    bool TCPConnection::isBlocking() const
    { return true; }

    void TCPConnection::setTimeout(double timeout)
    { if(s) s->setTimeout(timeout); }

    double TCPConnection::getTimeout() const
    { return (s ? s->getTimeout() : 0.); }

    /* disallow copying */
    TCPConnection::TCPConnection(const TCPConnection &other)
    {assert(false);}

    /* disallow assignment */
    TCPConnection& TCPConnection::operator=(const TCPConnection &other)
    {assert(false);}



    /********************************
     * TCPBaseSocket implementation
     ********************************/

    TCPBaseSocket::TCPBaseSocket()
      : flags(SOCKET_FLAG_REUSEADDR)
      , sock_fd(INVALID_SOCKET) {

      sock_timeout = -1;
#ifdef WIN32
      WSADATA wsaData;
      WORD wVersionRequested = MAKEWORD(2, 2);
      int result = WSAStartup(wVersionRequested, &wsaData);
      if (result != 0) {
        fprintf(stderr, "TCPBaseSocket::TCPBaseSocket: Fehler beim Initialisieren von Winsock: %d\n", result);
        return;
      }
#endif
    }

    TCPBaseSocket::~TCPBaseSocket() {
      close();
#ifdef WIN32
      WSACleanup();
#endif
    }

    SocketError TCPBaseSocket::init() {
      sock_fd = socket(getSocketFamily(), getSocketType(), getSocketProtocol());
      sock_timeout = defaultTimeout;
      setFlags(flags);
      if(sock_fd == -1) {
        sock_fd = INVALID_SOCKET;
        return SOCKET_CREATE_FAILED;
      }
      return SOCKET_SUCCESS;
    }

    void TCPBaseSocket::close() {
      if(isConnected()) {
#ifdef WIN32
        closesocket(sock_fd);
#else
        ::close(sock_fd);
#endif
      }
      sock_fd = INVALID_SOCKET;
    }

    SocketError TCPBaseSocket::bind(const std::string &host,
                                    unsigned short port) {
      if(sock_fd == INVALID_SOCKET) {
        fprintf(stderr, "TCPBaseSocket::bind: invalid socket\n");
        return SOCKET_INVALID_SOCKET;
      }
      struct hostent *hp = gethostbyname( host.c_str() );
      if(hp == NULL) {
        fprintf(stderr, "TCPBaseSocket::bind: gethostbyname failed\n");
        return SOCKET_HOST_NOT_FOUND;
      }

      struct sockaddr_in sa;
      memset(&sa, 0, sizeof(struct sockaddr_in));
      sa.sin_family = hp->h_addrtype;
      sa.sin_port = hton(port);

      if(::bind(sock_fd, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        close();
        fprintf(stderr, "TCPBaseSocket::bind: bind failed\n");
        return SOCKET_BIND_FAILED;
      }
      return SOCKET_SUCCESS;
    }

    SocketError TCPBaseSocket::listen(int queueSize) {
      if(::listen(sock_fd, queueSize) != 0) {
        fprintf(stderr, "errrror 4\n");
        return SOCKET_LISTEN_FAILED;
      }
      return SOCKET_SUCCESS;
    }

    TCPBaseSocket* TCPBaseSocket::accept() {
      if(sock_fd == INVALID_SOCKET) {
        fprintf(stderr, "TCPBaseSocket::accept: invalid socket\n");
        return NULL;
      }
      long now = 0, deadline = 0;
      double timeout = sock_timeout;
      bool has_timeout = sock_timeout > 0.;
      if(has_timeout) {
        now = mars::utils::getTime();
        deadline = now + (long)(sock_timeout * 1000.);
      }
      while(true) {
        errno = 0;
        int selected = internal_select(0, timeout);
        if(selected == 1) {
          TCPBaseSocket *client = new TCPBaseSocket;
          client->sock_fd = ::accept(sock_fd, NULL, NULL);
          return client;
        } else if(selected == 0) {
          fprintf(stderr, "TCPBaseSocket::accept: select failed\n");
          return NULL;
        }
        if(!has_timeout || (!CHECK_ERRNO(EWOULDBLOCK) && !CHECK_ERRNO(EAGAIN)))
          break;
        timeout = mars::utils::getTimeDiff(now) - deadline;
        now = mars::utils::getTime();
      }
      return NULL;
    }

    SocketError TCPBaseSocket::connect(const std::string &host,
                                       unsigned short port) {
      struct hostent *h;
      h = gethostbyname(host.c_str());
      if(!h) {
        fprintf(stderr, "TCPBaseSocket::connect: Host \"%s\" not found\n",
                host.c_str());
        return SOCKET_HOST_NOT_FOUND;
      }
      struct sockaddr_in servAddr;
      servAddr.sin_family = h->h_addrtype;
      memcpy((char*)&servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
      servAddr.sin_port = hton(port);
      int result = ::connect(sock_fd, (struct sockaddr*)&servAddr,
                             sizeof(servAddr));
      if(result != 0) {
        close();
        fprintf(stderr, "TCPBaseSocket::connect: connect failed to %s:%hu\n",
                host.c_str(), port);
        return SOCKET_CONNECTION_FAILED;
      }
      return SOCKET_SUCCESS;
    }

    size_t TCPBaseSocket::send(const char *data, size_t len) const {
      // MSG_NOSIGNAL prevents send from raising SIGPIPE on Linux
      return ::send(sock_fd, data, len, flags | MSG_NOSIGNAL);
    }

    size_t TCPBaseSocket::recv(char *data, size_t maxLen) {
      return ::recv(sock_fd, data, maxLen, flags);
    }

    int TCPBaseSocket::getSocketFamily() const
    { return AF_INET; }

    int TCPBaseSocket::getSocketType() const
    { return SOCK_STREAM; }

    int TCPBaseSocket::getSocketProtocol() const
    { return 0; }

    void TCPBaseSocket::setFlags(unsigned int flags) {
      this->flags = flags;
      if(!isConnected()) {
        return;
      }
      int val;
      val = int(flags & SOCKET_FLAG_REUSEADDR);
      setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                 (const char*)&val, sizeof(val));
      val = int(flags & SOCKET_FLAG_BROADCAST);
      setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST,
                 (const char*)&val, sizeof(val));
      val = int(flags & SOCKET_FLAG_KEEPALIVE);
      setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE,
                 (const char*)&val, sizeof(val));
      val = int(flags & SOCKET_FLAG_NODELAY);
      setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY,
                 (const char*)&val, sizeof(val));
      setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY,
                 (const char*)&val, sizeof(val));
#ifdef __APPLE__
      // prevent send() from raising SIGPIPE
      int set = 1;
      setsockopt(sock_fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&set, sizeof(int));
#endif
    }

    bool TCPBaseSocket::isConnected() const
    { return sock_fd != INVALID_SOCKET; }

    double TCPBaseSocket::getTimeout() const
    { return sock_timeout; }

    void TCPBaseSocket::setTimeout(double timeout)
    { sock_timeout = timeout; }

    int TCPBaseSocket::internal_select(bool writing, double timeout) {
      if(sock_timeout <= 0.)
        return 1;
      if(!isConnected())
        return 0;
      if(timeout < 0.)
        return 1;
      struct timeval timeoutStruct;
      timeoutStruct.tv_sec = (int)timeout;
      timeoutStruct.tv_usec = (int)((timeout - timeoutStruct.tv_sec) * 1e6);
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(sock_fd, &fds);
      int n;
      if(writing)
        n = select((int)(sock_fd+1), NULL, &fds, NULL, &timeoutStruct);
      else
        n = select((int)(sock_fd+1), &fds, NULL, NULL, &timeoutStruct);
      if(n < 0)
        return -1;
      if(n == 0)
        return 1;
      return 0;
    }

  } // end of namespace utils
} // end of namespace mars
