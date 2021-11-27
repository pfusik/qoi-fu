CFLAGS = -O2 -Wall
CSC = "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/Roslyn/csc.exe" -nologo
PAINT_NET_DIR = C:/Program Files/paint.net
DOTNET_REF_DIR = C:/Program Files/dotnet/packs/Microsoft.NETCore.App.Ref/6.0.0/ref/net6.0
TRANSPILED_LANG = c cpp cs java js py swift
TRANSPILED_ENCODER = $(TRANSPILED_LANG:%=transpiled/QOIEncoder.%)
TRANSPILED_DECODER = $(TRANSPILED_LANG:%=transpiled/QOIDecoder.%)

all: png2qoi $(TRANSPILED_ENCODER) $(TRANSPILED_DECODER)

png2qoi: png2qoi.c transpiled/QOIEncoder.c transpiled/QOIDecoder.c
	$(CC) $(CFLAGS) -I transpiled -o $@ $^ -lpng

QOIPaintDotNet.dll: QOIPaintDotNet.cs transpiled/QOIDecoder.cs
	$(CSC) -o+ -out:$@ -t:library $^ -nostdlib -r:"$(PAINT_NET_DIR)/PaintDotNet.Data.dll" -r:"$(PAINT_NET_DIR)/System.Drawing.Common.dll" -r:"$(DOTNET_REF_DIR)/System.Drawing.dll" -r:"$(DOTNET_REF_DIR)/System.Runtime.dll"

$(TRANSPILED_ENCODER): QOIEncoder.ci
	mkdir -p $(@D) && cito -o $@ $^

$(TRANSPILED_DECODER): QOIDecoder.ci
	mkdir -p $(@D) && cito -o $@ $^

clean:
	$(RM) png2qoi QOIPaintDotNet.dll $(TRANSPILED_ENCODER) $(TRANSPILED_DECODER)

.PHONY: all clean
