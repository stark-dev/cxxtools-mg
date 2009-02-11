/*
 * Copyright (C) 2006 by Marc Boris Duerner, Tommi Maekitalo
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * As a special exception, you may use this file as part of a free
 * software library without restriction. Specifically, if other files
 * instantiate templates or use macros or inline functions from this
 * file, or you compile this file and link it with other files to
 * produce an executable, this file does not by itself cause the
 * resulting executable to be covered by the GNU General Public
 * License. This exception does not however invalidate any other
 * reasons why the executable file might be covered by the GNU Library
 * General Public License.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CXXTOOLS_NET_TcpServerSocketImpl_H
#define CXXTOOLS_NET_TcpServerSocketImpl_H

#include "selectableimpl.h"
#include <cxxtools/signal.h>
#include <string>

namespace cxxtools {

class SelectorBase;

namespace net {

  class TcpServerSocket;

  class TcpServerSocketImpl : public SelectableImpl
  {
    private:
      TcpServerSocket& _server;
      struct sockaddr_storage servaddr;
      int m_fd;
      pollfd* _pfd;

    public:
      TcpServerSocketImpl(TcpServerSocket& server);

      void create(int domain, int type, int protocol);

      void close();

      void listen(const std::string& ipaddr, unsigned short int port, int backlog = 5);

      const struct sockaddr_storage& getAddr() const
      { return servaddr; }

      int fd() const
      { return m_fd; }

      bool wait(std::size_t msecs);

      void attach(SelectorBase& s);

      void detach(SelectorBase& s);

      // implementation using poll
      std::size_t pollSize() const;

      // implementation using poll
      std::size_t initializePoll(pollfd* pfd, std::size_t pollSize);

      // implementation using poll
      bool checkPollEvent();
  };

} // namespace net

} // namespace cxxtools

#endif // CXXTOOLS_NET_TCPSTREAM_H
