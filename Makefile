CXX=g++-mp-4.5
CXXFLAGS=-m32 -w -g -std=c++0x
	
LIB=Leap freetype json_spirit cef cef_dll_wrapper glfw3 GLEW opencv_core opencv_imgproc opencv_highgui opencv_video boost_date_time boost_thread boost_filesystem boost_system
LIB_PARAMS=$(foreach d,$(LIB),-l$d)

LIBDIR=/opt/local/lib/gcc45 /usr/lib/ /usr/local/lib /Users/adamskubel/lib/boost/boost_1_53_0/stage/lib /Users/adamskubel/lib/opencv-2.4.5/release/lib /Users/adamskubel/lib/LeapSDK/lib /Users/adamskubel/lib/cef_binary_3.1453.1255_macosx/Release
LIB_DIR_PARAMS=$(foreach l, $(LIBDIR),-L$l)

INC=/Users/adamskubel/lib/cef_binary_3.1453.1255_macosx /usr/local/include/freetype2 /usr/local/include /usr/include /Users/adamskubel/lib/boost/boost_1_53_0 /Users/adamskubel/lib/LeapSDK/include /Users/adamskubel/lib/opencv-2.4.5/include /Users/adamskubel/lib/glfw-3.0.1/include
INC_PARAMS=$(foreach d, $(INC),-I$d)

SOURCES= main.cpp AlbumCursorView.cpp AlbumPanel.cpp Animation.cpp Button.cpp Color.cpp ContentPanel.cpp CustomGrid.cpp DataViewGenerator.cpp DataListActivity.cpp FBNode.cpp FacebookBrowser.cpp FriendListCursorView.cpp FBDataCursorImpl.cpp FBFriendFQLCursor.cpp FacebookIntroView.cpp FakeDataSource.cpp FastSeekBar.cpp FixedAspectGrid.cpp FlyWheel.cpp FrameView.cpp FriendPanel.cpp GlobalConfig.cpp HandModel.cpp HandRotationGestureDetector.cpp ImageButton.cpp ImageDetailView.cpp ImageLoader.cpp ImagePanel.cpp LeapDebug.cpp LeapElement.cpp LeapStartScreen.cpp LinearLayout.cpp PackingGrid.cpp PicturePanel.cpp PanelBase.cpp PanelFactory.cpp LeapInput.cpp RadialMenu.cpp ResourceManager.cpp ScrollingView.cpp SeekVisual.cpp ShakeGestureDetector.cpp TextPanel.cpp TextureLoadTask.cpp TextureLoader.cpp TexturePanel.cpp TexturePool.cpp TypographyManager.cpp UniformGrid.cpp View.cpp ViewOrchestrator.cpp kiss_fft.cpp SwipeGestureDetector.cpp TextEditPanel.cpp ScrollBar.cpp Cefalopod.cpp InteractionsTutorial.cpp FriendCursorView.cpp Logger.cpp LeapCursor.cpp ColorPanel.cpp

OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=main

default: all

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -m32 $(LIB_DIR_PARAMS) $(LIB_PARAMS) -framework IOKit -framework OpenGL -framework Cocoa -o $@ $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INC_PARAMS) -c -o $@ $<
