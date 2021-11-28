CFLAGS = -O2 -Wall
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.0/ref/net6.0
TRANSPILED = $(addprefix transpiled/QOI., c cpp cs java js py swift)

all: png2qoi $(TRANSPILED)

png2qoi: png2qoi.c transpiled/QOI.c
	$(CC) $(CFLAGS) -I transpiled -o $@ $^ -lpng

QOIPaintDotNet.dll: QOIPaintDotNet.cs transpiled/QOI.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(PAINT_NET_DIR)/System.Drawing.Common.dll" -r:"$(DOTNET_REF_DIR)/System.Drawing.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

$(TRANSPILED): QOI.ci
	mkdir -p $(@D) && cito -o $@ $^

clean:
	$(RM) png2qoi QOIPaintDotNet.dll $(TRANSPILED)

.PHONY: all clean
