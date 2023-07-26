/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_API_H
#define TEA_API_H

#ifndef TEA_SERVER
#ifndef NDEBUGA
//#define TEA_SERVER "https://iredirect.000webhostapp.com/"
//#define TEA_SERVER "http://object.getenjoyment.net/"
#define TEA_SERVER "https://dragontea--lightmister.repl.co/"


#else
#define TEA_SERVER "http://localhost:8000/"
#endif
#endif

#define TEA_SERVER_AUTH_URL TEA_SERVER "api/auth.php"
#define TEA_SERVER_REGISTER_URL TEA_SERVER "api/register.php"
#define TEA_SERVER_MESSAGE_HANDLER_URL TEA_SERVER "api/messageHandler.php"

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

#endif
