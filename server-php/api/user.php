<?php

require_once __DIR__ . '/sender.php';

// This is script for data instrumental
// Get and Set UserInformation

const USER_DIRS = __DIR__ . "/../users";
const USER_FILE_EXT = ".id";

$USERS = []; // This is Associate Array
$CHANGED_USERS = [];

function init_users()
{
    global $USERS, $_SESSION;

    if (isset($_SESSION["inited"]))
        return;

    if (!file_exists(USER_DIRS)) {
        // create directory for assocaiate container
        mkdir(USER_DIRS);
    } else {
        internal_load();
    }

    $_SESSION["inited"]=true;
}

function internal_save()
{
    global $USERS, $CHANGED_USERS;

    foreach ($CHANGED_USERS as $id) {
        $user = $USERS[$id];

        // set file name
        $filename = USER_DIRS . '/' . $id . '.id';
        $filename_backup = $filename . ".bak";
        if (file_exists($filename)) {
            if (file_exists($filename_backup)) {
                // DELETE OLD BACKUP
                unlink($filename_backup);
            }
            // move last filename to backup
            rename($filename, $filename_backup);
        }

        //remove key [unuse]
        unset($user->changed);

        // save to new filename (json)
        $fd = fopen($filename, "w");

        if ($fd == null)
            continue;

        if (flock($fd, LOCK_SH)) {
            fwrite($fd, json_encode($user));
            flock($fd, LOCK_UN);
        }

        fclose($fd);
    }
    $CHANGED_USERS=[]; 
}

function internal_load()
{
    global $USERS;
    //get files
    $files = scandir(USER_DIRS);

    //Remove [.] [..] (current, parent) link
    $files = array_diff($files, array('.', '..'));

    foreach ($files as $file) {
        if (strcmp(substr($file, strlen($file) - strlen(USER_FILE_EXT)), USER_FILE_EXT)) {
            continue;
        }
        //Up location to Absolute
        $contents = file_get_contents(USER_DIRS . '/' . $file);
        $contents = json_decode($contents);
        $id = $contents->user_id;
        if (isset($USERS[$id])) {
            // Has Conflict
            // die("Conflict User ID [". $contents->user_id ."]");
            send(STATUS_INTERNAL_SERVER_ERROR);
            exit;
        }
        $USERS[$id] = $contents;
    }
}

function get_user_ids()
{
    global $USERS;
    $ids = array();
    foreach ($USERS as $user) {
        array_push($ids, $user->user_id);
    }
    return $ids;
}

function get_user($id, $change_last_login = true)
{
    global $USERS, $CHANGED_USERS;

    if (!is_int($id))
        return null;

    $get_id = $USERS[$id];
    if ($get_id && $change_last_login) {
        $get_id->last_login = time();
        $CHANGED_USERS[] = $get_id->user_id;
        internal_save();
    }

    return $get_id;
}

function push_user($user_nickname)
{
    global $USERS,$CHANGED_USERS;

    $time = time(); // get creation date
    $newID = -1;
    do {
        //0 .. 127 - reserved
        $newID = random_int(128, PHP_INT_MAX);
    } while (isset($USERS[$newID]));

    //push new user
    $USERS[$newID] = new stdClass();
    $USERS[$newID]->user_id = $newID;
    $USERS[$newID]->creation_date = $time;
    $USERS[$newID]->last_login = $time;
    $USERS[$newID]->user_nickname = $user_nickname;
    $CHANGED_USERS[] = $newID;
    
    internal_save(); // save state

    return $USERS[$newID];
}

function pop_user($user_id)
{
    global $USERS,$CHANGED_USERS;
    if ($user_id <= 127) {
        return null;
    }

    if (isset($USERS[$user_id]) == false) {
        return false;
    }

    //remove user filename
    $file = USER_DIRS . '/' . $user_id . '.id';
    if (file_exists($file)) {
        if (unlink($file) == false) {
            //If not remove, then move to as unremoved-mark 
            rename($file, $file . '.unremoved');
        }
    }
    //remove from array 
    unset($USERS[$user_id]);
    unset($CHANGED_USERS[$user_id]);
    return true;
}

init_users();
