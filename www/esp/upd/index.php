<?php

$db = array(
    "14732353" => "icebox2.ino.nodemcu.bin"
);


function put_log($msg, $echo = false) {
    $log = date('Y-m-d H:i:s') . ': ' . print_r($msg, true);
    file_put_contents('log.txt', $log . PHP_EOL, FILE_APPEND);
    if ($echo) {
        echo $log . "\n";
    }
}

//put_log(getallheaders());
//put_log($_SERVER);

header('Content-type: text/plain; charset=utf8', true);
 
function check_header($name, $value = false) {
    if(!isset($_SERVER[$name])) {
        return false;
    }
    if($value && $_SERVER[$name] != $value) {
        return false;
    }
    return true;
}

function sendFile($path) {
    header($_SERVER["SERVER_PROTOCOL"].' 200 OK', true, 200);
    header('Content-Type: application/octet-stream', true);
    header('Content-Disposition: attachment; filename='.basename($path));
    header('Content-Length: '.filesize($path), true);
    header('Cache-Control: no-cache', true);
    header('x-MD5: '.md5_file($path), true);
    readfile($path);
}

if(!check_header('HTTP_USER_AGENT', 'ESP8266-http-Update')) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    put_log("only for ESP8266 updater! (user agent)", true);
    exit();
}

if(
    !check_header('HTTP_X_ESP8266_CHIP_ID') ||
    !check_header('HTTP_X_ESP8266_SKETCH_MD5') ||
    !check_header('HTTP_X_ESP8266_SDK_VERSION') ||
    !check_header('HTTP_X_ESP8266_MODE')
) {
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    put_log("only for ESP8266 updater! (header)", true);
    exit();
}

//HTTP_X_ESP8266_MODE: sketch / spiffs

if(!isset($db[$_SERVER['HTTP_X_ESP8266_CHIP_ID']])) {
    header($_SERVER["SERVER_PROTOCOL"].' 500 ESP Chip-ID not configured for updates', true, 500);
    put_log("ESP Chip-ID not configured for updates" . $_SERVER['HTTP_X_ESP8266_CHIP_ID'], true);
    exit();
}

$localBinary = $db[$_SERVER['HTTP_X_ESP8266_CHIP_ID']];

//put_log($_SERVER["HTTP_X_ESP8266_SKETCH_MD5"]);
//put_log(md5_file($localBinary));

if(file_exists($localBinary) && $_SERVER["HTTP_X_ESP8266_SKETCH_MD5"] != md5_file($localBinary)) {
    sendFile($localBinary);
    put_log("CHIP_ID " . $_SERVER['HTTP_X_ESP8266_CHIP_ID']);
    put_log("sendFile " . $localBinary);
} else {
    header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
    put_log("CHIP_ID " . $_SERVER['HTTP_X_ESP8266_CHIP_ID']);
    put_log("Not Modified");
}
?>
