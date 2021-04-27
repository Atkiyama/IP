XLIB = /usr/X11R6/lib
XINC = /usr/X11R6/include

imgdisp : sat_main.o sub.o xwin.o
	cc -O -o imgdisp sat_main.o sub.o xwin.o -I${XINC} -L${XLIB} -lX11 -lm

.c.o:
	cc -c  $<


