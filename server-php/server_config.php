<?php
const MESSAGE_INDEXER_ENABLE = true;
const MESSAGE_DIRS = __DIR__ . "/messages";
const MESSAGE_FILE_EXT = ".tea";
const MESSAGE_FILE_CACHE_EXT = "_cache";
const MESSAGE_MAX_COUNT = 2048;
const PACK_FORMAT = "Q";
const WAITOUT_LOCK_MICROS = 10000;
const WAITOUT_TIME_CHANCES = 10;
const STR_MAX_NICKNAME = 16;
const STR_MAX_TEXT = 255;

// STATUS RESULT

const STATUS_OK = 0;
const STATUS_ID_NO_EXIST = 1;
const STATUS_INVALID_REQUEST_DATA = 2;
const STATUS_INVALID_AUTH_METHOD = 3;
const STATUS_INVALID_NICKNAME = 4;
const STATUS_INVALID_REGISTER = 5;
const STATUS_ADMIN_ACCOUNT_REACHABLE = 6;

const STATUS_PRIVATE_MESSAGE_NOT_SUPPORTED = 128;

const STATUS_INTERNAL_SERVER_ERROR = 500;