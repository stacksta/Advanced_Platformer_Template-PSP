TARGET = PLATFORMER
OPENTRI = include/openTRI/streams/streams.o  include/openTRI/triVAlloc.o include/openTRI/triMemory.o include/openTRI/triRefcount.o include/openTRI/triImage.o include/openTRI/rle.o include/openTRI/triGraphics.o include/openTRI/triLog.o include/openTRI/triInput.o ./include/openTRI/triWav.o include/openTRI/triAudioLib.o include/openTRI/triTimer.o include/openTRI/triVMath_vfpu.o include/openTRI/triFont.o
OBJS = $(OPENTRI) main.o callback.o 

INCDIR += ./include/ ./include/openTRI/ ./include/freetype/
CFLAGS = -G0 -Wall -O3 -DUSE_VFPU=true -g -D__PSP__ -D_DEBUG -D_DEBUG_LOG -D_DEBUG_MEMORY -DTRI_DDLIST -DTRI_SUPPORT_PNG -DTRI_SUPPORT_FT 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fpermissive
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lstdc++ -lpspgum -lfreetype -lpng -ljpeg -lg -lc -lGL -lGLU -lglut -lz -lm \
        -lpspvfpu -lpspvram -lpspaudio -lpspaudiocodec \
        -L${PSPDEV}/psp/sdk/lib \
        -L${PSPDEV}/psp \
        -lpspdebug   -lpspgu -lpspctrl -lpspge -lpspirkeyb -lpsppower -lpsppower_driver \
        -lpspdisplay -lpsphprm  -lpspsdk -lpsprtc -lpspaudio -lpsputility \
        -lpspnet_inet  -lc -lpspuser

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PLATFORMER

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
