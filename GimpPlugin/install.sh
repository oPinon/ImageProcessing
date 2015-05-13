# On any fresh Ubuntu installation, just do :
# sudo apt-get install gimp
# sudo apt-get install libgimp2.0-dev
# launch gimp at least once to create .gimp-2.8/
# then compile SuperSample.c with :

FLAGS="-Ofast -lm" # our custom flags for compilation
COMMAND=$(gimptool-2.0 --install "src/SuperSample.c src/GimpPlugin.c" --just-print) # gimptool does the linking with GCC
$COMMAND $FLAGS # compiling

# gimptool should output something like :
# gcc  -pthread -I/usr/include/gtk-2.0 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/cairo -I/usr/include/libpng12 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/harfbuzz -I/usr/include/gimp-2.0   -o /home/ubuntu/.gimp-2.8/plug-ins/SuperSample SuperSample.c  -lgimpui-2.0 -lgimpwidgets-2.0 -lgimpmodule-2.0 -lgimp-2.0 -lgimpmath-2.0 -lgimpconfig-2.0 -lgimpcolor-2.0 -lgimpbase-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 -lfontconfig -lgobject-2.0 -lglib-2.0 -lfreetype $FLAGS
