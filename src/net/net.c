#include "tea.h"

struct NetworkBody
{
    size_t size;
    char *data;
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
    CURLcode res;

    memset(result, 0, sizeof(*result));

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, result);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, net_write_data);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, len);

    // SEND
    res = curl_easy_perform(curl);

    // clear curl data
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

int net_get_result(const struct NetworkBody *input)
{
    return 0;
}

int net_api_signin(long user_id, struct tea_login_result *output)
{
    int res = 0;
    json_object *jdata, *jresult, *jval;
    const char *json_result;
    struct NetworkBody input;

    jdata = json_object_new_object();
    json_object_object_add(jdata, "user_id", json_object_new_int64(user_id));

    json_result = json_object_to_json_string_ext(jdata, JSON_C_TO_STRING_PLAIN);

    if(res = net_send(TEA_SERVER_AUTH_URL, json_result, strlen(json_result), &input))
    {
        jresult = json_tokener_parse(input.data);

        json_object_object_get_ex(jresult, "error", &jval);
        output->error = json_object_get_int(jval);

        json_object_object_get_ex(jresult, "authorized", &jval);
        if(res = output->authorized = json_object_get_boolean(jval))
        {
            json_object *jresult_obj;
            json_object_object_get_ex(jresult, "result", &jresult_obj);

            json_object_object_get_ex(jresult_obj, "user_id", &jval);
            output->result.user_id = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "user_nickname", &jval);
            json_result = json_object_get_string(jval);
            strncpy(output->result.user_nickname, json_result, TEA_MAXLEN_USERNAME);

            json_object_object_get_ex(jresult_obj, "creation_date", &jval);
            output->result.creation_date = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "last_login", &jval);
            output->result.last_login = json_object_get_int64(jval);
        }

        // free json resource
        json_object_put(jresult);
        // free result data
        free(input.data);
    }
    else
    {
        output->error = TEA_STATUS_NETWORK_ERROR;
    }
    // free json resource
    json_object_put(jdata);
    return res;
}

int net_api_signup(const char *nickname, tea_register_result *output)
{
    int res = 0;
    json_object *jdata, *jresult, *jval;
    const char *json_result;
    struct NetworkBody input;

    jdata = json_object_new_object();
    json_object_object_add(jdata, "user_nickname", json_object_new_string(nickname));

    json_result = json_object_to_json_string_ext(jdata, JSON_C_TO_STRING_PLAIN);

    if(res = net_send(TEA_SERVER_REGISTER_URL, json_result, strlen(json_result), &input))
    {
        jresult = json_tokener_parse(input.data);

        json_object_object_get_ex(jresult, "error", &jval);
        output->error = json_object_get_int(jval);

        json_object_object_get_ex(jresult, "authorized", &jval);
        if(res = output->authorized = json_object_get_boolean(jval))
        {
            json_object *jresult_obj;
            json_object_object_get_ex(jresult, "result", &jresult_obj);

            json_object_object_get_ex(jresult_obj, "user_id", &jval);
            output->result.user_id = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "user_nickname", &jval);
            json_result = json_object_get_string(jval);
            strncpy(output->result.user_nickname, json_result, TEA_MAXLEN_USERNAME);

            json_object_object_get_ex(jresult_obj, "creation_date", &jval);
            output->result.creation_date = json_object_get_int64(jval);

            json_object_object_get_ex(jresult_obj, "last_login", &jval);
            output->result.last_login = json_object_get_int64(jval);
        }

        // free json resource
        json_object_put(jresult);
        // free result data
        free(input.data);
    }
    else
    {
        output->error = TEA_STATUS_NETWORK_ERROR;
    }

    // free json resource
    json_object_put(jdata);
    return res;
}
