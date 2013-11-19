<?php

	// Load JSON state
    $string = file_get_contents("robot_state.json");
    $json_a= json_decode($string,true);

    $json_a['speed_left'] = $_GET["speedLeft"];
    $json_a['speed_right'] = $_GET["speedRight"];
    $json_a['direction_left'] = $_GET["directionLeft"];
    $json_a['direction_right'] = $_GET["directionRight"];

    $fp = fopen('robot_state.json', 'w');
    fwrite($fp, json_encode($json_a));
    fclose($fp);

?>