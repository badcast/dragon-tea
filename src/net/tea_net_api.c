#include <curl/curl.h>

#include "tea.h"

struct tea_net_stats net_stats;
struct tea_server_info current_server_info;

struct
{
    char SAUTH[255];
    char SREGISTER[255];
    char SMHANDLER[255];
    char SINFO[255];
} slocal_vars;

struct net_responce_t
{
    size_t size;
    int net_status;
    char *raw_data;
    char *json_data;
};

GMutex nmutex;
void net_init()
{
    g_mutex_init(&nmutex);
    curl_global_init(CURL_GLOBAL_ALL);
    memset(&net_stats, 0, sizeof(net_stats));
}

void net_free()
{
    g_mutex_clear(&nmutex);
    curl_global_cleanup();
}

size_t curl_writer(void *ptr, size_t size, size_t nmemb, struct net_responce_t *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char *tmp;

    data->size += (size * nmemb);

    tmp = (char *) realloc(data->raw_data, data->size + 1); /* +1 for '\0' */

    if(tmp)
    {
        data->raw_data = tmp;
    }
    else
    {
        if(data->raw_data)
        {
            free(data->raw_data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy(((char *) data->raw_data + index), ptr, n);
    data->raw_data[data->size] = '\0';

    return size * nmemb;
}

int curl_xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    size_t *p = (size_t *) clientp;
    *p += ulnow;
    ++p;
    *p += dlnow;
    return CURLE_OK;
}

int net_send(const char *url, const char *body, size_t len, struct net_responce_t *receiver)
{
    CURL *curl;
    CURLcode net_result;
    size_t request_stats[2];

    memset(receiver, 0, sizeof(*receiver));

    memset(request_stats, 0, sizeof(request_stats));

    curl = curl_easy_init();

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, receiver);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
    if(body != NULL && len)
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
    }
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, request_stats);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, curl_xferinfo);

#ifdef NDEBUG
    // timeout 10 seconds
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
#endif

    g_mutex_lock(&nmutex);
    net_stats.active_requests++;
    g_mutex_unlock(&nmutex);

    // SEND =====>
    net_result = curl_easy_perform(curl);

    // clear curl data ~
    curl_easy_cleanup(curl);

    g_mutex_lock(&nmutex);
    net_stats.transmitted_bytes += request_stats[0];
    net_stats.received_bytes += request_stats[1];
    net_stats.active_requests--;

    if(net_result == CURLE_OK)
        ++net_stats.success_req;
    else
        ++net_stats.error_req;

    g_mutex_unlock(&nmutex);

    if(net_result == CURLE_OK)
    {
        for(int x = 0; x < receiver->size; ++x)
        {
            if(receiver->raw_data[x] == '{')
            {
                receiver->json_data = receiver->raw_data + x;
                break;
            }
        }
    }
    else
    {
        if(receiver->raw_data)
        {
            free(receiver->raw_data);
            receiver->raw_data = NULL;
        }

#ifndef NDEBUG
        printf("curl error code: %s (%d)\n", curl_easy_strerror(net_result), net_result);
#endif
    }

    receiver->net_status = net_result;

    // if(net_result == CURLE_OK){
    // CURLINFO cf;
    // curl_easy_getinfo(curl, &cf);}

    // begin up to json data {}

    return net_result == CURLE_OK;
}

