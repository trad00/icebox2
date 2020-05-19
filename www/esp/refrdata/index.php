<?php
$db = array(
    '14732353' => ''
);

function put_log($msg, $echo = false) {
    $log = date('Y-m-d H:i:s') . ': ' . print_r($msg, true);
    file_put_contents('log.txt', $log . PHP_EOL, FILE_APPEND);
    if ($echo) {
        echo $log . "\n";
    }
}

if(!isset($db[$_SERVER['HTTP_X_ESP8266_CHIP_ID']])) {
    header($_SERVER['SERVER_PROTOCOL'].' 500 Error', true, 500);
    exit();
}

//Получаем данные
$json = json_decode(file_get_contents('php://input'));
$chip_id = $_SERVER['HTTP_X_ESP8266_CHIP_ID'];

//Отвечаем клиенту
header($_SERVER['SERVER_PROTOCOL'] . ' 200 OK', true, 200);
echo('Ok');

//И продолжаем обработку данных асинхронно
//fastcgi_finish_request();

$uri_parts = explode('?', $_SERVER['REQUEST_URI'], 2);
$process_url = 'http://' . $_SERVER['HTTP_HOST'] . $uri_parts[0];
$process_url .= 'process_data.php?chip_id=' . $chip_id . '&cnt=' . $json->cnt;
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $process_url);
curl_setopt($ch, CURLOPT_FRESH_CONNECT, true);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 50);
curl_exec($ch);
curl_close($ch);

?>
