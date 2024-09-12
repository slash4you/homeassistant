g++ backlight_service.cpp -lX11 -lXext -lpigpiod_if2 -o backlight_service
g++ backlight_dimming.cpp -lpigpiod_if2 -lpthread -o backlight_dimming
g++ backlight_screensaver.cpp -lpigpiod_if2 -lpthread -o backlight_screensaver


