<?php
// Message indexer for supress speed
const MESSAGE_INDEXER_ENABLE = true;
// Messages dirs for read and write (I/O)
const MESSAGE_DIRS = __DIR__ . "/messages";
// Message ext
const MESSAGE_FILE_EXT = ".tea";
// Message cache ext
const MESSAGE_FILE_CACHE_EXT = "_cache";
// Max message per request
const MESSAGE_MAX_COUNT = 2048;
// Lock on I/O
const WAITOUT_LOCK_MICROS = 10000;
// Lock chances after delay micros
const WAITOUT_TIME_CHANCES = 10;
// Max nicknamae length
const STR_MAX_NICKNAME = 16;
// Max message string length
const STR_MAX_TEXT = 255;

const PACK_FORMAT = "Q";

// STATUS RESULT

// Status with continue
const STATUS_OK = 0;
// User ID not exists or invalid id
const STATUS_ID_NO_EXIST = 1;
// Invalid request data
const STATUS_INVALID_REQUEST_DATA = 2;
// Invalid auth method, support POST, GET, etc.
const STATUS_INVALID_AUTH_METHOD = 3;
// Invalid nickname, exists characters
const STATUS_INVALID_NICKNAME = 4;
// Invalid register on server
const STATUS_INVALID_REGISTER = 5;
// Admin account is no reachable
const STATUS_ADMIN_ACCOUNT_REACHABLE = 6;
// Private message is not support by this server
const STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED = 128;
// Internal Server Error (bad code)
const STATUS_INTERNAL_SERVER_ERROR = 500;
