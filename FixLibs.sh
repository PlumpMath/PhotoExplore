change_lib () {
install_name_tool -change $1 @executable_path/../Frameworks/$2 ../PhotoExplorer.app/Contents/MacOS/PhotoExplorer
}

change_exec_libs(){

change_lib /usr/local/lib/libfreetype.6.dylib libfreetype.6.dylib
change_lib libboost_date_time.dylib libboost_date_time.dylib
change_lib libboost_filesystem.dylib libboost_filesystem.dylib
change_lib libboost_system.dylib libboost_system.dylib
change_lib libboost_thread.dylib libboost_thread.dylib
change_lib /usr/local/lib/libfreetype.6.dylib libfreetype.6.dylib

change_lib lib/libopencv_core.2.4.dylib libopencv_core.2.4.dylib 
change_lib lib/libopencv_imgproc.2.4.dylib libopencv_imgproc.2.4.dylib
change_lib lib/libopencv_highgui.2.4.dylib libopencv_highgui.2.4.dylib
change_lib lib/libopencv_video.2.4.dylib libopencv_video.2.4.dylib

change_lib /usr/lib/libGLEW.1.9.0.dylib libGLEW.1.9.0.dylib 
 
change_lib /opt/local/lib/gcc45/libgcc_s.1.dylib libgcc_s.1.dylib 
change_lib /usr/lib/libgcc_s.1.dylib libgcc_s.1.dylib 
change_lib /opt/local/lib/libstdc++.6.dylib libstdc++.6.dylib 

}

change_lib_2 () {
install_name_tool -change $1 @executable_path/../Frameworks/$2 ../PhotoExplorer.app/Contents/Frameworks/$3

}

change_dylibs() {

change_lib_2 lib/libopencv_core.2.4.dylib libopencv_core.2.4.dylib libopencv_highgui.2.4.dylib
change_lib_2 lib/libopencv_imgproc.2.4.dylib libopencv_imgproc.2.4.dylib libopencv_highgui.2.4.dylib

change_lib_2 lib/libopencv_core.2.4.dylib libopencv_core.2.4.dylib libopencv_imgproc.2.4.dylib

change_lib_2 lib/libopencv_core.2.4.dylib libopencv_core.2.4.dylib libopencv_video.2.4.dylib
change_lib_2 lib/libopencv_imgproc.2.4.dylib libopencv_imgproc.2.4.dylib  libopencv_video.2.4.dylib
}

change_exec_libs
change_dylibs