<?php
const STATUS_OK = 0;
const STATUS_ID_NO_EXIST = 1;
const STATUS_INVALID_REQUEST_DATA = 2;
const STATUS_INVALID_AUTH_METHOD = 3;
const STATUS_INVALID_NICKNAME = 4;
const STATUS_INVALID_REGISTER = 5;

const STATUS_INTERNAL_SERVER_ERROR = 500;

const STR_MAX_NICKNAME = 16;
const STR_MAX_TEXT = 128;

function check_valid_nickname($nickname) {
    // Проверяем длину имени пользователя
    if ($nickname == null || mb_strlen($nickname) < 3 || mb_strlen($nickname) > 16) {
        return false;
    }

    // Проверяем, что имя пользователя содержит только алфавиты (латинские и кириллические)
    // и цифры, также что оно не начинается с цифры
    if (!preg_match('/^[a-zA-Zа-яА-Я][a-zA-Zа-яА-Я0-9]*$/u', $nickname)) {
        return false;
    }

    return true;
}
function send_json($data) {
    header("HTTP/1.1 200 OK");
    header("Content-Type: application/json");
    echo json_encode($data);
}

function send_message_reply($status, $message_data) {

}

function send($authorized, $status, $result = null) {
    $json = new stdClass();
    $json->authorized = $authorized;
    $json->error = $status;
    $json->result = $result;
    send_json($json);
}

function send_auth_fail($status) {
    send(false, $status);
    exit(-1);
}

function send_reg_fail($status) {
    send(false, $status);
    exit(-1);
}
?>