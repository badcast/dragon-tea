<?php
require_once __DIR__ . '/api/sender.php';

if (!isset($_SESSION["message_db_inited"])) {
    $_SESSION["message_db_inited"] = true;

    if (!file_exists(MESSAGE_DIRS)) {
        // create directory for associate container
        mkdir(MESSAGE_DIRS);
    }
}

function message_first_id($uid_reader, $uid_target)
{
    //TODO: Get First Message ID 
}

function message_last_id($uid_reader, $uid_target)
{
    //TODO: Get Last Message ID
    $msg_file = MESSAGE_DIRS . "/" . $uid_target->user_id . MESSAGE_FILE_EXT;
    $msg_id = -1; 

    for(;;){
        if(MESSAGE_INDEXER_ENABLE)
        {
            $msg_cache_file = $msg_file . MESSAGE_FILE_CACHE_EXT;
           
            if(!file_exists($msg_cache_file) ||  !($fd = fopen($msg_cache_file, "rb")))
            {
                break;
            }
            
            // read from cached 
            fseek($fd, PHP_INT_SIZE, SEEK_END); 
            $flen = ftell($fd); 


            break; 
        }
        break; 
    }
    
    return $msg_id;
}

function message_read($uid_reader, $uid_target, $msg_id_start, $max_messages = -1)
{

    //check valid $msg_id
    if (is_int($msg_id_start) == false || $msg_id_start < 0) {
        return null;
    }

    $msg_file = MESSAGE_DIRS . "/" . $uid_target->user_id . MESSAGE_FILE_EXT;
    $msg_cache_file = $msg_file . MESSAGE_FILE_CACHE_EXT;

    $result = new stdClass();
    $result->messages = [];
    $result->length = 0;

    while ($max_messages != 0) {

        if (!file_exists($msg_file) || !MESSAGE_INDEXER_ENABLE) {
            //Remove cache_file then, message_file not exists or message_indexer turned off
            unlink($msg_cache_file);
        }

        if ($max_messages < 0 || $max_messages > MESSAGE_MAX_COUNT)
            $max_messages = MESSAGE_MAX_COUNT;

        //cache exist ? Load from cache 
        if (MESSAGE_INDEXER_ENABLE && file_exists($msg_cache_file)) {
            // $fd_index - The Message ID Cache
            $fd_index = fopen($msg_cache_file, "rb");
            if ($fd_index) {
                // $fd_mesg - The message id 
                $fd_mesg = fopen($msg_file, "r");
                if ($fd_mesg) {

                    fseek($fd_mesg, 0, SEEK_END);
                    $filelen = ftell($fd_mesg);
                    fseek($fd_index, -PHP_INT_SIZE, SEEK_END);
                    $flenw = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1];
                    
                    // Check filelength with last msg_id_len
                    if ($filelen === $flenw) {
                        //set to begin
                        fseek($fd_index, 0, SEEK_SET);

                        // Message Lines 
                        $jmsg_id_max = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1]; // int read 
                        // Message Offset ID
                        $jmsg_id_start = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1]; // int read 
                        // Relative Message Max ID
                        $relative_id_max = $jmsg_id_start + $jmsg_id_max; // relative ID

                        //Economy iterations, msg_id_start greather relative_id_max
                        if ($msg_id_start < $relative_id_max) {
                            // Установка позиций к начальному msg_id 
                            fseek($fd_index, PHP_INT_SIZE * 2 * $msg_id_start, SEEK_CUR);

                            for ($iter_id = $msg_id_start; $result->length < $max_messages && $iter_id < $relative_id_max; ++$iter_id) {
                                //Прочитать данные из cache_file 

                                //read message position 
                                $fmsg_pos = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1]; // int read 
                                //read message position + flength
                                $fmsg_len = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1]; // int read 

                                //Установить курсор прочитанное из cache_file 
                                fseek($fd_mesg, $fmsg_pos, SEEK_SET);

                                // Чтение одной строки до '\n'
                                $jdata = fread($fd_mesg, $fmsg_len - $fmsg_pos);
                                if ($jdata === false || ($json_dec = json_decode($jdata)) === null) {
                                    break; // fail 
                                }

                                $json_dec->msg_id = $iter_id + 1;

                                $result->messages[] = $json_dec;
                                $result->length++;
                            }
                        }
                    }
                    else {
                        fclose($fd_index);
                        unlink($msg_cache_file);
                    }
                    fclose($fd_mesg);
                }
                fclose($fd_index);
            }
            //Cache no exist, then create and load
        } else {
            $fd_cache = fopen($msg_cache_file, "wb+");

            if ($fd_cache) {
                $lock_while = WAITOUT_TIME_CHANCES;

                //while file not locked, wait for complete other I/O
                while (flock($fd_cache, LOCK_SH) == false && --$lock_while != 0) usleep(WAITOUT_LOCK_MICROS);

                if ($lock_while != 0) {
                    //Lock complete

                    $fdm = fopen($msg_file, "r");
                    if ($fdm) {
                        //Header 
                        $fmsg_id = 0;
                        $ftotal_len = 0;
                        fseek($fd_cache, PHP_INT_SIZE * 2);
                        while (($line = fgets($fdm)) !== false) {
                            $fmsg_id++;

                            if ($msg_id_start < $fmsg_id && $result->length <= $max_messages) {
                                $msg = json_decode($line);
                                $msg->msg_id = $fmsg_id;
                                $result->messages[] = $msg;
                                $result->length++;
                            }

                            //write message position 
                            fwrite($fd_cache, pack(PACK_FORMAT, $ftotal_len));
                            //write message position + flength
                            fwrite($fd_cache, pack(PACK_FORMAT, ($ftotal_len += mb_strlen($line))));
                        }
                        fclose($fdm);

                        //Set to header position 
                        fseek($fd_cache, 0, SEEK_SET);

                        //Update header 

                        //write lines 
                        fwrite($fd_cache, pack(PACK_FORMAT, $fmsg_id));
                        //write offset message id
                        fwrite($fd_cache, pack(PACK_FORMAT, 0));
                    }

                    flock($fd_cache, LOCK_UN);
                }
                fclose($fd_cache);
            }
        }
        break;
    }
    return $result;
}

