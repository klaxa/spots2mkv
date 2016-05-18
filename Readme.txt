Compile with:

gcc -o transcoder transcoder.c libspots.c

Usage:

./transcoder <input file> <fps>

Example:

./transcoder somefile.ogg 5

This will create a file called somefile.ogg.mkv with 5 fps. See the source for encoding details.

Depends on ffmpeg being in your $PATH. See source for invocation.
