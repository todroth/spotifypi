# ![spotifypi logo](https://github.com/todroth/spotifypi/blob/master/ressources/logo.png?raw=true "spotifypi logo") spotifypi

__Small personal project! You can do what you want, I'll guarantee nothing ;)__

spotifypi is a small program that I wrote for my [Raspberry Pi](http://www.raspberrypi.org/). With this program, you can attach your Raspberry Pi to your stereo and make it stream music via [Spotify](http://www.spotify.com). But the best part is: you can access and control the streaming with a little web application in your local network.
![Web Application Screenshot](https://github.com/todroth/spotifypi/blob/master/ressources/web_app_screenshot.png?raw=true "Web Application Screenshot")

## Requirements

There is one big requirement: You need a Spotify Premium Account to get an API key and to use the API.

## How it works

The Spotify API is writen in C, so about 80% of this project is written in C.
When you start the program, it will login on Spotify using the in *src/keys.c* specified data. Then, it will read the file *tmp/communication* once a second, and will trigger an action if there is some command in this file. So you can control it simply by writing commands in this file (`echo "command param" > tmp/communication`).
It will recognize the following commands:
  * `continue` Continues playing when it is paused
  * `next` Plays the next track of the current list
  * `play 1` Plays the first track of the last search query
  * `playlist_play 1 2` Plays the second track of the first playlist
  * `pause` Pauses
  * `previous` Plays the previous track of the current list
  * `random on` and `random off` Turns random-mode on or off
  * `search query` will search Spotify for the query
  
 The outputs of will be written in various files; the results of a search will be written in *tmp/results*, and so on.
 
 On the installed apache2 server is PHP installed, so these file-operations (writing to *tmp/communication* and reading from the other files in *tmp/*) are done in PHP, and will be triggered with ajax-requests, which will be triggered by user inputs in the web application.
 
 ...That's about it.

## Installation

It is only a small project I did for myself, so the installation isn't very comfortable, but it kind of works like that:

* Install an operating system on your Raspberry Pi (I used the latest [Raspbian (Wheezy)](http://www.raspbian.org))
* Set an root password
`sudo passwd root`
* Going root
`su`
* Install clang compiler on your Raspberry Pi
`apt-get install clang`
* Install the spotify API
```
cd /opt
wget https://developer.spotify.com/download/libspotify/libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
tar -xf libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
mv libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release libspotify
cd libspotify
mkdir 12.1.103
mv * 12.1.103
ln -s 12.1.103/ current
cd current
make install
```
	
Check, if it was installed correctly:
`pkg-config --print-provides libspotify`
* Install libasound package
`apt-get install libasound2-dev`
* Enter your API key, your Spotify username and your password into src/keys_example.c and save it as keys.c.
* Copy the source files on your Raspberry Pi
`scp -r ~/downloads/spotifypi root@IP_OF_YOUR_RASPBERRY:/opt`
* Build the application
```
cd /opt/spotifypi
make
```	
* Run it
`./spotifypi`
You should see:
__login
successfully logged in__

Cancel it with CTRL+C

* Install apache2
```
apt-get update
apt-get install apache2 php5 libapache2-mod-php5
```
	
* Change apache2 root directory	
In __/etc/apache2/sites-enabled__ change
_DocumentRoot /var/www_
to
_DocumentRoot /opt/spotifypi_

In /etc/apache2/apache2.conf change
_User ${APACHE_RUN_USER}
GROUP ${APACHE_RUN_GROUP}_
to
_User www-data
GROUP www-data_
	
`reboot`
	
* Setup auto login on bootup
In __/etc/inittab__ change
`1:2345:respawn:/sbin/getty --noclear 38400 tty1`
to
`#1:2345:respawn:/sbin/getty --noclear 38400 tty1`
and add after this line
`1:2345:respawn:/bin/login -f root tty1 </dev/tty1 >/dev/tty1 2>&1`
	
* Setup autostart spotifypi on bootup
`cd ~`
in __.bashrc__ add the following two lines
```
cd /opt/spotifypi
./spotifypi
```
