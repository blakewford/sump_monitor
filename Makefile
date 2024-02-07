SRCS:=./src/main.cpp ./src/mqtt.cpp ./src/bmp.cpp
sump_monitor: $(SRCS)
	g++ -std=c++11 -gdwarf-4 -O2 $(SRCS) -o $@

android:
	-@mkdir build
	cd build && /Applications/CMake.app/Contents/bin/cmake ../ -B . -DCMAKE_TOOLCHAIN_FILE=~/Library/Android/sdk/ndk/26.1.10909125/build/cmake/android.toolchain.cmake -DANDROID_ABI=armeabi-v7a -DANDROID_PLATFORM=android-26
	cd build && make

clean:
	-@rm sump_monitor
	-@rm -rf sump_monitor.dSYM
	-@rm -rf build
