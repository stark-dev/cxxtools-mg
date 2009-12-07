/*
 * Copyright (C) 2009 by Marc Boris Duerner, Tommi Maekitalo
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

#include <cxxtools/http/responder.h>
#include <cxxtools/http/reply.h>

namespace cxxtools {

namespace http {

// Responder
//

void Responder::beginRequest(std::istream& in, Request& request)
{
}

std::size_t Responder::readBody(std::istream& in)
{
    std::streambuf* sb = in.rdbuf();

    std::size_t ret = 0;
    while (sb->in_avail() > 0)
    {
        sb->sbumpc();
        ++ret;
    }

    return ret;
}

void Responder::replyError(std::ostream& out, Request& request, Reply& reply, const std::exception& ex)
{
    reply.httpReturn(500, "internal server error");
    reply.setHeader("Content-Type", "text/plain");
    reply.setHeader("Connection", "close");
    out << ex.what();
}

// NotFoundResponder
//

void NotFoundResponder::reply(std::ostream& out, Request& request, Reply& reply)
{
    reply.httpReturn(404, "Not found");
}

// NotFoundService
//

Responder* NotFoundService::createResponder(const Request&)
{
    return &_responder;
}

void NotFoundService::releaseResponder(Responder*)
{ }

// NotAuthenticatedResponder 
//

class NotAuthenticatedResponder : public Responder
{
        std::string _realm;
        std::string _content;

    public:
        explicit NotAuthenticatedResponder(Service& service, const std::string& realm, const std::string& content)
            : Responder(service),
              _realm(realm),
              _content(content)
            { }

        void reply(std::ostream&, Request& request, Reply& reply);
};

void NotAuthenticatedResponder::reply(std::ostream& out, Request& request, Reply& reply)
{
    reply.setHeader("WWW-Authenticate", "Basic realm=\"" + _realm + '"');

    reply.httpReturn(401, "not authorized");

    if (_content.empty())
        out << "<html><body><h1>not authorized</h1></body></html>";
    else
        out << _content;

}

// NotAuthenticatedService
//

Responder* NotAuthenticatedService::createResponder(const Request& request)
{
    return createResponder(request, std::string(), std::string());
}

Responder* NotAuthenticatedService::createResponder(const Request& request, const std::string& realm, const std::string& authContent)
{
    return new NotAuthenticatedResponder(*this, realm, authContent);
}

void NotAuthenticatedService::releaseResponder(Responder* responder)
{
    delete responder;
}

} // namespace http

} // namespace cxxtools
