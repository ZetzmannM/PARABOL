# Parabol

This is planned to be a long term project with Vulkan written in C++. 
The goal is to create somewhat of a framework that can later be used comfortably to create smaller/simpler games.
It is planned to support Windows & Ubuntu.

It uses:

- GLM library
- GLFW 

It uses the _dep_inc_ folder for any includes necessary for the libraries used.  
It uses the _lib_ folder for any binarys necessary.

Refer to the README.md files in the directiories _dep_inc_ and _lib_ for more information

The _rsccpy.bat.in_ file is responsible for copying the resource directory to the executable in the binary path since the resource folder is assumed to always be in the same directory as the compiled executable
