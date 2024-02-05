<?php

require_once __DIR__ . '/sender.php';

/*
 Request: 
    user_id: The User ID

 Responce:
    error: the status of code
    result: deleted account from server 
*/

if ($_SERVER["REQUEST_METHOD"] != "POST") {
    send(STATUS_INVALID_AUTH_METHOD);
    exit;
}

// Получаем JSON-строку из данных
$json_data = file_get_contents('php://input');

// Парсим JSON-строку
$data = json_decode($json_data);
$request->user_id = patch_convert_id($request->user_id);

// Проверяем, удалось ли распарсить JSON
if ($data === null || !is_int($data->user_id)) {
    //uncorrect data
    send(STATUS_INVALID_REQUEST_DATA);
    exit;
} else {
    require_once __DIR__ . '/user.php';

    //User ID
    $user_del_status = pop_user($data->user_id);

    if ($user_del_status === false) {
        send(STATUS_ID_NO_EXIST, false);
        exit;
    }
    else
    if ($user_del_status === null) {
        send(STATUS_ADMIN_ACCOUNT_REACHABLE, false);
        exit;
    }

    //user found, and send information
    send(STATUS_OK, true);
}
?>
