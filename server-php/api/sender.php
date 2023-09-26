<?php

require_once __DIR__ . '/../server_config.php';


function check_valid_nickname($nickname)
{
    // Проверяем длину имени пользователя
    $len = mb_strlen($nickname);
    if ($nickname == null || $len < 3 || $len > 16) {
        return false;
    }

    // Проверяем, что имя пользователя содержит только алфавиты (латинские и кириллические)
    // и цифры, также что оно не начинается с цифры
    if (!preg_match('/^[a-zA-Zа-яА-Я][a-zA-Zа-яА-Я0-9]*$/u', $nickname)) {
        return false;
    }

    return true;
}
function send_json($data)
{
    header("HTTP/1.1 200 OK");
    header("Content-Type: application/json");
    echo json_encode($data);
}

function send($status, $result = null)
{
    $json = new stdClass();
    $json->status = $status;
    $json->result = $result;
    send_json($json);
}

function send_auth($authorized, $status, $result = null)
{
    $json = new stdClass();
    $json->authorized = $authorized;
    $json->status = $status;
    $json->result = $result;
    send_json($json);
}

function send_auth_fail($status)
{
    send_auth(false, $status);
}

function send_reg_fail($status)
{
    send_auth(false, $status);
}
