var last_result = "";

$(function() {
	$('#alert_didumean').hide();
	write_current_result();
	write_playlists();
	highlight_current_row();
	set_correct_pause_button();
	start_loops();
});

function write_current_result() {
	console.log("write current result");
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data: {
			command : "current_result"
		},
		dataType : "json",
		success : function(data) {
			var result = JSON.parse(data);
			if (result != false) {
				if (data != last_result) {
					var collapse = last_result != "";
					last_result = data;
					$("#results > tbody").empty();
					if (result.success == "true") {
						write_result(result.data);
					}
					if (result.didumean != "") {
						$('#alert_didumean').show();
						write_didumean(result.didumean);
					} else {
						$('#alert_didumean').hide();
					}
					if (collapse) {
						$('#results-collapse').collapse('show');
					}
				}
			}
			
		}
	});
}

function write_playlists() {
	console.log("write playlists");
	$.ajax({
		type : 'GET',
		url : 'web/functions.php',
		data : {
			command : 'playlists'
		},
		dataType : 'json',
		success : function (data) {
			var playlists = JSON.parse(data);
			var div = $('#playlists');
			div.empty();
			var string = "";
			for (var i = 0; i < playlists.length - 1; ++i) {
				if (playlists[i].tracks.length > 1) {
					string += '<div class="panel panel-default panel-notopmargin">';
					string += '<a data-toggle="collapse" data-parent="accordion" href="#toggle' + i + '">';
					string += '<div class="panel-heading">';
					string += '<h4 class="panel-title">';
					string += '<div class="panel-symbol" id="panel-symbol-playlist' + i + '"></div>';
					string += playlists[i].name;
					string += '</h4>';
					string += '</div>';
					string += '</a>';
					string += '<div id="toggle' + i + '" class="panel-collapse collapse">';
					string += '<div class="panel-body">';
						string += '<table class="table table-striped table-hover table-condensed panel-body">';
							string += '<thead>';
							string += '<tr><th></th><th>track</th><th>artist</th></tr>';
							string += '</thead>';
							string += '<tbody>';
							for (var j = 0; j < playlists[i].tracks.length - 1; ++j) {
								string += '<tr><td><div></div></td><td data-row="' + j + '" data-playlist="' + i + '" class="track">';
								string += playlists[i].tracks[j].name;
								string += '</td><td class="artist">';
								string += playlists[i].tracks[j].artist;
								string += '</td>';
								string += '</tr>';
							}
							string += '</tbody>';
						string += '</table>';
					string += '</div>';
					string += '</div>';
					string += '</div>';
				}
			}
			div.append(string);
		}
	});
}

function highlight_current_row() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data: {
			command : "current_index"
		},
		dataType : "json",
		success : function(data) {
			
			var current_track = "";
			var current_artist = "";
	
			$("td.track").each(function(td) {
				$(this).parent().children().first().children().removeClass("glyphicon glyphicon-volume-up");
			});
			
			$('.panel-symbol').each(function () {
				$(this).removeClass('glyphicon glyphicon-volume-up');
			});
			
			switch (data.mode) {
				case "results":
				
				$("td.track").each(function(td) {
					if ($(this).data("row") == data.track_index && typeof $(this).data("playlist") == 'undefined') {
						$(this).parent().children().first().children().addClass("glyphicon glyphicon-volume-up");
						current_track = $(this)[0].innerHTML;
						current_artist = $(this)[0].parentElement.children[2].innerHTML;
					}
				});
				
				$('#panel-symbol-results').addClass('glyphicon glyphicon-volume-up');
				
				break;
				
				case "playlist":
				
				$("td.track").each(function(td) {

					if ($(this).data("row") == data.track_index && $(this).data("playlist") == data.playlist_index) {
						$(this).parent().children().first().children().addClass("glyphicon glyphicon-volume-up");
						current_track = $(this)[0].innerHTML;
						current_artist = $(this)[0].parentElement.children[2].innerHTML;
					}
				});
				
				$('#panel-symbol-playlist' + data.playlist_index).addClass("glyphicon glyphicon-volume-up");
				
				break;
			}

			if (current_artist != "" && current_track != "") {
				$("#currently_playing").html(current_artist + " - " + current_track);
			}
		}
	});
	
}

