<?php
require_once __DIR__ . '/user.php';

/*
 Responce:
    status - the status of code
    result - users (List IDs)
*/

//User ID
$users = get_user_ids();

//user found, and send information
send(STATUS_OK, $users);

?>