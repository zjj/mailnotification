mail-daemon:mail-daemon.c
	cc mail-daemon.c -o mail-daemon -pthread `pkg-config libnotify gtk+-2.0 --cflags --libs` 