int net_api_read_messages(
    const struct tea_id_info *user,
    tea_id_t target_user_id,
    tea_id_t msg_id_start,
    int max_messages,
    struct tea_message_read_result *output)
{
    const char *jstr;
    int result;
    size_t _size;
    struct net_responce_t input;
    json_object *json_request, *json_result, *jresult_obj, *jmessages, *jval;
    /*
    Request:
        type: The Type is Read - for use = reader
        user_id: The User Id (Unique Identification Number)
        user_nickname: The User Nickname
        msg_id: for start message get's
        max_mesa

    Responce:
        status: the status of code

        result: message information
            length: the message count
            messages[N]: the messages content
            [0]
            {
              send_user_id: - The sender User ID
              send_user_nickname: The sender User Nickname
              message: The text of the sender
              time_received: The time reply on server
              time_saved: The time saved on server
              msg_id: The message identifier
            }
            [N]
    */
    json_request = json_object_new_object();
    json_object_object_add(json_request, "type", json_object_new_string("reader"));
    json_object_object_add(json_request, "user_id", json_object_new_int64(user->user_id));
    json_object_object_add(json_request, "user_nickname", json_object_new_string(user->user_nickname));
    json_object_object_add(json_request, "target_user_id", json_object_new_int64(target_user_id));
    json_object_object_add(json_request, "msg_id", json_object_new_int64(msg_id_start));
    json_object_object_add(json_request, "max_messages", json_object_new_int(max_messages));
    jstr = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);

    if((result = net_send(tea_url_server_message_handler(), jstr, _size, &input)) && input.raw_data)
    {
        json_result = json_tokener_parse(input.json_data);

        if(json_object_object_get_ex(json_result, "status", &jval) && (output->status = json_object_get_int(jval)) == TEA_STATUS_OK)
        {
            json_object_object_get_ex(json_result, "result", &jresult_obj);

            json_object_object_get_ex(jresult_obj, "messages", &jmessages);
            _size = json_object_array_length(jmessages);
            output->message_length = _size;
            if(_size > 0)
            {
                output->messages = g_array_new(FALSE, FALSE, sizeof(struct tea_message_id));
                // set array size
                g_array_set_size(output->messages, _size);

                // enumerate messages
                json_object *jmsg;
                struct tea_message_id *msg;
                for(; _size-- > 0;)
                {
                    jmsg = json_object_array_get_idx(jmessages, _size);
                    msg = &g_array_index(output->messages, struct tea_message_id, _size);

                    json_object_object_get_ex(jmsg, "msg_id", &jval);
                    msg->msg_id = json_object_get_int64(jval);

                    json_object_object_get_ex(jmsg, "send_user_id", &jval);
                    msg->sent_user_id = json_object_get_int64(jval);

                    json_object_object_get_ex(jmsg, "time_received", &jval);
                    msg->time_received = json_object_get_int64(jval);

                    json_object_object_get_ex(jmsg, "time_saved", &jval);
                    msg->time_saved = json_object_get_int64(jval);

                    json_object_object_get_ex(jmsg, "send_user_nickname", &jval);
                    jstr = json_object_get_string(jval);
                    strncpy(msg->sent_user_name, jstr, TEA_MAXLEN_USERNAME);

                    json_object_object_get_ex(jmsg, "message", &jval);
                    jstr = json_object_get_string(jval);
                    msg->message_text = strdup(jstr);
                }
            }
        }

        json_object_put(json_result);
    }
    else
    {
        output->status = TEA_STATUS_NETWORK_ERROR;
    }

    json_object_put(json_request);

    // free result data
    if(input.raw_data)
    {
        free(input.raw_data);
    }

    return result;
}

int net_api_write_message(
    const struct tea_id_info *user_sender, tea_id_t target_user_id, const char *message, int len, struct tea_message_send_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *json_result, *jresult_obj, *jval;

    const char *json_serialized;
    struct net_responce_t input;
    /*
    Request:
        type: The Type is Write - for use = writer
        user_id: The User Id (Unique Identification Number)
        user_nickname: The User Nickname
        target_user_id: The User ID to send
        message: The text message
    Responce:
       status: the status of code
       time_received: the time received message
    */
    json_request = json_object_new_object();
    json_object_object_add(json_request, "type", json_object_new_string("writer"));
    json_object_object_add(json_request, "user_id", json_object_new_int64(user_sender->user_id));
    json_object_object_add(json_request, "user_nickname", json_object_new_string(user_sender->user_nickname));
    json_object_object_add(json_request, "target_user_id", json_object_new_int64(target_user_id));
    json_object_object_add(json_request, "message", json_object_new_string_len(message, len));
    json_serialized = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);

    if((net = net_send(tea_url_server_message_handler(), json_serialized, _size, &input)) && input.raw_data)
    {
        json_result = json_tokener_parse(input.json_data);

        if(json_object_object_get_ex(json_result, "status", &jval) && (output->status = json_object_get_int(jval)) == TEA_STATUS_OK)
        {
            json_object_object_get_ex(json_result, "result", &jresult_obj);

            json_object_object_get_ex(jresult_obj, "time_received", &jval);
            output->time_received = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "time_saved", &jval);
            output->time_saved = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "msg_id", &jval);
            output->msg_id = json_object_get_int64(jval);
        }

        json_object_put(json_result);
    }
    else
    {
        output->status = TEA_STATUS_INTERNAL_SERVER_ERROR;
    }

    json_object_put(json_request);

    // free result data
    if(input.raw_data)
    {
        free(input.raw_data);
    }

    return net;
}

int net_api_signin(tea_id_t user_id, tea_login_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *jresult, *jval;
    const char *json_serialized;
    struct net_responce_t input;

    while(1)
    {
        json_request = json_object_new_object();
        json_object_object_add(json_request, "user_id", json_object_new_int64(user_id));

        json_serialized = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);
        if((net = net_send(tea_url_server_auth(), json_serialized, _size, &input)) && input.raw_data)
        {
            jresult = json_tokener_parse(input.json_data);

            if(!json_object_object_get_ex(jresult, "status", &jval))
            {
                output->status = TEA_STATUS_INTERNAL_SERVER_ERROR;
                break;
            }

            output->status = json_object_get_int(jval);

            if(json_object_object_get_ex(jresult, "authorized", &jval) && (output->authorized = json_object_get_boolean(jval)))
            {
                json_object *jresult_obj;
                json_object_object_get_ex(jresult, "result", &jresult_obj);

                json_object_object_get_ex(jresult_obj, "user_id", &jval);
                output->result.user_id = json_object_get_int64(jval);

                json_object_object_get_ex(jresult_obj, "user_nickname", &jval);
                json_serialized = json_object_get_string(jval);
                strncpy(output->result.user_nickname, json_serialized, TEA_MAXLEN_USERNAME);

                json_object_object_get_ex(jresult_obj, "creation_date", &jval);
                output->result.creation_date = json_object_get_int64(jval);

                json_object_object_get_ex(jresult_obj, "last_login", &jval);
                output->result.last_login = json_object_get_int64(jval);
            }

            // free json resource
            json_object_put(jresult);
        }
        else
        {
            output->status = TEA_STATUS_NETWORK_ERROR;
        }
        break;
    }
    // free json resource
    json_object_put(json_request);

    // free result data
    if(input.raw_data)
    {
        free(input.raw_data);
    }

    return net;
}

