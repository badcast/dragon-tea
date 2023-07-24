<?php

require __DIR__ . '/sender.php';

// This is script for data instrumental
// Get and Set UserInformation

const USER_DIRS = __DIR__ . "/../users";
const USER_FILE_EXT = ".id";

$INITED = false;
$USERS = []; // This is Associate Array

function init_users(){
    global $INITED, $USERS;

    if($INITED == true)
       return;

    if(!file_exists(USER_DIRS))
    {
        mkdir(USER_DIRS);
        // create directory for assocaiate container
    }
    else
    {
        internal_load();
    }

    $INITED = true;
}

function internal_save() {
    global $USERS;

    //TODO: save

    foreach($USERS as $id => $user)
    {
        if($user->changed)
        {
            // set file name
            $filename = USER_DIRS . '/' . $id . '.id';
            $filename_backup = $filename . ".bak";
            if(file_exists($filename))
            {
                if(file_exists($filename_backup)){
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

            if($fd == null)
                continue;

            if(flock($fd, LOCK_EX)){
                fwrite($fd, json_encode($user));
                flock($fd, LOCK_UN);
            }

            fclose($fd);
        }
    }
}

function internal_load() {
    global $USERS;
    //get files
    $files = scandir(USER_DIRS);

    //Remove [.] [..] (current, parent) link
    $files = array_diff($files, array('.','..'));

    foreach($files as $file)
    {
        if(strcmp(substr($file,strlen($file)-strlen(USER_FILE_EXT)), USER_FILE_EXT))
        {
            continue;
        }
        //Up location to Absolute
        $contents = file_get_contents(USER_DIRS . '/' . $file);
        $contents = json_decode($contents);
        $id = $contents->user_id;
        if(isset($USERS[$id]))
        {
            // Has Conflict
            // die("Conflict User ID [". $contents->user_id ."]");
            send_auth_fail(STATUS_INTERNAL_SERVER_ERROR);
        }
        $USERS[$id] = $contents;
        $USERS[$id]->changed = false;
    }
}

function get_user($id)
{
    global $USERS;

    if(!is_int($id))
        return null;

    $get_id = $USERS[$id];
    if($get_id){
        $get_id->last_login = time();
        $get_id->changed = true;
        internal_save();
    }

    return $get_id;
}

function push_user($user_nickname)
{
    global $USERS;

    $time = time(); // get creation date
    $newID = -1;
    do
    {
        //0 .. 127 - reserved
        $newID = random_int(128,PHP_INT_MAX);
    }while(isset($USERS[$newID]));

    //push new user
    $USERS[$newID] = new stdClass();
    $USERS[$newID]->user_id = $newID;
    $USERS[$newID]->changed = true;
    $USERS[$newID]->creation_date = $time;
    $USERS[$newID]->last_login = $time;
    $USERS[$newID]->user_nickname = $user_nickname;

    internal_save(); // save state

    return $USERS[$newID];
}

function pop_user()
{

}

init_users();

?>
