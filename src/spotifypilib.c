#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <libspotify/api.h>
#include "audio.h"
#include "spotifypilib.h"

#define DEBUG 1

extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;

void debug(const char* format, ...);

static void on_login(sp_session *session, sp_error error);
static void on_main_thread_notified(sp_session *session);
static int on_music_delivered(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames);
static void on_log(sp_session *session, const char *data);
static void on_end_of_track(sp_session *session);
static void on_search_complete(sp_search *search, void *userdata);
static void playlistcontainer_loaded(sp_playlistcontainer *pc, void *userdata);
static void playlist_loaded(sp_playlist *pl, void *userdata);

static audio_fifo_t g_audiofifo;
static sp_search *last_search;
sp_session *current_session;
sp_playlistcontainer *playlistcontainer;
int current_result_index;
int current_playing_track_index = -1;
int search_complete;
int is_paused = 1;
char *playlist_file;
int current_playing_playlist = -1;
enum play_mode current_mode;
int mode_random = 0;

int logged_in;

static sp_session_callbacks session_callbacks = {
	.logged_in = &on_login,
	.notify_main_thread = &on_main_thread_notified,
	.music_delivery = &on_music_delivered,
	.log_message = &on_log,
	.end_of_track = &on_end_of_track
};

static sp_playlist_callbacks pl_callbacks = {
	.playlist_state_changed = &playlist_loaded
};

static sp_session_config spconfig = {
	.api_version = SPOTIFY_API_VERSION,
	.cache_location = "tmp",
	.settings_location = "tmp",
	.application_key = g_appkey,
	.application_key_size = 0, // must be defined during runtime
	.user_agent = "spotify-pi",
	.callbacks = &session_callbacks,
	NULL
};

/* Public Functions */

int login(const char* username, const char* password) {
	debug("login");

	sp_error error;
	logged_in = 0;
	
	spconfig.application_key_size = g_appkey_size;
	
	error = sp_session_create(&spconfig, &current_session);
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: unable to create spotify session: %s\n", sp_error_message(error));
		return 1;
	}
	
	error = sp_session_login(current_session, username, password, 0, NULL);
	
	int next_timeout = 0;
	while (!logged_in) {
		sp_session_process_events(current_session, &next_timeout);
	}
	
	audio_init(&g_audiofifo);
	
	srand(time(NULL));
		
	return 0;
}

int is_logged_in() {
	return logged_in;
}

int is_search_complete() {
	return search_complete;
}

void search(char* query, int num_results, int offset) {
	debug("search");
	search_complete = 0;
	last_search = NULL;
	current_result_index = 0;
	if (!logged_in) {
		fprintf(stderr, "Not logged in!\n");
		return;
	}
	sp_search_create(current_session, query, offset, num_results, 0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD, &on_search_complete, NULL);
	int next_timeout = 0;
	while (last_search == NULL) {
		sp_session_process_events(current_session, &next_timeout);
	}
}

int get_num_tracks() {
	if (!search_complete) {
		return -1;
	}
	return sp_search_num_tracks(last_search);
}

search_result get_next_result() {
	search_result result;
	while (!search_complete) {
		printf("search not complete\n");
	}

	if (current_result_index == get_num_tracks()) {
		result.track_name = "\0";
		result.artist_name = "\0";
	} else {
		sp_track *track = sp_search_track(last_search, current_result_index);
		result.track_name = sp_track_name(track);
		result.artist_name = sp_artist_name(sp_track_artist(track, 0));
		++current_result_index;
	}
	return result;
}

void play_track(int result) {
	if (last_search == NULL) {
		return;
	}
	if (result >= get_num_tracks() || result < 0) {
		return;
	}
	
	
	sp_error error = sp_session_player_load(current_session, sp_search_track(last_search, result));
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: %s\n", sp_error_message(error));
		return;
	}
	
	sp_session_player_play(current_session, 1);
	current_playing_track_index = result;
	current_mode = RESULTS;
	is_paused = 0;
}

