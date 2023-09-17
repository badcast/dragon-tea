/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_API_H
#define TEA_API_H

enum TeaErrorStatus
{
    TEA_STATUS_OK = 0,
    TEA_STATUS_ID_NO_EXIST = 1,
    TEA_STATUS_INVALID_REQUEST_DATA = 2,
    TEA_STATUS_INVALID_AUTH_METHOD = 3,
    TEA_STATUS_INVALID_NICKNAME = 4,
    TEA_STATUS_INVALID_REGISTER = 5,
    TEA_STATUS_ADMIN_ACCOUNT_REACHABLE = 6,

    TEA_STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED = 128,

    TEA_STATUS_INTERNAL_SERVER_ERROR = 500,

    TEA_STATUS_NETWORK_ERROR = 1000
};

const char *tea_get_server_uri();
const char *tea_get_server_auth();
const char *tea_get_server_register();
const char *tea_get_server_message_handler();


#endif
