Compile with:

gcc -o transcoder transcoder.c libspots.c

Usage:

./transcoder <input file> <fps>

Example:

./transcoder somefile.ogg 5

This will create a file called somefile.ogg.mkv with 5 fps. See the source for encoding details.
This will also create a file called somefile.ogg.ffmetadata containing chapter information about the spots stream.

Depends on ffmpeg being in your $PATH. See source for invocation.



How does it work?

The ogg file gets read and demuxed (we actually only demux the spots stream). The whole spots stream is kept in memory.
Then the spots stream is read from our in-memory copy. The position at which the image data beings is decoded and
then the end marker of it (this far JPEG is assumed) is searched by scanning one byte at a time. This information (the
location in the in-memory copy and the size) along with the timestamp encoded in the ogg stream is stored in a linked
list for each image. Next an ffmpeg instance is spawned with a pipe connection. With the linked list a continuous image
stream is generated with the correct image being sent to ffmpeg at the correct time (rounding errors may occur depending
on the framerate chosen). Additionally a metadata file containing chapter information for each image is generated and
attached to the final file, allowing users to skip through the file based on slides.
