## Hüseyin Göbekli (G211210041) 2B
## Okan Başol (G211210083) 2B
## Şimal Ece Kazdal (G221210068) 2B
## Muhammed İrfan Bakar (G221210596) 2B
## Betül Kurt (G221210054) - 2C 

Bu proje, bir Linux shell simülatörüdür. Komutları çalıştırabilir, giriş/çıkış yönlendirme, boru (pipe) kullanımı ve arka plan işlemleri gibi temel özellikleri destekler.

## Özellikler
- **Tekli Komut Çalıştırma**: `ls`, `pwd` gibi komutlar çalıştırılabilir.
- **Giriş/Çıkış Yönlendirmesi**: `komut < dosya` ve `komut > dosya` desteklenir.
- **Boru (Pipe)**: `komut1 | komut2` şeklinde komutlar bağlanabilir.
- **Arka Plan İşlemleri**: `komut &` ile komutlar arka planda çalıştırılabilir.
- **Komut Geçmişi**: `history` ile geçmiş komutlar listelenir, `!<num>` ile geçmişten komutlar çalıştırılır.
- **Yerleşik Komutlar**: `cd`, `exit`, `history` desteklenir.

## Kullanım
1. Projeyi derleyin:
   ```bash
   make
