
# If MSBUILD_CYGWIN is set correctly then msbuild is used with cygwin.
#MSBUILD_CYGWIN="/cygdrive/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe"
# If MSBUILD_CMD is set correctly then msbuild is used with cmd.
#MSBUILD_CMD="C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\MSBuild\\Current\\Bin\\MSBuild.exe"

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
ifneq (,$(findstring /cygdrive/,$(PATH)))
ifdef MSBUILD_CYGWIN
	@echo ""
	@echo "MSBUILD_CYGWIN is set. Start compilation with msbuild."
	$(MSBUILD_CYGWIN) build/tml.sln
else
	@echo ""
	@echo "No MSBUILD_CYGWIN is set. Open the project with Visual Studio and compile it ;-)"
endif
else ifeq ($(OS),Windows_NT)
ifdef MSBUILD_CMD
	@echo ""
	@echo "MSBUILD_CMD is set. Start compilation with msbuild."
	$(MSBUILD_CMD) build/tml.sln
else
	@echo ""
	@echo "No MSBUILD_CMD is set. Open the project with Visual Studio and compile it ;-)"
endif
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