void playlist_play(int playlist, int track) {

	if (playlistcontainer == NULL) {
		printf("playlist_play\n");
		return;
	}
	if (playlist >= sp_playlistcontainer_num_playlists(playlistcontainer) || playlist < 0) {
		return;
	}
	sp_playlist *pl = sp_playlistcontainer_playlist(playlistcontainer, playlist);
	if (track >= sp_playlist_num_tracks(pl) || track < 0) {
		return;
	}
	sp_error error = sp_session_player_load(current_session, sp_playlist_track(pl, track));
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: %s\n", sp_error_message(error));
		return;
	}
	sp_session_player_play(current_session, 1);
	current_playing_track_index = track;
	current_playing_playlist = playlist;
	current_mode = PLAYLIST;
	is_paused = 0;
}

void pause_track() {
	sp_session_player_play(current_session, 0);
	is_paused = 1;
}

void continue_track() {
	sp_session_player_play(current_session, 1);
	is_paused = 0;
}

void next_track() {
	switch (current_mode) {
		case RESULTS:
			if (mode_random == 1) {
				int random = rand() % get_num_tracks();
				play_track(random);
				printf("play random: %d\n", random);
			} else {
				play_track(current_playing_track_index + 1);
			}
			break;
		case PLAYLIST:
			if (mode_random == 1) {
				sp_playlist *pl = sp_playlistcontainer_playlist(playlistcontainer, current_playing_playlist);
				int random = rand() % sp_playlist_num_tracks(pl);
				playlist_play(current_playing_playlist, random);
				printf("play random: %d\n", random);
			} else {
				playlist_play(current_playing_playlist, current_playing_track_index + 1);
			}
			break;
	}
}

void prev_track() {
	switch (current_mode) {
		case RESULTS:
			play_track(get_current_playing_track_index() - 1);
			break;
			
		case PLAYLIST:
			playlist_play(current_playing_playlist, current_playing_track_index - 1);
			break;
	}
}

int is_playing() {
	return !is_paused;
}

int get_current_playing_track_index() {
	return current_playing_track_index;
}

int get_current_playing_playlist() {
	return current_playing_playlist;
}

enum play_mode get_current_mode() {
	return current_mode;
}

void wait_for_it() {
	int timeout = 0;
	sp_session_process_events(current_session, &timeout);
}

const char *get_did_u_mean() {
	if (last_search == NULL) {
		return "";
	}
	return sp_search_did_you_mean(last_search);
}

void set_random(int random) {
	mode_random = random;
}

/* Libspotify Functions */

void debug(const char* format, ...) {
	if (!DEBUG) {
		return;
	}
	va_list argptr;
	va_start(argptr, format);
	vprintf(format, argptr);
	printf("\n");
}

static void on_login(sp_session *session, sp_error error) {
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error: unable to log in: %s\n", sp_error_message(error));
		return;
	}
	logged_in = 1;
	debug("successfully logged in");
}

static void on_search_complete(sp_search *search, void *userdata) {
	debug("on_search_complete");
	sp_error error = sp_search_error(search);
	if (error != SP_ERROR_OK) {
		fprintf(stderr, "Error during search: %s\n", sp_error_message(error));
	}
	
	last_search = search;
	search_complete = 1;
}

static void on_main_thread_notified(sp_session *session) {

}

static int on_music_delivered(sp_session *session, const sp_audioformat *format, const void *frames, int num_frames) {
	audio_fifo_t *af = &g_audiofifo;
	audio_fifo_data_t *afd;
	size_t s;
	
	if(num_frames == 0) {
		return 0;
	}
	
	pthread_mutex_lock(&af->mutex);
	// Buffer one second of audio
	if (af->qlen > format->sample_rate) {
		pthread_mutex_unlock(&af->mutex);
		return 0;
	}
	
	s = num_frames * sizeof(int16_t) * format->channels;
	afd = malloc(sizeof(*afd) + s);

	memcpy(afd->samples, frames, s);

	afd->nsamples = num_frames;
	afd->rate = format->sample_rate;
	afd->channels = format->channels;

	TAILQ_INSERT_TAIL(&af->q, afd, link);

	af->qlen += num_frames;

	pthread_cond_signal(&af->cond);
	pthread_mutex_unlock(&af->mutex);
	
	return num_frames;
}

