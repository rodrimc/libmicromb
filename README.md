#Micro Multimedia Backend Library

This is a simple multimedia backend. This library has been designed to provide a higher level API for 
handling multimedia objects. 
The library is built on top of the [GStreamer >= 1.6] (http://gstreamer.freedesktop.org/) framework. 

GStreamer is a powerful and flexible framework, maybe that's why it lacks a simple and clear API to 
handle media components. The main purpose of this project is to provide a simple API (start, stop, pause, set...)
to facilitate one to add multimedia capabilities to applications. Under the roof, the library
makes the appropriate calls to the GStreamer API.

##API
The mb.h file contains the definition of the external API that should be used. 

Some examples of how to use the library are available in the src/tests folder. 
One should notice that some of the examples contains hardcoded file paths. Before trying to run
the examples it's necessary to change those paths.

##How to compile
./bootstrap

./configure

./make

./make install

##Dependencies
This library depends on the [GStreamer 1.x] (http://gstreamer.freedesktop.org/) and [GLib] (https://developer.gnome.org/glib/stable/).

This is an ongoing project. Feel free to contribute.
