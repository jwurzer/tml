
all: build compile

build:
	-mkdir build
ifneq (,$(findstring /cygdrive/,$(PATH)))
	echo "Using cygwin"
	echo "visual studio 2022 (version number 17.0)"
	cd build && cmake -G "Visual Studio 17" ..
else ifeq ($(OS),Windows_NT)
	echo visual studio 2022 (version number 17.0)
	cd build && cmake -G "Visual Studio 17" ..
else
	cd build; cmake ..
	#cd build; cmake -DCMAKE_BUILD_TYPE=Debug ..
endif

compile:
ifeq ($(OS),Windows_NT)
	@echo ""
	@echo "Open the project with Visual Studio and compile it ;-)"
else
	make -C build
endif

clean:
ifneq (,$(findstring /cygdrive/,$(PATH)))
	rm -rf build
else ifeq ($(OS),Windows_NT)
	-rmdir build /s /q
else
	rm -rf build
endif

