<?php
	error_reporting(E_ALL);


	if (isset($_GET["command"])) {
		$command = $_GET["command"];
		if (isset($_GET["data"])) {
			$data = $_GET["data"];
		}
		if (isset($_GET["data2"])) {
			$data2 = $_GET["data2"];
		}
		switch ($command) {
			case "search":
				search($data);
			break;
			case "play":
				play_track($data);
			break;
			case "playlist_play":
				playlist_play($data, $data2);
			break;
			case "pause":
				pause_track();
			break;
			case "continue":
				continue_track();
			break;
			case "current_result":
				current_result();
			break;
			case "current_index":
				current_index();
			break;
			case "is_playing":
				is_playing();
			break;
			case "next":
				next_track();
			break;
			case "previous":
				prev_track();
			break;
			case "playlists":
				get_playlists();
			break;
			case "set_random":
				set_random($data);
			break;
			case "get_random":
				get_random();
			break;
		}
	}

	function search($query) {

		write_to_communication("search ".$query);

		do { // Wait for results
			$file = fopen("../tmp/last_query", "r");
			if (!$file) {
				return;
			}
			$line = fgets($file);
			fclose($file);
			time_nanosleep(0, 100000000);
		} while($line != $query);
		
		echo get_result_json();
		
	}
	
	
	function play_track($row) {
		write_to_communication("play ".$row);
	}
	
	function playlist_play($playlist, $track) {
		write_to_communication("playlist_play ".$playlist." ".$track);
	}
	
	function pause_track() {
		write_to_communication("pause");
	}
	
	function continue_track() {
		write_to_communication("continue");
	}
	
	function current_result() {
		echo get_result_json();
	}
	
	function current_index() {
		$file = fopen("../tmp/current_playing_track_index", "r");
		if (!$file) {
			return;
		}
		
		$content = fgets($file);
		fclose($file);
		echo $content; 
	}
	
	function is_playing() {
		$file = fopen("../tmp/is_playing", "r");
		if (!$file) {
			return;
		}
		$content = fgets($file);
		fclose($file);
		
		if ($content == "false") {
			echo "false";
		} else if ($content == "true"){
			echo "true";
		}
	}
	
	function next_track() {
		write_to_communication("next");
	}
	
	function prev_track() {
		write_to_communication("previous");
	}
	
	function get_result_json() {
		$file = fopen("../tmp/results", "r");
		if (!$file) {
			return;
		}		
		
		$content = fgets($file);
		fclose($file);
		
		return json_encode($content);
	}
	
	function get_playlists() {
		$file = fopen("../tmp/playlists", "r");
		if (!$file) {
			return;
		}
		$content = fgets($file);
		fclose($file);
		echo json_encode($content);
	}
	
	function set_random ($state) {
		if ($state == "on") {
			write_to_communication("random on");
		} else {
			write_to_communication("random off");
		}
	}
	
	function write_to_communication($str) {
		$file = fopen("../tmp/communication", "w");
		fwrite($file, $str);
		fclose($file);
	}
	
	function get_random() {
		$file = fopen("../tmp/random", "r");
		if (!$file) {
			return;
		}
		$content = fgets($file);
		fclose($file);
		echo json_encode($content);
	}
	
?>
