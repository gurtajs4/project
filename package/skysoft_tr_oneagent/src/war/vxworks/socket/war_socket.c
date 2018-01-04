/*=======================================================================
  
       Copyright(c) 2009, Works Systems, Inc. All rights reserved.
  
       This software is supplied under the terms of a license agreement 
       with Works Systems, Inc, and may not be copied nor disclosed except 
       in accordance with the terms of that agreement.
  
  =======================================================================*/
/*
 * All rights reserved.
 *
 * Redistribution and use in source code and binary executable file, with or without modification,
 * are prohibited without prior written permission from Works Systems, Inc.
 * The redistribution may be allowed subject to the terms of the License Agreement with Works Systems, Inc.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <hostLib.h>
#include "war_type.h"

/*!
 * \fn war_loadsock.
 * 
 * \brief This function load socket dll before uesing socket [WIN32].
 * 
 * \return zero when success, -1 when any error.
 * 
 */
int war_loadsock ()
{
    /*do nothing, just for win32*/
    return 0;
}

/*!
 * \fn war_getsockopt.
 * 
 * \brief This function get options on sockets.
 * 
 * \param s:Descriptor identifying a socket. 
 * \param level: Level at which the option is defined.
 * \param optname: Socket option for which the value is to be retrieved
 * \param optval: Pointer to the buffer in which the value for the requested option is to be returned.
 * \param optlen: Pointer to the size of the optval buffer
 * 
 * \return zero when success, -1 when any error.
 * 
 */
int war_getsockopt(int s, int level, int optname, void *optval, int *optlen)
{
    /*need extend according level and optname*/
   return  getsockopt(s, level, optname, (char *)optval, optlen);
}

/*!
 * \fn war_setsockopt.
 * 
 * \brief This function set options on sockets.
 * 
 * \param s:Descriptor identifying a socket 
 * \param level: Level at which the option is defined.
 * \param optname: Socket option for which the value is to be set
 * \param optval: Pointer to the buffer in which the value for the requested option is supplied
 * \param optlen: Size of the optval buffe
 * 
 * \return zero when success, -1 when any error.
 * 
 */
int war_setsockopt( int s, int level, int optname, const void *optval, int optlen)
{
    /*need extend according level and optname*/
	if (optname == SO_PRIORITY)
		return 0;
    else
    	return setsockopt(s, level, optname, (char *)optval, optlen);
}

/*!
 * \fn war_ioctl.
 * 
 * \brief This function controls the I/O mode of a socke.
 * 
 * \param d: Descriptor identifying a socket.
 * \param request: Command to perform on socket.
 * \param argp: Pointer to a parameter for request.
 * 
 * \return zero when success, -1 when any error.
 * \Note req int(linux) long(win32)  argp int*(linux) u_long *(win32)
 * 
 */
int war_ioctl( int d, ioctl_req_t request, ioctl_argp_t * argp)
{
    return ioctl(d, request, argp);
}

/*!
 * \fn war_socket.
 * 
 * \brief This function creates a socket that is bound to a specific transport service provider.
 * 
 * \param domain: The communication domain.
 * \param type: The type specification for the new socket.
 * \param protocol: The protocol to be used.
 * 
 * \return -1 if an error occurs; otherwise the return value is a descriptor referencing the socket.
 * 
 */
int war_socket( int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

/*!
 * \fn war_sockclose.
 * 
 * \brief This function closes an existing socket.
 * 
 * \param fd: Descriptor identifying the socket to close.
 * 
 * \return the zero when success, -1 when any error.
 * 
 */
int war_sockclose(int fd)
{
    return close(fd);
}

/*!
 * \fn war_gethostbyname.
 * 
 * \brief This function retrieves host information corresponding to a host name from a host database.
 * 
 * \param fd: Here name is either a host name, or an IPv4 address in standard dot notation, or an IPv6 address in colon (and possibly dot) notation.
 * 
 * \return the hostent structure or a NULL pointer if an error occurs. 
 * 
 */
struct hostent * war_gethostbyname (const char *name)
{
	return gethostbyname(name);
}

/*!
 * \fn war_recvfrom.
 * 
 * \brief This function receives a message from a socket.
 * 
 * \param s: The socket file descriptor.
 * \param buf: The buffer to store the message.
 * \param len: The length of buf.
 * \param flags: The optional choice for recvfrom.
 * \param from: If from is not NULL, and the underlying protocol provides the source 
 * address, this source address is filled in. 
 * \param fromlen: The fromlen is initialized to the size of the buffer associated with 
 * from, and modified on return to indicate the actual size of the address stored there.
 * 
 * \return -1 if an error occurs, errno is set appropriately; return 0 if success.
 * 
 */
ssize_t war_recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
	//int recvfrom (int s, char *buf, int bufLen, int flags,struct sockaddr *from, int *pFromLen); 
	return (ssize_t)recvfrom(s, (char *)buf, (int)len, flags, from, (int*)fromlen);
}

/*!
 * \fn war_connect.
 * 
 * \brief This function initiates a connection on a socket.
 * 
 * \param sockfd: The socket file descriptor.
 * \param serv_addr: Remote server address.
 * \param addrlen: The lenght of serv_addr.
 * 
 * \return -1 if an error occurs, errno is set appropriately; return 0 if success.
 * 
 */
int war_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	//extern STATUS connect (int s, struct sockaddr *name, int namelen);
	return (int)connect(sockfd, (struct sockaddr *)serv_addr, (int)addrlen);
}

/*!
 * \fn war_accept.
 * 
 * \brief This function accepts a connection on a socket.
 * 
 * \param s: The socket file descriptor.
 * \param addr: A pointer to a sockaddr structure. This structure is filled in with the address of the connecting entity.
 * \param addrlen: The lenght of addr.
 * 
 * \return -1 if an error occurs, errno is set appropriately; 
 * returns a non-negative integer as a descriptor for the accepted socket when success.
 * 
 */
int war_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	//extern int accept (int s, struct sockaddr *addr, int *addrlen);
	return accept(s, addr, (int *)addrlen);
}

/*!
 * \fn war_send.
 * 
 * \brief This function sends a message from a socket.
 * 
 * \param s: The socket file descriptor.
 * \param buf: A pointer to the message.
 * \param len: The lenght of buf.
 * \param flags: The optional choice for send.
 * 
 * \return -1 if an error occurs, errno is set appropriately; returns the number of charactors sent when success.
 * 
 */
ssize_t war_send(int s, const void *buf, size_t len, int flags)
{
	//extern int send (int s, const char *buf, int bufLen, int flags);
	return (ssize_t)send(s, (const char *)buf, (int)len, flags);
}

/*!
 * \fn war_getsockname.
 * 
 * \brief This function gets socket name.
 * 
 * \param s: The socket file descriptor.
 * \param name: Getsockname returns the current name for the specified socket.
 * \param namelen:  The namelen parameter should be initialized to indicate the amount of space
 * pointed to by name. On return it contains the actual size of the name returned (in bytes).
 * 
 * \return -1 if an error occurs, errno is set appropriately; returns 0 when success.
 * 
 */
int war_getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	//extern STATUS getsockname (int s, struct sockaddr *name, int *namelen);
	return (int)getsockname(s, name, (int *)namelen);
}

