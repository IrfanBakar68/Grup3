#Hüseyin Göbekli (G211210041) 2B
#Okan Başol (G211210083) 2B
#Şimal Ece Kazdal (G221210068) 2B
#Muhammed İrfan Bakar (G221210596) 2B
#Betül Kurt (G221210054) - 2C 
# Derleme komutları
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# Çıktı adı
TARGET=shell

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

