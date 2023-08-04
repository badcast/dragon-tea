<?php

require_once __DIR__ . '/sender.php';

/*
 Request Json Structure:
    user_nickname: The User NickName, DisplayName, or Name;

 Responce:
    status - the status of code
    authorized - the authorized result
    result - result new ID for created date
*/

if ($_SERVER["REQUEST_METHOD"] != "POST") {
    send_auth_fail(STATUS_INVALID_AUTH_METHOD);
    exit;
}

// Получаем JSON-строку из данных
$json_data = file_get_contents('php://input');

// Парсим JSON-строку
$data = json_decode($json_data);

// Проверяем, удалось ли распарсить JSON
if ($data === null || !is_string($data->user_nickname)) {
    //uncorrect data
    send_auth_fail(STATUS_INVALID_REQUEST_DATA);
    exit;
} else {
    if (!check_valid_nickname($data->user_nickname)) {
        send_auth_fail(STATUS_INVALID_NICKNAME);
        exit;
    }

    require_once __DIR__ . '/user.php';

    //User ID
    $uid = push_user($data->user_nickname);

    //if user not registered
    if ($uid == null) {
        send_auth_fail(STATUS_INVALID_REGISTER);
        exit;
    }

    //user found, and send information
    send_auth(true, STATUS_OK, $uid);
}
