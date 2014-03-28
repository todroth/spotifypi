#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "spotifypilib.h"

#define INPUT_BUFFER_SIZE 512
#define PATH_TO_COMMUNICATION "tmp/communication"
#define PATH_TO_RESULTS "tmp/results"
#define PATH_TO_LAST_QUERY "tmp/last_query"
#define PATH_TO_CURRENT_PLAYING_TRACK_INDEX "tmp/current_playing_track_index"
#define PATH_TO_IS_PLAYING "tmp/is_playing"
#define PATH_TO_PLAYLISTS "tmp/playlists"
#define PATH_TO_RANDOM "tmp/random"

enum command {PLAY, PAUSE, CONTINUE, SEARCH, PREVIOUS, NEXT, PLAYLIST_PLAY, RANDOM};
enum result_type {TRACK_ARTIST};

int done = 0;

extern const char *username;
extern const char *password;

void get_input();
void parse_input(char *input);
void execute_command(enum command key, char* value);
void clear_file(char *path);
void escape(const char *sequence, char* buf);

void handle_search(char *query);
void handle_play(char *track);
void handle_playlist_play(char *val);
void handle_pause();
void handle_continue();
void handle_next();
void handle_previous(); 
void handle_random(char *value);

void write_error();
void write_result(enum result_type result_type);
void write_last_query(char *query);
void write_current_playing_track_index();
void write_is_playing();

int main() {
	login(username, password);
	clear_file(PATH_TO_COMMUNICATION);
	clear_file(PATH_TO_RESULTS);
	clear_file(PATH_TO_LAST_QUERY);
	clear_file(PATH_TO_CURRENT_PLAYING_TRACK_INDEX);
	clear_file(PATH_TO_IS_PLAYING);
	clear_file(PATH_TO_PLAYLISTS);
	clear_file(PATH_TO_RANDOM);
	
	FILE *random = fopen(PATH_TO_RANDOM, "w");
	fwrite("false", 1, strlen("false"), random);
	fclose(random);
	
	fill_playlist_file(PATH_TO_PLAYLISTS);
	
	while (!done) {
		get_input();
		write_current_playing_track_index();
		write_is_playing();
		sleep(1);
		wait_for_it();
	}
	
	return 0;
}

void get_input() {
	int i = 0, c;
	FILE *file;
	struct stat stat;
	char buffer[INPUT_BUFFER_SIZE];
	
	file = fopen(PATH_TO_COMMUNICATION, "r");
	if (file == NULL) {
		perror("Error opening PATH_TO_COMMUNICATION");
		return;
	}
	
	fstat(fileno(file), &stat);
	if (stat.st_size > 1) {
		while((c = fgetc(file)) != EOF && i < INPUT_BUFFER_SIZE) {
			buffer[i] = c;
			++i;
		}
		buffer[i] = '\0';
		parse_input(buffer);
	}
	
	fclose(file);
	clear_file(PATH_TO_COMMUNICATION);
}

void parse_input(char *input) {
	int i, offset = -1;
	enum command key;
	char value[1024];
	
	printf("parse_input: %s\n", input);
		
	switch (input[0]) {
	
		case 'c':
			key = CONTINUE;
			offset = 9; // CONTINUE_
			break;
		
		case 'n':
			key = NEXT;
			offset = 5; // NEXT_
			break;
	
		case 'p':
			switch (input[1]) {
				case 'l':
					switch (input[4]) {
						case ' ':
							key = PLAY;
							offset = 5; // PLAY_
							break;
						case 'l':
							key = PLAYLIST_PLAY;
							offset = 14; // PLAYLIST_PLAY_
							break;
					}
					
					break;
				
				case 'a':
					key = PAUSE;
					offset = 6; // PAUSE_
					break;
				
				case 'r':
					key = PREVIOUS;
					offset = 9; // PREVIOUS_
					break;
			}
			
			break;
			
		case 'r':
			key = RANDOM;
			offset = 7;// RANDOM_
			break;
		
		case 's':
			key = SEARCH;
			offset = 7;// SEARCH_
			break;
	}
	
	if (offset != -1) {
		for (i = offset; i < strlen(input); ++i) {
			value[i - offset] = input[i];
		}
		value [i - offset] = '\0';
		
		execute_command(key, value);
	}
}

void execute_command(enum command key, char *value) {
	printf("command: %d\n", key);
	printf("value: %s\n", value);
	printf("======\n");
	
	switch (key) {
		case PLAY:
			handle_play(value);
		break;
		
		case PLAYLIST_PLAY:
			handle_playlist_play(value);
		break;
		
		case PAUSE:
			handle_pause();
		break;
		
		case CONTINUE:
			handle_continue();
		break;
		
		case SEARCH:
			handle_search(value);
		break;
		
		case NEXT:
			handle_next();		
		break;

		case PREVIOUS:
			handle_previous();
		break;
		
		case RANDOM:
			handle_random(value);
		break;
	}
}

void handle_random(char *value) {
	printf("handle random\n");
	if (value[1] == 'n') {
		set_random(1);
		FILE *random = fopen(PATH_TO_RANDOM, "w");
		fwrite("true", 1, strlen("true"), random);
		fclose(random);
	} else {
		set_random(0);
		FILE *random = fopen(PATH_TO_RANDOM, "w");
		fwrite("false", 1, strlen("false"), random);
		fclose(random);
	}
}

void handle_search(char *query) {
		
	printf("handle search\n");
	search(query, 100, 0);
	
	while (!is_search_complete()) {
		wait_for_it();
	}
	
	if (get_num_tracks() == 0) {
		write_error();
		write_last_query(query);
		return;
	}
	
	write_result(TRACK_ARTIST);
	write_last_query(query);
	
}