static void on_log(sp_session *session, const char *data) {

}

static void on_end_of_track(sp_session *session) {
	debug("on_end_of_track");
	audio_fifo_flush(&g_audiofifo);
	switch (current_mode) {
		case RESULTS:
			if (mode_random == 1) {
				int random = rand() % get_num_tracks();
				play_track(random);
				printf("play random: %d\n", random);
			} else {
				play_track(current_playing_track_index + 1);
				}
			break;
		case PLAYLIST:
			if (mode_random == 1) {
				sp_playlist *pl = sp_playlistcontainer_playlist(playlistcontainer, current_playing_playlist);
				int random = rand() % sp_playlist_num_tracks(pl);
				playlist_play(current_playing_playlist, random);
				printf("play random: %d\n", random);
			} else {
				playlist_play(current_playing_playlist, current_playing_track_index + 1);
			}
			break;
	}
}

void fill_playlist_file(char *file) {
	playlist_file = file;
	sp_playlistcontainer_callbacks playlistcontainer_callbacks = {
		.container_loaded = playlistcontainer_loaded
	};
	sp_playlistcontainer *container = sp_session_playlistcontainer(current_session);
	sp_playlistcontainer_add_callbacks(container, &playlistcontainer_callbacks, NULL);
	
}

static void playlistcontainer_loaded(sp_playlistcontainer *pc, void *userdata) {
	playlistcontainer = pc;
	for (int i = 0; i < sp_playlistcontainer_num_playlists(pc); ++i) {
		sp_playlist *pl = sp_playlistcontainer_playlist(pc, i);
		sp_playlist_add_callbacks(pl, &pl_callbacks, NULL);
	}
}

static void playlist_loaded(sp_playlist *playlist, void *userdata) {
	
	int num_playlists = sp_playlistcontainer_num_playlists(playlistcontainer);
	int i;
	
	for (i = 0; i < num_playlists; ++i) {
		if (!sp_playlist_is_loaded(sp_playlistcontainer_playlist(playlistcontainer, i))) {
			return;
		}
	}	
	
	FILE *file = fopen(playlist_file, "w");
	char *tmp_string;
	
	tmp_string = "[";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	for (i = 0; i < sp_playlistcontainer_num_playlists(playlistcontainer); ++i) {
		
		sp_playlist *pl = sp_playlistcontainer_playlist(playlistcontainer, i);
		
		tmp_string = "{\"name\":\"";
		fwrite(tmp_string, 1, strlen(tmp_string), file);
		
		const char *tmp_string_1 = sp_playlist_name(pl);
		fwrite(tmp_string_1, 1, strlen(tmp_string_1), file);
		
		tmp_string = "\",\"tracks\":[";
		fwrite(tmp_string, 1, strlen(tmp_string), file);
		
		for (int j = 0; j < sp_playlist_num_tracks(pl); ++j) {
			sp_track *track = sp_playlist_track(pl, j);
			sp_artist *artist = NULL;
			
			while (!sp_track_is_loaded(track)) {
				printf("not loaded\n");
				wait_for_it();
				sleep(1);
			}
			
			if (sp_track_num_artists(track) > 0) {
				artist = sp_track_artist(track, 0);
			}
			
			tmp_string = "{\"name\":\"";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
		
			const char *tmp_string_2 = sp_track_name(track);
			fwrite(tmp_string_2, 1, strlen(tmp_string_2), file);
			
			tmp_string = "\",\"artist\":\"";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
			if (artist != NULL) {
				const char *tmp_string_3 = sp_artist_name(artist);
				fwrite(tmp_string_3, 1, strlen(tmp_string_3), file);
			}
			
			
			tmp_string = "\"},";
			fwrite(tmp_string, 1, strlen(tmp_string), file);
			
		}
		
		tmp_string = "{}]},";
		fwrite(tmp_string, 1, strlen(tmp_string), file);
	}
	
	tmp_string = "{}]";
	fwrite(tmp_string, 1, strlen(tmp_string), file);
	
	fclose(file);
	
	printf("done\n");
}
