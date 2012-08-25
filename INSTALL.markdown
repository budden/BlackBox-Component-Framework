# Install #


## Build instructions ##

This distribution of BlackBox uses .cp extension as default for storage of source code files in UTF-8 encoding instead of .odc .
Hence you should install CpcUtf8Conv from [www.zinnamturm.eu](http://www.zinnamturm.eu/downloadsAC.htm#CpcUtf8Conv) . Otherwise you cannot build from sources.

Open document at Dev/Docu/BuildTool and run build command. When everything compiled, file BlackBox2.exe should be created. Name BlackBox2.exe is used intentionally so it will not conflict with existing BlackBox.exe.

## Run-time dependencies ##
* Cairo graphics library (if version 1.6.4 used): libcairo-2.dll, freetype6.dll, libexpat-1.dll, libfontconfig-1.dll, libpng12-0.dll, zlib1.dll
 can be downloaded from http://www.gtk.org/download/win32.php
