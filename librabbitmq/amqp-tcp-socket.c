/*
 * Copyright 2012 Michael Steinert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "amqp_private.h"
#include "amqp-tcp-socket.h"
#include <stdlib.h>

struct amqp_tcp_socket_t {
	amqp_socket_t base;
	int sockfd;
};

static ssize_t
amqp_tcp_socket_writev(void *base, const struct iovec *iov, int iovcnt)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	return amqp_os_socket_writev(self->sockfd, iov, iovcnt);
}

static ssize_t
amqp_tcp_socket_send(void *base, const void *buf, size_t len, int flags)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	return send(self->sockfd, buf, len, flags);
}

static ssize_t
amqp_tcp_socket_recv(void *base, void *buf, size_t len, int flags)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	return recv(self->sockfd, buf, len, flags);
}

static int
amqp_tcp_socket_open(void *base, const char *host, int port)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	struct sockaddr_in addr;
	int status, one = 1;
	struct hostent *he;
	status = amqp_socket_init();
	if (status) {
		return status;
	}
	he = gethostbyname(host);
	if (!he) {
		return -ERROR_GETHOSTBYNAME_FAILED;
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = *(uint32_t *)he->h_addr_list[0];
	self->sockfd = amqp_socket_socket(PF_INET, SOCK_STREAM, 0);
	if (-1 == self->sockfd) {
		return -amqp_os_socket_error();
	}
	status = amqp_socket_setsockopt(self->sockfd, IPPROTO_TCP, TCP_NODELAY,
					&one, sizeof(one));
	if (0 > status) {
		status = -amqp_os_socket_error();
		amqp_os_socket_close(self->sockfd);
		return status;
	}
	status = connect(self->sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (0 > status) {
		status = -amqp_os_socket_error();
		amqp_os_socket_close(self->sockfd);
		return status;
	}
	return 0;
}

static int
amqp_tcp_socket_close(void *base)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	int status = -1;
	if (self) {
		status = amqp_os_socket_close(self->sockfd);
		free(self);
	}
	return status;
}

static int
amqp_tcp_socket_error(AMQP_UNUSED void *base)
{
	return amqp_os_socket_error();
}

static int
amqp_tcp_socket_get_sockfd(void *base)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	return self->sockfd;
}

amqp_socket_t *
amqp_tcp_socket_new(void)
{
	struct amqp_tcp_socket_t *self = calloc(1, sizeof(*self));
	if (!self) {
		return NULL;
	}
	self->base.writev = amqp_tcp_socket_writev;
	self->base.send = amqp_tcp_socket_send;
	self->base.recv = amqp_tcp_socket_recv;
	self->base.open = amqp_tcp_socket_open;
	self->base.close = amqp_tcp_socket_close;
	self->base.error = amqp_tcp_socket_error;
	self->base.get_sockfd = amqp_tcp_socket_get_sockfd;
	self->sockfd = -1;
	return (amqp_socket_t *)self;
}

void
amqp_tcp_socket_set_sockfd(amqp_socket_t *base, int sockfd)
{
	struct amqp_tcp_socket_t *self = (struct amqp_tcp_socket_t *)base;
	self->sockfd = sockfd;
}
