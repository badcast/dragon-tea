#include <curl/curl.h>

#include "tea.h"

struct NetworkBody
{
    size_t size;
    int net_status;
    char *data;
    char *json_data;
};

void net_init()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void net_free()
{
    curl_global_cleanup();
}

size_t net_write_data(void *ptr, size_t size, size_t nmemb, struct NetworkBody *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char *tmp;

    data->size += (size * nmemb);

    tmp = (char *) realloc(data->data, data->size + 1); /* +1 for '\0' */

    if(tmp)
    {
        data->data = tmp;
    }
    else
    {
        if(data->data)
        {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy(((char *) data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

int net_send(const char *url, const char *body, long len, struct NetworkBody *result)
{
    CURL *curl;
    CURLcode net;

    memset(result, 0, sizeof(struct NetworkBody));

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, net_write_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);
    #ifndef NDEBUG
    // timeout 10 seconds
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    #endif

    // SEND
    net = curl_easy_perform(curl);
    result->net_status = net;

    // clear curl data
    curl_easy_cleanup(curl);

    for(int x = 0; x < result->size; ++x)
    {
        if(result->data[x] == '{')
        {
            result->json_data = result->data + x;
            break;
        }
    }

    return net == CURLE_OK;
}

int net_api_read_messages(
    const struct tea_id_info *user,
    tea_id_t target_user_id,
    tea_id_t msg_id_start,
    int max_messages,
    struct tea_message_read_result *output)
{
    int result;
    size_t _size;
    json_object *json_request, *json_result, *jresult_obj, *jmessages, *jval;

    const char *jstr;
    struct NetworkBody input;
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

    if(result = net_send(TEA_SERVER_MESSAGE_HANDLER_URL, jstr, _size, &input) && input.data != NULL)
    {
        json_result = json_tokener_parse(input.data);
        // free result data
        free(input.data);

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
                for(size_t x = 0; x < _size; ++x)
                {
                    jmsg = json_object_array_get_idx(jmessages, x);
                    msg = &g_array_index(output->messages, struct tea_message_id, x);

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
    }
    else
    {
        output->status = TEA_STATUS_NETWORK_ERROR;
    }

    json_object_put(json_request);

    return result;
}

int net_api_write_message(
    const struct tea_id_info *user_sender, tea_id_t target_user_id, const char *message, int len, struct tea_message_send_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *json_result, *jresult_obj, *jval;

    const char *json_serialized;
    struct NetworkBody input;
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

    if(net = net_send(TEA_SERVER_MESSAGE_HANDLER_URL, json_serialized, _size, &input) && input.data != NULL)
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
        // free result data
        free(input.data);
    }
    else
    {
        output->status = TEA_STATUS_INTERNAL_SERVER_ERROR;
    }

    json_object_put(json_request);

    return net;
}

int net_api_signin(tea_id_t user_id, tea_login_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *jresult, *jval;
    const char *json_serialized;
    struct NetworkBody input;

    while(1)
    {
        json_request = json_object_new_object();
        json_object_object_add(json_request, "user_id", json_object_new_int64(user_id));

        json_serialized = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);
        if(net = net_send(TEA_SERVER_AUTH_URL, json_serialized, _size, &input) && input.data != NULL)
        {
            jresult = json_tokener_parse(input.json_data);
            // free result data
            free(input.data);

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
    return net;
}

int net_api_signup(const char *nickname, tea_register_result *output)
{
    int net;
    size_t _size;
    json_object *json_request, *jresult, *jval;
    const char *json_serialized;
    struct NetworkBody input;

    json_request = json_object_new_object();
    json_object_object_add(json_request, "user_nickname", json_object_new_string(nickname));

    json_serialized = json_object_to_json_string_length(json_request, JSON_C_TO_STRING_PLAIN, &_size);

    if(net = net_send(TEA_SERVER_REGISTER_URL, json_serialized, _size, &input) && input.data != NULL)
    {
        jresult = json_tokener_parse(input.json_data);
        // free result data
        free(input.data);

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
    return net;
}
