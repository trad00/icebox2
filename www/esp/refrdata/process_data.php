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


$chip_id = $_GET['chip_id'];
$cnt = $_GET['cnt'];
$date = date('Y-m-d H:i:s');

if(!isset($db[$chip_id])) {
    header($_SERVER['SERVER_PROTOCOL'].' 500 Error', true, 500);
    exit();
}

$files = '[';
for ($i = 1; $i <= 2; $i++) {
    $img = 'snapshots/' . $chip_id . '_' . $date . '_' . time() . '.jpg';
    $sleep_until = time() + 2; //2 -> 1 сек
    time_sleep_until($sleep_until);
    $files .= ",'". $img. "'";
}
$files .= ']';

$data_line = $chip_id .';'. $date .';'. $cnt .';'. $files;
file_put_contents('db.txt', $data_line . PHP_EOL, FILE_APPEND);

?>
