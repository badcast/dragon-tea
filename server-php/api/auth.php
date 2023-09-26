<?php

require_once __DIR__ . '/sender.php';

/*
 Request Json Structure:
    user_id: The User Id (Unique Identification Number)

 Responce:
    status - the status of code
    authorized - the authorized result
    result - data logged
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
if ($data === null || !is_int($data->user_id)) {
    //uncorrect data
    send_auth_fail(STATUS_INVALID_REQUEST_DATA);
    exit;
} else {
    require_once __DIR__ . '/user.php';

    //User ID
    $uid = get_user($data->user_id);

    //if user not registered
    if ($uid == null) {
        send_auth_fail(STATUS_ID_NO_EXIST);
        exit;
    }

    //user found, and send information
    send_auth(true, STATUS_OK, $uid);
}
?>
