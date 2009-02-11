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

#ifndef CXXTOOLS_NET_TcpServerSocket_H
#define CXXTOOLS_NET_TcpServerSocket_H

#include <cxxtools/api.h>
#include <cxxtools/selectable.h>
#include <cxxtools/signal.h>
#include <string>

namespace cxxtools {

namespace net {

  class CXXTOOLS_API TcpServerSocket : public Selectable
  {
    class TcpServerSocketImpl* _impl;

    public:
      TcpServerSocket();

      /** @brief Creates a server socket and listens on an address
      */
      TcpServerSocket(const std::string& ipaddr, unsigned short int port, int backlog = 5);

      ~TcpServerSocket();

      void listen(const std::string& ipaddr, unsigned short int port, int backlog = 5);

      /// @brief TODO
      const struct sockaddr_storage& getAddr() const;

      /// @brief TODO
      int getFd() const;

      // inherit doc
      virtual SelectableImpl& simpl();

      TcpServerSocketImpl& impl() const;

      Signal<TcpServerSocket&> connectionPending;

    protected:
      // inherit doc
      virtual void onClose();

      // inherit doc
      virtual bool onWait(std::size_t msecs);

      // inherit doc
      virtual void onAttach(SelectorBase&);

      // inherit doc
      virtual void onDetach(SelectorBase&);
  };

} // namespace net

} // namespace cxxtools

#endif // CXXTOOLS_NET_TCPSTREAM_H
