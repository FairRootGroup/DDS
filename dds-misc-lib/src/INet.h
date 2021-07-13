// Copyright 2014 GSI, Inc. All rights reserved.
//
//  helpers for Socket and Network operations.
//
#ifndef _DDS_INET_H_
#define _DDS_INET_H_

// API
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
// STD
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unistd.h>
// Misc
#include "ErrorCode.h"
#include "MiscUtils.h"
#include "def.h"
// BOOST
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

/// this macro indicates an invalid status of the socket
#define INVALID_SOCKET -1

namespace dds::misc
{
    /**
     *
     *  @brief INet declares helpers for Socket and Network operations
     *
     */
    namespace INet
    {
        /**
         *
         * @brief A basic socket type
         *
         */
        typedef int Socket_t;

        // Forward declaration
        inline std::string socket_error_string(Socket_t _socket, const char* _strMsg = NULL);

        /**
         *
         *  @brief A wrapper for a basic Socket
         *
         */
        class smart_socket : public NONCopyable
        {
            // TODO: Implement reference count
          public:
            smart_socket()
                : m_Socket(INVALID_SOCKET)
            {
            }
            smart_socket(int _Socket)
                : m_Socket(_Socket)
            {
            }
            smart_socket(int _domain, int _type, int _protocol, bool _Block = false)
            {
                m_Socket = ::socket(_domain, _type, _protocol);
                // Blocking or Non-blocking socket
                if (_Block)
                    set_nonblock();
            }
            ~smart_socket()
            {
                close();
            }
            //                 Socket_t* operator&()
            //                 {
            //                     return & m_Socket;
            //                 }
            operator int() const
            {
                return static_cast<int>(m_Socket);
            }
            int operator=(const int& _Val)
            {
                close();
                return m_Socket = _Val;
            }
            Socket_t detach()
            {
                Socket_t Socket(m_Socket);
                m_Socket = INVALID_SOCKET;
                return Socket;
            }
            Socket_t get()
            {
                return m_Socket;
            }
            int set_nonblock(bool _val = true)
            {
                int opts = fcntl(m_Socket, F_GETFL);

                if (opts < 0)
                    return -1;

                opts = _val ? (opts | O_NONBLOCK) : (opts & ~O_NONBLOCK);

                return fcntl(m_Socket, F_SETFL, opts);
            }
            void close()
            {
                if (INVALID_SOCKET != m_Socket)
                {
                    shutdown();
                    ::close(m_Socket); // ignoring error code
                    m_Socket = INVALID_SOCKET;
                }
            }
            bool is_valid() const
            {
                return (INVALID_SOCKET != m_Socket);
            }
            int shutdown(int _How = SHUT_RDWR)
            {
                return ::shutdown(m_Socket, _How);
            }
            /// This function indicates that socket is ready to be read (for non-blocking sockets)
            int is_read_ready(size_t m_SecTimeOut, size_t m_USecTimeOut = 0)
            {
                if (!is_valid())
                    throw std::runtime_error("Socket is invalid");
                fd_set readset;
                FD_ZERO(&readset);
                FD_SET(m_Socket, &readset);

                // Setting time-out
                timeval timeout;
                timeout.tv_sec = m_SecTimeOut;
                timeout.tv_usec = m_USecTimeOut;

                // TODO: Send errno to log
                int retval = ::select(m_Socket + 1, &readset, NULL, NULL, &timeout);
                if (retval < 0)
                    throw std::runtime_error("Server's socket got error while calling \"select\"");
                if (0 == retval)
                    return 0;

                return FD_ISSET(m_Socket, &readset);
            }

          private:
            Socket_t m_Socket;
        };

