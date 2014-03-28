# spotifypi

spotifypi is a small program that I wrote for my [Raspberry Pi](http://www.raspberrypi.org/). With this program, you can attach your Raspberry Pi to your stereo and make it stream music via [Spotify](http://www.spotify.com). But the best part is: you can access and control the streaming with a little web application in your local network.
![Web Application Screenshot](https://github.com/todroth/spotifypi/blob/master/ressources/web_app_screenshot.png "Web Application Screenshot")

There is one big requirement: You need a Spotify Premium Account to get an API key and to use the API.

It is only a small project I did for myself, so the installation isn't very comfortable, but it kind of works like that:

1. Install an operating system on your Raspberry Pi (I used the latest [Raspbian (Wheezy)](http://www.raspbian.org))
2. Set an root password
`sudo passwd root`	
3. Going root
`su`
4. Install clang compiler on your Raspberry Pi
`apt-get install clang`
5. Install the spotify API
`cd /opt
wget https://developer.spotify.com/download/libspotify/libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
tar -xf libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release.tar.gz
mv libspotify-12.1.103-Linux-armv6-bcm2708hardfp-release libspotify
cd libspotify
mkdir 12.1.103
mv * 12.1.103
ln -s 12.1.103/ current
cd current
make install`
	
Check, if it was installed correctly:
`pkg-config --print-provides libspotify`
6. Install libasound package
`apt-get install libasound2-dev`
7. Enter your API key, your Spotify username and your password into src/keys_example.c and save it as keys.c.
8. Copy the source files on your Raspberry Pi
`scp -r ~/downloads/spotifypi root@IP_OF_YOUR_RASPBERRY:/opt`
8. Build the application
`cd /opt/spotifypi
make`	
9. Run it
`./spotifypi`
You should see:
*login
successfully logged in*

Cancel it with CTRL+C

10. Install apache2
`apt-get update
apt-get install apache2 php5 libapache2-mod-php5`
	
11. Change apache2 root directory	
In */etc/apache2/sites-enabled* change
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
	
12. Setup auto login on bootup
In */etc/inittab* change
_1:2345:respawn:/sbin/getty --noclear 38400 tty1_
to
_#1:2345:respawn:/sbin/getty --noclear 38400 tty1_
and add after this line
_1:2345:respawn:/bin/login -f root tty1 </dev/tty1 >/dev/tty1 2>&1_
	
13. Setup autostart spotifypi on bootup
`cd ~`
in *.bashrc* add the following two lines
_cd /opt/spotifypi
./spotifypi_

`reboot`
