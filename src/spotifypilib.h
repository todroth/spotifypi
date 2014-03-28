#ifndef SPOTIFYPILIB_H
#define SPOTIFYPILIB_H

typedef struct search_result {
	const char *track_name;
	const char *artist_name;
} search_result;

enum play_mode {RESULTS, PLAYLIST};

int login(const char* username, const char* password);
int is_logged_in();
void search(char* query, int num_results, int offset);
int get_num_tracks();
search_result get_next_result();
void play_track(int result);
void pause_track();
void continue_track();
void wait_for_it();
int is_search_complete();
int get_current_playing_track_index();
int get_current_playing_playlist();
enum play_mode get_current_mode();
int is_playing();
void next_track();
void prev_track();
const char *get_did_u_mean();
void fill_playlist_file(char *file);
void playlist_play(int playlist, int track);
void set_random(int random);

#endif