        inline size_t read_from_socket(smart_socket& _Socket, BYTEVector_t* _Buf)
        {
            if (!_Buf)
                throw std::runtime_error("The given buffer pointer is NULL.");

            const ssize_t bytes_read = ::recv(_Socket, &(*_Buf)[0], _Buf->capacity(), 0);
            if (0 == bytes_read) // The  return value will be 0 when the peer has performed an orderly shutdown
            {
                _Socket.close();
                return 0;
            }
            if (bytes_read < 0)
            {
                if (ECONNRESET == errno || ENOTCONN == errno)
                    _Socket.close();
                throw system_error("");
            }

            return bytes_read;
        }
        /**
         *
         * @brief This is a stream operator which helps to \b receive data from the given socket.
         * @brief Generic declaration (no implementation).
         *
         */
        template <typename _T>
        smart_socket& operator>>(smart_socket& _Socket, _T* _Buf);
        /**
         *
         * @brief This is a stream operator which helps to \b receive data from the given socket.
         * @brief A template specialization for BYTEVector_t type.
         *
         */
        template <>
        inline smart_socket& operator>>(smart_socket& _Socket, BYTEVector_t* _Buf)
        {
            if (!_Buf)
                throw std::runtime_error("The given buffer pointer is NULL.");

            const ssize_t bytes_read = ::recv(_Socket, &(*_Buf)[0], _Buf->capacity(), 0);
            if (0 == bytes_read) // The  return value will be 0 when the peer has performed an orderly shutdown
            {
                _Buf->resize(bytes_read);
                _Socket.close();
                return _Socket;
            }
            if (bytes_read < 0)
            {
                if (ECONNRESET == errno || ENOTCONN == errno)
                    _Socket.close();
                throw system_error("");
            }

            _Buf->resize(bytes_read);
            return _Socket;
        }
        /**
         *
         * @brief A helper function, which insures that whole buffer was send.
         *
         */
        inline int sendall(int s, const unsigned char* const buf, int len, int flags)
        {
            // TODO: sendall - Make this code safer!!!
            int total = 0;
            int n = 0;

            while (total < len)
            {
                n = ::send(s, buf + total, len - total, flags);
                if (n == -1)
                {
                    // TODO: may be EWOULDBLOCK or on some systems EAGAIN when it returned
                    // due to its inability to send off data without blocking.
                    if (EAGAIN == errno || EWOULDBLOCK == errno)
                    {
                        // wait for a reasonable amount of time until
                        // we could send()
                        // sleep for 100 ms
                        // usleep(100 * 1000);
                        continue;
                    }
                    else
                        throw system_error("send data exception: ");
                }
                total += n;
            }

            return total;
        }
        /**
         *
         * @brief This is a stream operator which helps to \b send data to the given socket.
         * @brief Generic declaration (no implementation).
         *
         */
        template <typename _T>
        smart_socket& operator<<(smart_socket& _Socket, _T& _Buf);
        /**
         *
         * @brief This is a stream operator which helps to \b send data to the given socket.
         * @brief A template specialization for BYTEVector_t type.
         *
         */
        template <>
        inline smart_socket& operator<<(smart_socket& _Socket, BYTEVector_t& _Buf)
        {
            //::send( _Socket, &_Buf[ 0 ], _Buf.size(), 0 );
            sendall(_Socket, &_Buf[0], _Buf.size(), 0);
            return _Socket;
        }
        /**
         *
         * @brief A helper function, which sends a string to the given socket.
         *
         */
        inline void send_string(smart_socket& _Socket, const std::string& _Str2Send)
        {
            BYTEVector_t buf;
            copy(_Str2Send.begin(), _Str2Send.end(), back_inserter(buf));
            _Socket << buf;
        }
        /**
         *
         * @brief A helper function, which receives a string from the given socket.
         *
         */
        inline void receive_string(smart_socket& _Socket, std::string* _Str2Receive, size_t _BufSize)
        {
            if (!_Str2Receive)
                throw std::invalid_argument("smart_socket::receive_string: Parameter is NULL");

            BYTEVector_t buf(_BufSize);
            _Socket >> &buf;
            *_Str2Receive = std::string(reinterpret_cast<char*>(&buf[0]), buf.size());
        }
        /**
         *
         * @brief This function checks whether _Addr is an IP address or not.
         *
         */
        inline bool is_ip_address(std::string _Addr)
        {
            // removing all dots
            _Addr.erase(remove(_Addr.begin(), _Addr.end(), '.'), _Addr.end());
            // Checking for all numerics
            return (_Addr.end() == std::find_if(_Addr.begin(), _Addr.end(), std::not_fn(IsDigit())));
        }
        /**
         *
         * @brief host2ip converts a given host name to IP address.
         *
         */
        inline void host2ip(const std::string& _Host, std::string* _IP) // _Host can be either host name or IP address
        {
            if (!_IP)
                return;

            if (is_ip_address(_Host))
            {
                *_IP = _Host;
                return;
            }

            hostent* he = gethostbyname(_Host.c_str());
            if (!he)
                return; // TODO: throw... herror()

            *_IP = inet_ntoa(*(reinterpret_cast<in_addr*>(he->h_addr)));
        }
        /**
         *
         * @brief ip2host converts a given IP address to host name.
         *
         */
        inline void ip2host(const std::string& _IP, std::string* _Host)
        {
            if (!_Host)
                return;

            if (!is_ip_address(_IP))
            {
                *_Host = _IP;
                return;
            }

            in_addr addr;
            inet_aton(_IP.c_str(), &addr);
            hostent* he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
            if (!he)
                return;

            *_Host = he->h_name;
        }
        /**
         *
         * @brief CSocketServer implements a simple socket server.
         *
         */
        class CSocketServer
        {
          public:
            CSocketServer()
                : m_Socket(AF_INET, SOCK_STREAM, 0)
            {
            }
            void Bind(unsigned short _nPort, const std::string* _Addr = NULL)
            {
                if (m_Socket < 0)
                    throw std::runtime_error(socket_error_string(m_Socket, "NULL socket has been given to Bind"));

                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(_nPort);
                if (!_Addr)
                    addr.sin_addr.s_addr = htonl(INADDR_ANY);
                else
                    inet_aton(_Addr->c_str(), &addr.sin_addr);

                if (::bind(m_Socket, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0)
                    throw std::runtime_error(socket_error_string(m_Socket, "Socket bind error..."));
            }

            void Listen(int _Backlog)
            {
                if (::listen(m_Socket, _Backlog) < 0)
                    throw std::runtime_error(socket_error_string(m_Socket, "can't call listen on socket server"));
            }

            Socket_t Accept()
            {
                return ::accept(m_Socket, NULL, NULL);
            }
            Socket_t getSocket()
            {
                return m_Socket.get();
            }
            void setNonBlock(bool _val = true)
            {
                m_Socket.set_nonblock(_val);
            }
            Socket_t detach()
            {
                return m_Socket.detach();
            }

          protected:
            smart_socket m_Socket;
        };
        /**
         *
         * @brief CSocketClient implements a simple socket client.
         *
         */
        class CSocketClient
        {
          public:
            CSocketClient()
                : m_Socket(AF_INET, SOCK_STREAM, 0)
            {
            }

