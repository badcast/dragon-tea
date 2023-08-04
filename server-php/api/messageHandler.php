<?php
require_once __DIR__ . '/sender.php';

/*
 The Script for read/send messages communication system

 -- Read Message Handler
 Request:
    type: The Type is Read - for use = reader
    user_id: The User Id (Unique Identification Number)
    user_nickname: The User Nickname
    msg_id: The Message ID (line)
    max_messages: The Maximum messages overload

 Responce:
    status: the status of code

    result: message information
        - messages[N]
          [0]
          {
                user_id: - The sender User ID
                user_nickname: The sender User Nickname
                message: The text of the sender
                time: The message time
          }
          [N]
        
 -- Send Message Handler
 Request:
    type: The Type is Write - for use = writer
    user_id: The User Id (Unique Identification Number)
    user_nickname: The User Nickname
    target_user_id: The User ID to send
    message: The text message

 Responce:
    status: the status of code
    result: message applyed info
        - time_received: the time received 
        - time_saved: the time saved in server 
        - message_id: the message id, for line 
*/

//TODO: Next use Private Message (for target user)

if ($_SERVER["REQUEST_METHOD"] != "POST") {
    send(STATUS_INVALID_AUTH_METHOD);
    exit;
}

// Получаем JSON-строку из данных
$json_data = file_get_contents('php://input');

// Парсим JSON-строку
$request = json_decode($json_data);

// Проверяем, удалось ли распарсить JSON
if (
    $request === null ||
    !is_int($request->user_id) ||
    !is_int($request->target_user_id)
) {
    //uncorrect data
    send(STATUS_INVALID_REQUEST_DATA);
    exit;
} else {

    $FLAG = -1;
    if (strcmp($request->type, "reader") == 0 && is_int($request->msg_id)  && $request->msg_id > -1) {
        $FLAG = 0;
    } else if (strcmp($request->type, "writer") == 0 && is_string($request->message) && mb_strlen($request->message) <= STR_MAX_TEXT) {
        $FLAG = 1;
    } else {
        send(STATUS_INVALID_REQUEST_DATA);
        exit;
    }

    if ($request->target_user_id !== -1) {
        send(STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED);
        exit;
    }

    require_once __DIR__ . '/user.php';

    //Get User ID
    $uid = get_user($request->user_id, false);

    //if user not registered
    if ($uid === null || strcmp($uid->user_nickname, $request->user_nickname)) {
        send(STATUS_ID_NO_EXIST);
        exit;
    }

    if ($request->target_user_id === -1) {
        $target_id = new stdClass();
        $target_id->user_id = -1;
        $target_id->user_nickname = "public";
    } else {
        $target_id = get_user($request->target_user_id, false);
    }

    require_once __DIR__ . '/../messagedb.php';

    switch ($FLAG) {
            //On Read
        case 0:
            if ($request->max_messages == null) {
                $request->max_messages = -1;
            }
            $result = message_read($uid, $target_id, $request->msg_id, $request->max_messages);
            break;
            // On Write
        case 1:
            $result = message_write($uid,  $target_id, $request->message);
            break;
    }

    if ($result === null)
        $status = STATUS_INTERNAL_SERVER_ERROR;
    else
        $status = STATUS_OK;

    send($status, $result);
}
