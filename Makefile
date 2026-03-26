# ============================================================
#  Makefile  –  Multi-Format Compression & Decompression Tool
# ============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -O2
LDFLAGS = -lm

TARGET  = compressor

SRCS    = main.c \
          types.c \
          huffman.c \
          text_compress.c \
          bmp_compress.c \
          color_image.c \
          dct_compress.c

OBJS    = $(SRCS:.c=.o)

# ---- Default target ----------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ---- Pattern rule: compile each .c to .o ------------------
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ---- Explicit header dependencies -------------------------
main.o:          main.c          types.h text_compress.h bmp_compress.h dct_compress.h
types.o:         types.c         types.h
huffman.o:       huffman.c       huffman.h types.h
text_compress.o: text_compress.c text_compress.h huffman.h types.h
bmp_compress.o:  bmp_compress.c  bmp_compress.h types.h
color_image.o:   color_image.c   color_image.h types.h
dct_compress.o:  dct_compress.c  dct_compress.h color_image.h types.h

# ---- Utility targets ---------------------------------------
clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
