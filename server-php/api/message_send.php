<?php

require __DIR__ . '/user.php';

/*
 The Script for send message
 Request:
    user_id: The User Id (Unique Identification Number)
    user_nickname: The User Nickname
    message: The text message

 Responce:
    status: the status of code

    result: message information
        - time: The time of registered
*/

if($_SERVER["REQUEST_METHOD"] != "POST")
{
    send_auth_fail(STATUS_INVALID_AUTH_METHOD);
}

// Получаем JSON-строку из данных
$json_data = file_get_contents('php://input');

// Парсим JSON-строку
$data = json_decode($json_data);

// Проверяем, удалось ли распарсить JSON
if ($data === null)
{
    //uncorrect data
    send_auth_fail(STATUS_INVALID_REQUEST_DATA);
}
else
{
    //User ID
    $uid = get_user($data->user_id);

    //if user not registered
    if($uid == null){
        send_auth_fail(STATUS_ID_NO_EXIST);
    }

    //user found, and send information
    send(true, STATUS_OK, $uid);
}
?>