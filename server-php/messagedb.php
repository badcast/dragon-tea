<?php
require_once __DIR__ . '/api/sender.php';

const MESAGE_DIRS = __DIR__ . "/messages";
const MESSAGE_FILE_EXT = ".tea";

if (!isset($_SESSION["message_db_inited"])) {
    $_SESSION["message_db_inited"] = true;

    if (!file_exists(MESAGE_DIRS)) {
        // create directory for assocaiate container
        mkdir(MESAGE_DIRS);
    }
}

function message_read($uid_reader, $uid_reply, $msg_id_start, $max_messages)
{
    $msg_file = MESAGE_DIRS . "/" . $uid_reply->user_id . MESSAGE_FILE_EXT;
    $result = new stdClass();
    $result->length = 0;
    $result->messages = [];

    if (file_exists($msg_file)) {
        //Open file for read 
        $fd = fopen($msg_file, "r");
        if ($fd) {
            $lines = 1;
            //read lines
            while (($line = fgets($fd)) !== false) {
                if ($lines > $msg_id_start && ($line = json_decode($line)) !== null) {
                    if ($result->length >= $max_messages)
                        break;
                    $line->msg_id = $lines;
                    array_push($result->messages, $line);
                    ++$result->length;
                }
                $lines++;
            }
            fclose($fd);
            //calc length
        } else {
            $result = null;
        }
    }

    return $result;
}

function message_write($uid_writer, $uid_reply, $message_text)
{
    $msg_file = MESAGE_DIRS . "/" . $uid_reply->user_id . MESSAGE_FILE_EXT;
    //result 
    $result = null;

    //Open file for Append 
    $fd = fopen($msg_file, "a+");

    if ($fd) {
        $lock_while = 3; // 3 time chanses

        //while file not locked
        while (flock($fd, LOCK_SH) == false && --$lock_while != 0) sleep(1);

        if ($lock_while != -1) {
            //locked complete             
            $msg = new stdClass();
            $msg->time_received = $_SERVER['REQUEST_TIME'];
            $msg->time_saved = time();

            //get message line 
            $result = clone $msg;
            $result->msg_id = 1;

            rewind($fd); // set to begin 

            while (($line = fgets($fd)) !== false) {
                ++$result->msg_id;
            }

            $msg->send_user_id = $uid_writer->user_id;
            $msg->send_user_nickname = $uid_writer->user_nickname;
            $msg->message = $message_text;

            //Delim for message_output
            $message_data = json_encode($msg) . "\n";

            fwrite($fd, $message_data);

            //unlock file handle!
            flock($fd, LOCK_UN);
        }
        fclose($fd);
    }

    return $result;
}
