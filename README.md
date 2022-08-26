About
-----

wxWidgets-wasm is a WebAssembly port of wxWidgets.

wxWidgets is a free and open source cross-platform C++ framework
for writing advanced GUI applications using native controls.

![wxWidgets Logo](https://www.wxwidgets.org/assets/img/header-logo.png)

wxWidgets allows you to write native-looking GUI applications for
all the major desktop platforms and also helps with abstracting
the differences in the non-GUI aspects between them. It is free
for the use in both open source and commercial applications, comes
with the full, easy to read and modify, source and extensive
documentation and a collection of more than a hundred examples.
You can learn more about wxWidgets at https://www.wxwidgets.org/
and read its documentation online at https://docs.wxwidgets.org/


Licence
-------

[wxWidgets licence](https://github.com/wxWidgets/wxWidgets/blob/master/docs/licence.txt)
is a modified version of LGPL explicitly allowing not distributing the sources
of an application using the library even in the case of static linking.

WASM sources are released under the LGPL v2 licence.


Building
--------

git submodule update --init src/jpeg

git submodule update --init 3rdparty/catch

export CFLAGS=-I$EMSCRIPTEN/system/local/include

export CXXFLAGS=-I$EMSCRIPTEN/system/local/include

export LDFLAGS=-L$EMSCRIPTEN/system/local/lib -sERROR_ON_UNDEFINED_SYMBOLS=0

export CONFIGURE_ARGS="
  --host=emscripten \
  --with-cxx=14 \
  --enable-utf8 \
  --enable-universal \
  --disable-shared \
  --disable-exceptions \
  --disable-richtext \
  --without-libtiff \
  --disable-xlocale"

$EMSCRIPTEN/emconfigure $SOURCE_DIR/configure $CONFIGURE_ARGS

$EMSCRIPTEN/emmake make


Example Apps
------------

- [Life Demo](https://life.dj.app/)
- [Transitions DJ](https://dj.app/)
- [Wavvy Audio Editor](https://wavvy.app/)


Further information
-------------------

If you are looking for community support, you can get it from

- [Mailing Lists](https://www.wxwidgets.org/support/mailing-lists/)
- [Discussion Forums](https://forums.wxwidgets.org/)
- [#wxwidgets IRC channel](https://www.wxwidgets.org/support/irc/)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/wxwidgets)
  (tag your questions with `wxwidgets`)
- And you can report bugs at https://trac.wxwidgets.org/newticket

[Commercial support](https://www.wxwidgets.org/support/commercial/) is also
available.

Finally, keep in mind that wxWidgets is an open source project collaboratively
developed by its users and your contributions to it are always welcome. Please
check [our guidelines](.github/CONTRIBUTING.md) if you'd like to do it.


Have fun!

The wxWidgets Team.
