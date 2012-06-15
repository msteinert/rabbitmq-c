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

/**
 * \file
 * Open an SSL/TLS connection
 */

#ifndef AMQP_SSL_H
#define AMQP_SSL_H

#include <amqp.h>

/**
 * Create a new SSL/TLS socket object.
 *
 * \return A new socket object or NULL if an error occurred.
 */
AMQP_PUBLIC_FUNCTION
amqp_socket_t *
AMQP_CALL
amqp_ssl_socket_new(void);

/**
 * Set the CA certificate.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] cacert Path to the CA cert file in PEM format.
 *
 * \return Zero if successful, -1 otherwise.
 */
AMQP_PUBLIC_FUNCTION
int
AMQP_CALL
amqp_ssl_socket_set_cacert(amqp_socket_t *self,
			   const char *cacert);

/**
 * Set the client key & certificate.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] key Path to the client key in PEM format.
 * \param [in] cert Path to the client certificate in PEM foramt.
 *
 * \return Zero if successful, false otherwise.
 */
AMQP_PUBLIC_FUNCTION
int
AMQP_CALL
amqp_ssl_socket_set_key(amqp_socket_t *self,
			const char *key,
			const char *cert);

/**
 * Enable or disable peer verification.
 *
 * If peer verification is enabled then the common name in the server
 * certificate must match the server name.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] verify Enable or disable peer verification.
 */
AMQP_PUBLIC_FUNCTION
void
AMQP_CALL
amqp_ssl_socket_set_verify(amqp_socket_t *self,
			   amqp_boolean_t verify);

/**
 * Enable or disable blocking.
 *
 * \param [in,out] self An SSL/TLS socket object.
 * \param [in] blocking Enable or disable blocking.
 */
AMQP_PUBLIC_FUNCTION
void
AMQP_CALL
amqp_ssl_socket_set_blocking(amqp_socket_t *self,
			     amqp_boolean_t blocking);

/**
 * Sets whether rabbitmq-c initializes the underlying SSL library.
 *
 * For SSL libraries that require a one-time initialization across
 * a whole program (e.g., OpenSSL) this sets whether or not rabbitmq-c
 * will initialize the SSL library when the first call to
 * amqp_open_ssl_socket() is made. You should call this function with
 * do_init = 0 if the underlying SSL library is intialized somewhere else
 * the program.
 *
 * Failing to initialize or double initialization of the SSL library will
 * result in undefined behavior
 *
 * By default rabbitmq-c will initialize the underlying SSL library
 *
 * NOTE: calling this function after the first socket has been opened with
 * amqp_open_ssl_socket() will not have any effect.
 *
 * \param [in] do_initalize If 0 rabbitmq-c will not initialize the SSL
 *                          library, otherwise rabbitmq-c will initialize the
 *                          SL library
 *
 */
AMQP_PUBLIC_FUNCTION
void
AMQP_CALL
amqp_set_initialize_ssl_library(amqp_boolean_t do_initialize);

#endif /* AMQP_SSL_H */
