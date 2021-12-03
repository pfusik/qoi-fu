CFLAGS = -O2 -Wall
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.0/ref/net6.0
TRANSPILED = $(addprefix transpiled/QOI., c cpp cs js py swift) transpiled/QOIDecoder.java

all: png2qoi $(TRANSPILED)

png2qoi: png2qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ png2qoi.c QOI-stdio.c transpiled/QOI.c -lpng

file-qoi: file-qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled `gimptool-2.0 --cflags` -o $@ file-qoi.c QOI-stdio.c transpiled/QOI.c `gimptool-2.0 --libs`

file-qoi.exe: file-qoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled `gimptool-2.0 --cflags` -o $@ file-qoi.c QOI-stdio.c transpiled/QOI.c -Wl,-subsystem,windows `gimptool-2.0 --libs`

Xqoi.usr: Xqoi.c QOI-stdio.c QOI-stdio.h transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ Xqoi.c QOI-stdio.c transpiled/QOI.c -shared

QOIPaintDotNet.dll: QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(PAINT_NET_DIR)/System.Drawing.Common.dll" -r:"$(DOTNET_REF_DIR)/System.Drawing.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

$(TRANSPILED): QOI.ci
	mkdir -p $(@D) && cito -o $@ $^

clean:
	$(RM) png2qoi file-qoi file-qoi.exe Xqoi.usr QOIPaintDotNet.dll $(TRANSPILED) transpiled/QOI.h transpiled/QOI.hpp transpiled/QOIColorspace.java transpiled/QOIEncoder.java

.PHONY: all clean