            void connect(unsigned short _nPort, const std::string& _Addr)
            {
                if (m_Socket < 0)
                    throw std::runtime_error(
                        socket_error_string(m_Socket, "there was NULL socket given as a client socket to Connect"));

                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(_nPort);
                std::string ip;
                host2ip(_Addr, &ip);
                inet_aton(ip.c_str(), &addr.sin_addr);

                if (::connect(m_Socket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
                    throw std::runtime_error(socket_error_string(m_Socket, "Can't connect to the server"));
            }

            Socket_t getSocket()
            {
                return m_Socket.get();
            }
            void setNonBlock(bool _val = true)
            {
                m_Socket.set_nonblock(_val);
            }
            Socket_t detach()
            {
                return m_Socket.detach();
            }

          protected:
            smart_socket m_Socket;
        };
        /**
         *
         * @brief A Trait class for _socket2string template. This class operates on a local side of the socket.
         *
         */
        struct SSocket2String_Trait
        {
            bool operator()(Socket_t _socket, sockaddr_in* _addr) const
            {
                socklen_t size = sizeof(sockaddr);
                return (getsockname(_socket, reinterpret_cast<sockaddr*>(_addr), &size) == -1) ? false : true;
            }
        };
        /**
         *
         * @brief A Trait class for _socket2string template. This class operates a peer of the socket.
         *
         */
        struct SSocketPeer2String_Trait
        {
            bool operator()(Socket_t _socket, sockaddr_in* _addr) const
            {
                socklen_t size = sizeof(sockaddr);
                return (getpeername(_socket, reinterpret_cast<sockaddr*>(_addr), &size) == -1) ? false : true;
            }
        };
        /**
         *
         * @brief A template class, which makes a string representation of the socket.
         * @brief In a form of [Host name]:[Port].
         *
         */
        template <class _Type>
        struct _socket2string
        {
            _socket2string(Socket_t _Socket, std::string* _Str)
            {
                if (!_Str)
                    return;

                sockaddr_in addr;
                if (!_Type()(_Socket, &addr))
                    return;

                std::string host;
                ip2host(inet_ntoa(addr.sin_addr), &host);

                std::stringstream ss;
                ss << host << ":" << ntohs(addr.sin_port);
                *_Str = ss.str();
            }
        };
        /**
         *
         * @brief Socket to string representation.
         *
         */
        typedef _socket2string<SSocket2String_Trait> socket2string;
        /**
         *
         * @brief Socket-peer to string representation.
         *
         */
        typedef _socket2string<SSocketPeer2String_Trait> peer2string;
        /**
         *
         * @brief The function returns socket's error string.
         *
         */
        inline std::string socket_error_string(Socket_t _socket, const char* _strMsg)
        {
            std::string strSocket;
            socket2string(_socket, &strSocket);
            std::string strSocketPeer;
            peer2string(_socket, &strSocketPeer);
            std::string sErr;
            dds::misc::errno2str(&sErr);

            std::ostringstream ss;
            if (_strMsg)
            {
                ss << _strMsg << "\n";
            }
            ss << "Error on Socket<" << strSocket << ">";

            if (!strSocketPeer.empty())
            {
                ss << "and peer <" << strSocketPeer << ">";
            }
            ss << ": " << sErr;

            return ss.str();
        }
        /**
         *
         * @brief The function checks and returns a free port from the given range of the ports.
         *
         */
        inline int get_free_port(int _Min, int _Max)
        {
            CSocketServer serv;
            for (int i = _Min; i <= _Max; ++i)
            {
                try
                {
                    serv.Bind(i);
                    return i;
                }
                catch (...)
                {
                    continue;
                }
            }
            return 0;
        }
        /**
         *
         * @brief The function checks whether the given port is free or not.
         *
         */
        inline int get_free_port(int _Port)
        {
            CSocketServer serv;
            try
            {
                serv.Bind(_Port);
                return _Port;
            }
            catch (...)
            {
            }
            return 0;
        }

// some old Linux systems don't have ntohll and htonll
#ifndef ntohll
        inline uint64_t htonll(uint64_t val)
        {
            return (((uint64_t)htonl(val)) << 32) + htonl(val >> 32);
        }

        inline uint64_t ntohll(uint64_t val)
        {
            return (((uint64_t)ntohl(val)) << 32) + ntohl(val >> 32);
        }
#endif
        // The following 4 functions convert values between host and network byte order.
        // Whenever data should be send to a remote peer the _normalizeWrite must be used.
        // Whenever data are going to be read from the _normalizeRead must be used to check that Endianness is correct.
        template <typename T>
        T normalizeRead(T _value);

        template <>
        inline uint16_t normalizeRead<uint16_t>(uint16_t _value)
        {
            return ntohs(_value);
        }

        template <>
        inline uint32_t normalizeRead<uint32_t>(uint32_t _value)
        {
            return ntohl(_value);
        }

        template <>
        inline uint64_t normalizeRead<uint64_t>(uint64_t _value)
        {
            return ntohll(_value);
        }

        template <typename T>
        T normalizeWrite(T _value);

        template <>
        inline uint16_t normalizeWrite<uint16_t>(uint16_t _value)
        {
            return htons(_value);
        }

        template <>
        inline uint32_t normalizeWrite<uint32_t>(uint32_t _value)
        {
            return htonl(_value);
        }

        template <>
        inline uint64_t normalizeWrite<uint64_t>(uint64_t _value)
        {
            return htonll(_value);
        }

        /**
         *
         * @brief A helper function, which insures that whole buffer was written.
         *
         */
        inline void writeall(int _handle, const std::string& _msg)
        {
            size_t total = 0;
            int n = 0;

            std::vector<char> buf;
            buf.reserve(_msg.size());
            copy(_msg.begin(), _msg.end(), back_inserter(buf));

            const size_t len = buf.size();

            while (total < len)
            {
                if ((n = write(_handle, &buf[total], len - total)) < 0)
                    throw system_error("send command: Write error");
                total += n;
            }
        }
    }; // namespace INet
};     // namespace dds::misc

#endif