int net_api_signup(const char *nickname, tea_register_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *jresult, *jval;
    const char *json_serialized;
    struct net_responce_t input;

    json_request = json_object_new_object();
    json_object_object_add(json_request, "user_nickname", json_object_new_string(nickname));

    json_serialized = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);

    if((net = net_send(tea_url_server_register(), json_serialized, _size, &input)) && input.raw_data)
    {
        jresult = json_tokener_parse(input.json_data);

        json_object_object_get_ex(jresult, "status", &jval);
        output->status = json_object_get_int(jval);

        if(json_object_object_get_ex(jresult, "authorized", &jval) && (output->authorized = json_object_get_boolean(jval)))
        {
            json_object *jresult_obj;
            json_object_object_get_ex(jresult, "result", &jresult_obj);

            json_object_object_get_ex(jresult_obj, "user_id", &jval);
            output->result.user_id = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "user_nickname", &jval);
            json_serialized = json_object_get_string(jval);
            strncpy(output->result.user_nickname, json_serialized, TEA_MAXLEN_USERNAME);

            json_object_object_get_ex(jresult_obj, "creation_date", &jval);
            output->result.creation_date = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "last_login", &jval);
            output->result.last_login = json_object_get_int64(jval);
        }

        // free json resource
        json_object_put(jresult);
    }
    else
    {
        output->status = TEA_STATUS_NETWORK_ERROR;
    }

    // free json resource
    json_object_put(json_request);

    // free result data
    if(input.raw_data)
    {
        free(input.raw_data);
        input.raw_data = NULL;
    }

    return net;
}

int net_api_server_info(struct tea_server_info *serverInfo)
{
    int net;
    struct net_responce_t responce;

    net = net_send(tea_url_server_info(), NULL, 0, &responce);
    // OK
    if(net)
    {
        sscanf(
            responce.raw_data,
            "%d.%d.%d",
            &serverInfo->server_version.major,
            &serverInfo->server_version.minor,
            &serverInfo->server_version.patch);

        /*
         * 1.0.0 - no supported get first/last message id
         * 1.0.1 - supported get first/last message id
         */

        // calculate supported version
        if(serverInfo->server_version.major == 1)
        {
            // MAJOR = 1
        }

        if(serverInfo->server_version.minor == 0)
        {
            // MINOR = 0
        }

        if(serverInfo->server_version.patch == 1)
        {
            // PATCH = 1
        }
    }

    if(responce.raw_data)
    {
        free(responce.raw_data);
    }

    return net;
}

const char *tea_url_server()
{
    int s = -1;
    for(int x = 0; x < sizeof(app_settings.servers) / sizeof(app_settings.servers[0]); ++x)
    {
        if(strlen(app_settings.servers[x]) == 0)
            break;
        if(app_settings.active_server == tea_get_server_id(app_settings.servers[x]))
        {
            s = x;
            break;
        }
    }

    if(s == -1)
    {
        return NULL;
    }

    return app_settings.servers[s];
}

const char *tea_url_server_auth()
{
    const char *server = tea_url_server();
    if(!server)
        return server;
    snprintf(slocal_vars.SAUTH, sizeof(slocal_vars.SAUTH), "%sapi/auth.php", server);
    return slocal_vars.SAUTH;
}

const char *tea_url_server_register()
{

    const char *server = tea_url_server();
    if(!server)
        return server;
    snprintf(slocal_vars.SREGISTER, sizeof(slocal_vars.SREGISTER), "%sapi/register.php", server);
    return slocal_vars.SREGISTER;
}

const char *tea_url_server_message_handler()
{

    const char *server = tea_url_server();
    if(!server)
        return server;
    snprintf(slocal_vars.SMHANDLER, sizeof(slocal_vars.SMHANDLER), "%sapi/messageHandler.php", server);
    return slocal_vars.SMHANDLER;
}

const char *tea_url_server_info()
{
    const char *server = tea_url_server();

    if(!server)
        return server;
    snprintf(slocal_vars.SINFO, sizeof(slocal_vars.SINFO), "%sapi/TEA_SERVER_VERSION", server);
    return slocal_vars.SINFO;
}
