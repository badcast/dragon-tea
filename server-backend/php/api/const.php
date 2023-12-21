<?php

require_once __DIR__ . '/../server_config.php';
require_once __DIR__ . '/sender.php';

if ($_SERVER["REQUEST_METHOD"] != "GET") {
    send_auth_fail(STATUS_INVALID_AUTH_METHOD);
    exit;
}

$server_config = new stdClass();
$server_config->waitoutLock = WAITOUT_LOCK_MICROS;
$server_config->waitoutTimeChances = WAITOUT_TIME_CHANCES;
$server_config->strMaxNickname = STR_MAX_NICKNAME;
$server_config->strMaxMessage = STR_MAX_TEXT;
$server_config->msgMaxCount = MESSAGE_MAX_COUNT;
$server_config->msgIndexerSupported = true; 
$server_config->msgIndexerEnable =  MESSAGE_INDEXER_ENABLE;
$server_config->sysOS = PHP_OS;
$server_config->sysInfo = php_uname();
$server_config->serverVer = file_get_contents("TEA_SERVER_VERSION");
$server_config->diskTotalSpace = disk_total_space(__DIR__);
$server_config->diskFreeSpace = disk_free_space(__DIR__);
$server_config->welcome = file_get_contents("WELCOME.txt");

// LIFETIME
$_sess = $server_config->session = new stdClass(); 
$_sess->name = ini_get("session.name"); 
$_sess->lifetime = ini_get("session.gc_maxlifetime");

// MEMORY
$_mem = $server_config->memory = new stdClass(); 
$_mem->mem_usage = memory_get_usage();
$_mem->mem_peak_usage = memory_get_peak_usage();

//user found, and send information
send(STATUS_OK, $server_config);
