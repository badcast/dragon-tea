#include <curl/curl.h>
#include <inttypes.h>

#include "tea.h"

struct net_responce_t
{
    size_t size;
    int net_status;
    char *raw_data;
    char *json_data;
};

struct tea_net_stats net_stats;
struct tea_server_info cur_server;
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

    /* Last 1 byte for NULL terminator */
    tmp = (char *) realloc(data->raw_data, data->size + 1);

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

    return n;
}

int curl_xferinfo(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    size_t *p = (size_t *) clientp;
    *p += ulnow;
    ++p;
    *p += dlnow;
    return CURLE_OK;
}

int net_send(const char *url, const char *body, size_t len, struct net_responce_t *receiver, int find_json)
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
        if(find_json)
        {
            char *p = receiver->raw_data;
            char *pe = p + receiver->size;
            for(; p < pe;)
            {
                if(*p == '{')
                {
                    receiver->json_data = p;
                    break;
                }
                ++p;
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

    if((result = net_send(cur_server.urls.url_msg_handler, jstr, _size, &input, 1)) && input.raw_data)
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

    if((net = net_send(cur_server.urls.url_msg_handler, json_serialized, _size, &input, 1)) && input.raw_data)
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
        if((net = net_send(cur_server.urls.url_auth, json_serialized, _size, &input, 1)) && input.raw_data)
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

    if((net = net_send(cur_server.urls.url_reg, json_serialized, _size, &input, 1)) && input.raw_data)
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

const char *tea_url_server()
{
    int s = -1;
    for(int x = 0; x < sizeof(env.servers) / sizeof(env.servers[0]); ++x)
    {
        if(strlen(env.servers[x]) == 0)
            break;
        if(env.active_server == tea_get_server_id(env.servers[x]))
        {
            s = x;
            break;
        }
    }

    if(s == -1)
    {
        return NULL;
    }

    return env.servers[s];
}

int tea_switch_server(int newServerID)
{
    env.active_server = newServerID;

    // init new server ID
    tea_read_urls(&cur_server.urls);
    tea_fetch_server();

    return 0;
}

int tea_fetch_server()
{
    int result;

    const char *server = tea_url_server();

    if(server != NULL)
    {
        struct net_responce_t nrwp;

        if((result = net_send(cur_server.urls.url_info, NULL, 0, &nrwp, 0)))
        {
            if(nrwp.raw_data != NULL)
            {
                int rfeatures = 0;
                sscanf(
                    nrwp.raw_data,
                    "%" SCNd8 ".%" SCNd8 ".%" SCNd8 "\n%s\n%s",
                    &cur_server.server_version.major,
                    &cur_server.server_version.minor,
                    &cur_server.server_version.patch,
                    &cur_server.maintainer,
                    &cur_server.license);

                /*
                 * 1.0.0: - no supported get first/last message id
                 *
                 * 1.1.0: - supported get first/last message id
                 *        - supported get server info
                 */

                // calculate supported version
                if(cur_server.server_version.major == 1)
                {
                    // MAJOR = 1

                    if(cur_server.server_version.minor == 1)
                    {
                        rfeatures |= SV_VERFI | SV_MSGFL;
                    }
                }

                if(cur_server.server_version.minor == 1)
                {
                    // MINOR = 0
                }

                if(cur_server.server_version.patch == 0)
                {
                    // PATCH = 1
                }

                // Set features
                cur_server.features = rfeatures;

                // put unused resource
                free(nrwp.raw_data);
            }
            else
                result = 0;
        }
    }
    else
        result = 0;

    if(!result)
        tea_log("Server fetch status is failed");

    return result;
}

void tea_read_urls(struct tea_server_urls *wrData)
{
    const char *server = tea_url_server();

    if(server == NULL)
    {
        tea_log("server == NULL");
        return;
    }

    int len = MIN(strlen(server), sizeof(wrData->url_base));
    strncpy(wrData->url_base, server, len);

    if(len >= (sizeof(wrData->url_base) - 1))
    {
        tea_log("Warn: Server string length biggested, maximum length 255");
    }
    else if(wrData->url_base[len - 1] != '/')
    {
        strncat(wrData->url_base, "/", MIN(len - 1, 1));
        tea_log("Server is no contains \"/\", inserted as default");
    }

    snprintf(wrData->url_auth, sizeof(wrData->url_auth), "%sapi/auth.php", server);
    snprintf(wrData->url_reg, sizeof(wrData->url_reg), "%sapi/register.php", server);
    snprintf(wrData->url_msg_handler, sizeof(wrData->url_msg_handler), "%sapi/messageHandler.php", server);
    snprintf(wrData->url_info, sizeof(wrData->url_info), "%sapi/TEA_SERVER_VERSION", server);
}
