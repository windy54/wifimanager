This is the initial commit of "Andy's Fish tank"
based on randomnerdturorials wifi manager project.

oN first run it looks for a configured wifi manager, this does not exist so creates a wifi hotspot ESP-Wifi_mam=nager
connect to this on 148.196.4.1

enter details of SSID and password that you want to use.

ESP reboots and connects to wifi.
In a browser connect to the esp, e.g. 192.168.4.177

a simple page opens with options for brightness of white and blue leds, white on and blue on hours and entry for current time hours and minutes.

This version uses two leds rather than neopixels.
Future changes:
1) use neopixels
2) set hostname
3) get time from ntp server, this does not seem to work if system remains connected to wifi, using random nerd tutorials

4) 
