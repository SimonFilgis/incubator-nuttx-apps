#include <nuttx/config.h>
#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

static int callback_echo(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len)
{
    switch (reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connection established\n");
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            const char *msg = "hello from NuttX";
            unsigned char buf[LWS_PRE + 128];
            size_t n = sprintf((char *)(&buf[LWS_PRE]), "%s", msg);

            int written = lws_write(wsi, &buf[LWS_PRE], n, LWS_WRITE_TEXT);
            printf("Sent %d bytes\n", written);
            break;
        }

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("Received: %.*s\n", (int)len, (const char *)in);
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            break;

        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        .name = "echo-protocol",
        .callback = callback_echo,
        .per_session_data_size = 0,
        .rx_buffer_size = 0,
    },
    { NULL, NULL, 0, 0 } // terminator
};

int main(int argc, char **argv)
{
    struct lws_context_creation_info info;
    struct lws_client_connect_info ccinfo;
    struct lws_context *context;
    struct lws *wsi;

    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    context = lws_create_context(&info);
    if (!context) {
        printf("Failed to create context\n");
        return 1;
    }

    memset(&ccinfo, 0, sizeof(ccinfo));
    ccinfo.context = context;
    ccinfo.address = "192.168.1.7"; // example echo server
    ccinfo.port = 8080; // or 443 for wss
    ccinfo.path = "/";
    ccinfo.host = ccinfo.address;
    ccinfo.origin = ccinfo.address;
    ccinfo.protocol = protocols[0].name;
    ccinfo.ssl_connection = 0; // set to 1 for wss

    wsi = lws_client_connect_via_info(&ccinfo);
    if (!wsi) {
        printf("Failed to initiate connection\n");
        lws_context_destroy(context);
        return 1;
    }

    while (lws_service(context, 0) >= 0) {
        // event loop
    }

    lws_context_destroy(context);
    return 0;
}

/*
 * lws-minimal-ws-client-echo
 *
 * Written in 2010-2019 by Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates a ws client that echoes back what it was sent, in a
 * way compatible with autobahn -m fuzzingserver
 */