function message_write($uid_writer, $uid_reply, $message_text)
{
    $msg_file = MESSAGE_DIRS . "/" . $uid_reply->user_id . MESSAGE_FILE_EXT;
    $msg_cache_file = $msg_file . MESSAGE_FILE_CACHE_EXT;
    //result 
    $result = null;

    //Open file for Append 
    $fd = fopen($msg_file, "a+");

    if ($fd) {
        $lock_while = WAITOUT_TIME_CHANCES; // time chanses

        //while file not locked, wait for complete other I/O
        while (flock($fd, LOCK_SH) == false && --$lock_while != 0) usleep(WAITOUT_LOCK_MICROS);

        if ($lock_while != 0) {
            //locked complete             
            $msg = new stdClass();
            $msg->time_received = $_SERVER['REQUEST_TIME'];
            $msg->time_saved = time();

            //get message line 
            $result = clone $msg;
            $result->msg_id = 1; // default msg ID

            //calc msg id for current 

            $msg->send_user_id = $uid_writer->user_id;
            $msg->send_user_nickname = $uid_writer->user_nickname;
            $msg->message = $message_text;
            $msg_string = json_encode($msg) . "\n";

            if (MESSAGE_INDEXER_ENABLE && file_exists($msg_cache_file)) {
                //has cache file 
                $fd_index = fopen($msg_cache_file, "rb+");
                if ($fd_index) {
                    $lock_while = WAITOUT_TIME_CHANCES; // time chanses

                    //while file not locked, wait for complete other I/O
                    while (flock($fd_index, LOCK_SH) == false && --$lock_while != 0) usleep(WAITOUT_LOCK_MICROS);

                    if ($lock_while != -1) {
                        $msg_ids = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1];
                        $msg_ids_offset = unpack(PACK_FORMAT, fread($fd_index, PHP_INT_SIZE))[1];

                        $result->msg_id = $msg_ids + $msg_ids_offset + 1; // set MsgID 

                        //seek end  
                        fseek($fd_index, 0, SEEK_END);

                        $tea_last_len = filesize($msg_file);

                        //Update message position 
                        fwrite($fd_index, pack(PACK_FORMAT, $tea_last_len));
                        //Update message position + flength
                        fwrite($fd_index, pack(PACK_FORMAT, $tea_last_len + mb_strlen($msg_string)));

                        //Update head index 
                        fseek($fd_index, 0, SEEK_SET);
                        fwrite($fd_index, pack(PACK_FORMAT, $msg_ids + 1));

                        flock($fd_index, LOCK_UN);
                    }
                    fclose($fd_index);
                }
            } else {
                rewind($fd); // set to begin 
                //calc message_ids custimly O(N) - operation 
                //TODO: Optimize here with: message_last_id() 
                while (($line = fgets($fd)) !== false) {
                    ++$result->msg_id;
                }
            }

            //Delim for message_output
            // write message
            fwrite($fd, $msg_string);

            //unlock file handle!
            flock($fd, LOCK_UN);
        }
        fclose($fd);
    }

    return $result;
}
