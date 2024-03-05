ifeq ($(OS),Windows_NT)
	MAKE := mingw32-make
	MAKE_TARGET := mf_os/Makefile-win32-mingw
else
	MAKE := make
	MAKE_TARGET := mf_os/Makefile-unix
endif

.PHONY: all clean

all:
	$(MAKE) -f $(MAKE_TARGET)

clean:
	$(MAKE) -f $(MAKE_TARGET) clean
