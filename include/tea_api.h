/*
 * author: badcast <lmecomposer@gmail.com>
 * year 2023
 * status: already coding
 */

#ifndef TEA_API_H
#define TEA_API_H

#ifndef TEA_SERVER
#if NADEBUG
#define TEA_SERVER "https://iredirect.000webhostapp.com/"
#else
#define TEA_SERVER "http://localhost:8000/"
#endif
#endif

#define TEA_SERVER_AUTH_URL TEA_SERVER "api/auth.php"
#define TEA_SERVER_REGISTER_URL TEA_SERVER "api/register.php"

enum TeaErrorStatus
{
    TEA_STATUS_OK = 0,
    TEA_STATUS_ID_NO_EXIST = 1,
    TEA_STATUS_INVALID_REQUEST_DATA = 2,
    TEA_STATUS_INVALID_AUTH_METHOD = 3,
    TEA_STATUS_INVALID_NICKNAME = 4,
    TEA_STATUS_INVALID_REGISTER = 5,
    TEA_STATUS_INTERNAL_SERVER_ERROR = 500,

    TEA_STATUS_NETWORK_ERROR = 1000
};

#endif
