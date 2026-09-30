# ministreamer

ministreamer is a wannabe clone of OBS based on GStreamer and GTK4. It is meant
as a practice project around both technologies.

## Features 

- Capture screen/windows/screen portions with pipewire through libportal
- Overlay several sources on top of another
- Dynamically add/remove sources

## How to compile 

Run `meson setup build` to generate the ninja files.

Run `ninja -C build` to generate the binary.

