<?php

require_once __DIR__ . '/../server_config.php';

require_once __DIR__ . '/base_functional.php';

function send_json($data)
{
    header("HTTP/1.1 200 OK");
    header("Content-Type: application/json");
    header("Access-Control-Allow-Origin: *");
    echo json_encode($data);
}

function send($status, $result = null)
{
    $json = new stdClass();
    $json->status = $status;
    $json->result = $result;
    send_json($json);
}

function send_auth($authorized, $status, $result = null)
{
    $json = new stdClass();
    $json->authorized = $authorized;
    $json->status = $status;
    $json->result = $result;
    send_json($json);
}

function send_auth_fail($status)
{
    send_auth(false, $status);
}

function send_reg_fail($status)
{
    send_auth(false, $status);
}