void handle_play(char *track) {
	int val = strtol(track, NULL, 10);
	if (val == 0 && errno == ERANGE) {
		return;
	}
	play_track(val);
	write_current_playing_track_index();
}

void handle_playlist_play(char *val) {

	char playlist[16];
	char track[16];
	
	int i = 0;
	while (val[i] != ' ') {
		playlist[i] = val[i];
		++i;
	}
	playlist[i] = '\0';
	
	int j = i + 1;
	int k = 0;
	for (i = j; i < strlen(val); ++i) {
		track[k] = val[i];
		++k;
	}
	track[k] = '\0';
	
	int playlist_index = strtol(playlist, NULL, 10);
	int track_index = strtol(track, NULL, 10);
	
	playlist_play(playlist_index, track_index);

}

void handle_pause() {
	pause_track();
}

void handle_continue() {
	continue_track();
}

void handle_next() {
	next_track();
}

void handle_previous() {
	prev_track();
}

void clear_file(char *path) {
	fclose(fopen(path, "w"));
}

void escape(const char *sequence, char* buf) {
	int i;
	for (i = 0; i < strlen(sequence); ++i) {
		switch(sequence[i]) {
			case '"':
				buf[i] = '\'';
			break;
			
			default:
				buf[i] = sequence[i];
			break;
		}
	}
}

void write_error() {
	char *tmp_string;
	FILE *file;

	clear_file(PATH_TO_RESULTS);	
	
	file = fopen(PATH_TO_RESULTS, "a+");
	
	tmp_string = "{\"success\":\"false\",\"didumean\":\"";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	const char* tmp_string_1 = get_did_u_mean();
	fwrite(tmp_string_1, 1, strlen(tmp_string_1), file);
	
	tmp_string = "\",\"data\":\"No tracks found!\"}";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	fclose(file);
}

void write_result(enum result_type result_type) {
	FILE *file;
	char *tmp_string;
	search_result result;
	
	clear_file(PATH_TO_RESULTS);
	
	file = fopen(PATH_TO_RESULTS, "a+");
	
	tmp_string = "{\"success\":\"true\",\"didumean\":\"";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	const char *tmp_string_1 = get_did_u_mean();
	fwrite(tmp_string_1, 1, strlen(tmp_string_1), file);
	
	tmp_string = "\",\"data\":[";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	switch(result_type) {
		case TRACK_ARTIST:
		
			result = get_next_result();
			
			while (result.track_name[0] != '\0' && result.artist_name[0] != '\0') {
			
				char artist[strlen(result.artist_name) + 1];
				char track[strlen(result.track_name) + 1];
				
				tmp_string = "{\"artist\":\"";
				fwrite(tmp_string, 1, strlen(tmp_string), file);
				
				escape(result.artist_name, artist);
				fwrite(artist, 1, strlen(result.artist_name), file);
				
				tmp_string = "\",\"track\":\"";
				fwrite(tmp_string, 1, strlen(tmp_string), file);
				
				escape(result.track_name, track);
				fwrite(track, 1, strlen(result.track_name), file);
				
				tmp_string = "\"},";
				fwrite(tmp_string, 1, strlen(tmp_string), file);
				
				result = get_next_result();
			}
			break;
	}
	
	tmp_string = "{}]}";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
		
	fclose(file);
}

void write_last_query(char *query) {

	FILE *file;
	
	clear_file(PATH_TO_LAST_QUERY);
	file = fopen(PATH_TO_LAST_QUERY, "w");
	
	if (file == NULL) {
		perror("Error opening PATH_TO_LAST_QUERY");
		return;
	}
	
	fwrite(query, 1, strlen(query), file);
	
	fclose(file);
}

void write_current_playing_track_index() {

	FILE *file;
	int index = get_current_playing_track_index();
	int playlist = get_current_playing_playlist();
	char current_index[5];
	char current_playlist[5];
	
	snprintf(current_index, 5, "%d", index);
	snprintf(current_playlist, 5, "%d", playlist);
	
	clear_file(PATH_TO_CURRENT_PLAYING_TRACK_INDEX);
	file = fopen(PATH_TO_CURRENT_PLAYING_TRACK_INDEX, "w");
	if (file == NULL) {
		perror("Error opening PATH_TO_CURRENT_PLAYING_TRACK_INDEX");
		return;
	}
	
	char *tmp_string = "{\"mode\":";
	fwrite(tmp_string, 1, strlen(tmp_string), file);

	switch (get_current_mode()) {
		case RESULTS:
		
			tmp_string = "\"results\",\"track_index\":\"";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
			fwrite(current_index, 1, strlen(current_index), file);
			
			tmp_string = "\"}";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
			break;
		case PLAYLIST:
		
			tmp_string = "\"playlist\",\"track_index\":\"";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
			fwrite(current_index, 1, strlen(current_index), file);
			
			tmp_string = "\",\"playlist_index\":\"";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
			fwrite(current_playlist, 1, strlen(current_playlist), file);
			
			tmp_string = "\"}";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
		
			break;
	}
	
	fclose(file);
}

void write_is_playing() {
	FILE *file;
	
	clear_file(PATH_TO_IS_PLAYING);
	file = fopen(PATH_TO_IS_PLAYING, "w");
	if (file == NULL) {
		perror("Error opening PATH_TO_IS_PLAYING");
		return;
	}
	
	char* to_write;
	if (is_playing()) {
		to_write = "true";
	} else {
		to_write = "false";
	}
	
	fwrite(to_write, 1, strlen(to_write), file);
	
	fclose(file);
}