function set_correct_pause_button() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "is_playing"
		},
		dataType : "text",
		success : function(data) {
			if (data == "true") {
				set_pause_button("pause");
			} else {
				set_pause_button("cont");
			}
		}
	});
}

function set_correct_random_button() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "get_random"
		},
		dataType : "json",
		success : function (data) {
			if (data == "true") {
				$("#random_button").addClass("active");
				console.log("active");
			} else if (data == "false") {
				$("#random_button").removeClass("active");
			}
		}
	});
}

function start_loops() {
	setInterval(write_current_result, 2000);	
	setInterval(highlight_current_row, 2000);
	setInterval(set_correct_pause_button, 2000);
	setInterval(set_correct_random_button, 2000);
}

function write_result(data) {
	if(typeof data == 'undefined') {
		return;
	}
	
	for (var i = 0; i < data.length; ++i) {
		if (typeof data[i].artist != 'undefined' && typeof data[i].track != 'undefined') {
			$("#results > tbody:last").append("\
				<tr>\
					<td><div></div></td>\
					<td data-row=\"" + i + "\" class=\"track\">" + data[i].track + "</td>\
					<td class=\"artist\">" + data[i].artist + "</td>\
				</tr>");
		}
	}
}

function write_didumean(didumean) {
	$('#didumean').text(didumean);
}

$("#search_button").click(function() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data: {
			command : "search",
			data : $("#search_text").val()
		},
		dataType : "json",
		success : function(data) {
			write_current_result();
		}
	});	
});

$('#alert_didumean').click(function() {
	$('#search_text').val($('#didumean').html());
	$('#search_button').click();
});

$("#search_text").keyup(function(event) {
	if(event.keyCode == 13) {
		$("#search_button").click();
	}
});

$("#pause_button").click(function() {
	var command;
	var button = $("#pause_button");
	switch(button.attr("data-action")) {
		case "pause":
			command = "pause";
			set_pause_button("cont");
		break;
		
		case "cont":
			command = "continue";
			set_pause_button("pause");
		break;
	}
	$.ajax({
		type : "GET",
		url : "web/functions.php", 
		data : {
			command : command
		}
	});
});

$("#next_button").click(function() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "next"
		}
	});
});

$('#prev_button').click(function() {
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "previous"
		}
	});
});

$("#results").on('click', 'td.track', function() {
	var row = $(this).data("row");
	if (typeof row == 'undefined') {
		return;
	}
	
	set_pause_button("pause");
	
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "play",
			data : row
		}
	});

});

$("#playlists").on('click', 'td.track', function () {
	var playlist = $(this).data("playlist");
	var row = $(this).data("row");
	if (typeof row == 'undefined') {
		return;
	}
	
	$.ajax({
		type : "GET",
		url : "web/functions.php",
		data : {
			command : "playlist_play",
			data : playlist,
			data2 : row
		}
	});
});

$("#accordion").on('click', 'td.artist', function () {
	$('#search_text').val($(this).text());
	$('#search_button').click();
	$('html, body').animate({
		scrollTop : $("#accordion").offset().top
	}, 1000);
});

$("#random_button").on('click', function () {
	if ($(this).hasClass("active")) {
		$.ajax({
			type : "GET",
			url : "web/functions.php",
			data : {
				command : "set_random",
				data : "off"
			}
		});
	} else {
		$.ajax({
			type : "GET",
			url : "web/functions.php",
			data : {
				command : "set_random",
				data : "on"
			}
		});
	}
});

function set_pause_button(state) {
	var button = $("#pause_button");
	var icon = $("#pause_icon");
	
	switch (state) {
	
		case "pause":
		button.attr("data-action", "pause");
		icon.removeClass("glyphicon-play");
		icon.addClass("glyphicon-pause");
		break;
		
		case "cont":
		button.attr("data-action", "cont");
		icon.removeClass("glyphicon-pause");
		icon.addClass("glyphicon-play");
		break;
	}
}
