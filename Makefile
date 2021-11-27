CFLAGS = -O2 -Wall
TRANSPILED_LANG = c cpp cs java js py swift
TRANSPILED_ENCODER = $(TRANSPILED_LANG:%=transpiled/QOIEncoder.%)
TRANSPILED_DECODER = $(TRANSPILED_LANG:%=transpiled/QOIDecoder.%)

all: png2qoi $(TRANSPILED_ENCODER) $(TRANSPILED_DECODER)

png2qoi: png2qoi.c transpiled/QOIEncoder.c transpiled/QOIDecoder.c
	$(CC) $(CFLAGS) -I transpiled -o $@ $^ -lpng

$(TRANSPILED_ENCODER): QOIEncoder.ci
	mkdir -p $(@D) && cito -o $@ $^

$(TRANSPILED_DECODER): QOIDecoder.ci
	mkdir -p $(@D) && cito -o $@ $^

clean:
	$(RM) png2qoi $(TRANSPILED_ENCODER) $(TRANSPILED_DECODER)

.PHONY: all clean
