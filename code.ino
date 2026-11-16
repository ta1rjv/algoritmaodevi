/*
===========================================================================
                 DEVRE BAĞLANTI ŞEMASI (NE NEREYE TAKILACAK?)
===========================================================================
1. HC-SR04 ULTRASONİK SENSÖR:
   - VCC bacağı  ---> Arduino üzerindeki 5V pinine
   - GND bacağı  ---> Breadboard (-) eksi hattına
   - Trig bacağı ---> Arduino Dijital 9 numaralı pine
   - Echo bacağı ---> Arduino Dijital 10 numaralı pine

2. LED (HEDEF UYARI IŞIĞI):
   - Uzun bacak (+ Anot)  ---> 220Ω veya 330Ω direncin bir ucuna,
                               direncin diğer ucu Arduino Dijital 6 numaralı pine
   - Kısa bacak (- Katot) ---> Breadboard (-) eksi hattına

3. AKTİF BUZZER (SESLİ BİLDİRİM):
   - Uzun bacak (+) ---> Arduino Dijital 7 numaralı pine (üzerinde + işareti yazar)
   - Kısa bacak (-) ---> Breadboard (-) eksi hattına

4. ORTAK TOPRAK (GND) BAĞLANTISI:
   - Sensör, LED ve Buzzer'ın birleştiği Breadboard (-) eksi hattından 
     tek bir jumper kablo ile Arduino'nun üzerindeki GND pinine bağlantı yapın.
===========================================================================
*/

// --- PİN TANIMLAMALARI ---
const int trigPin = 9; // Ultrasonik sensörün ses dalgası göndereceği pini (9) sabit tam sayı olarak tanımlar
const int echoPin = 10; // Ultrasonik sensörden dönen yankıyı okuyacak pini (10) sabit tam sayı olarak tanımlar
const int ledPin = 6; // Hedef uyarısı verecek LED'in bağlı olduğu pini (6) sabit tam sayı olarak tanımlar
const int buzzerPin = 7; // Sesli bildirim verecek buzzer'ın bağlı olduğu pini (7) sabit tam sayı olarak tanımlar

// --- SİSTEM AYARLARI ---
const int ALGI_MESAFESI = 15; // Kutunun algılanması için gereken maksimum mesafeyi (15 cm) sabit olarak belirler
const int URETIM_HEDEFI = 5; // Uyarı verilmesi için ulaşılması gereken toplam ürün sayısını (5 adet) belirler
const unsigned long BEKLEME_SURESI = 10000; // Ürün geçmediğinde bekleme uyarısı verilecek süreyi milisaniye cinsinden (10000 ms = 10 sn) tanımlar

// --- DURUM DEĞİŞKENLERİ ---
int urunSayisi = 0; // Geçen toplam ürün adedini hafızada tutan sayacı oluşturur ve sıfırdan başlatır
bool kutuOnundeMi = false; // Sensörün önünde şu an kutu olup olmadığını tutan mantıksal (doğru/yanlış) bayrak değişkenidir
bool hedefUlasildi = false; // Üretim hedefine ulaşıldığında LED/buzzer uyarısının sadece bir kez verilmesini sağlayan bayraktır
bool beklemeMesajiVerildi = false; // Bekleme mesajının ekrana sürekli yazılmasını engelleyip tek sefer yazılmasını sağlayan bayraktır
unsigned long sonUrunZamani = 0; // Son ürünün sensör önünden geçtiği anın zaman bilgisini (milisaniye) saklayan değişkendir

void setup() { // Arduino'ya güç verildiğinde veya resetlendiğinde yalnızca bir kez çalışan kurulum fonksiyonunu başlatır
  pinMode(trigPin, OUTPUT); // Trig pinini dışarıya elektrik sinyali gönderecek şekilde ÇIKIŞ (OUTPUT) olarak ayarlar
  pinMode(echoPin, INPUT); // Echo pinini sensörden gelen sinyali okuyacak şekilde GİRİŞ (INPUT) olarak ayarlar
  pinMode(ledPin, OUTPUT); // LED pinini ışık yakmak için ÇIKIŞ (OUTPUT) olarak ayarlar
  pinMode(buzzerPin, OUTPUT); // Buzzer pinini ses çıkarmak için ÇIKIŞ (OUTPUT) olarak ayarlar

  digitalWrite(ledPin, LOW); // Sistem ilk açıldığında LED'in kapalı (0V) konumda olmasını sağlar
  digitalWrite(buzzerPin, LOW); // Sistem ilk açıldığında buzzer'ın sessiz (0V) konumda olmasını sağlar

  Serial.begin(9600); // Bilgisayar ile Arduino arasında 9600 baud hızında seri haberleşmeyi başlatır
  Serial.println("=== KONVEYOR URUN SAYIM VE TAKIP SISTEMI BASLATILDI ==="); // Seri port ekranına başlangıç bilgisini yazar ve alt satıra geçer
  Serial.print("Hedef Uretim: "); // Seri port ekranına yan yana yazmak üzere etiket metnini gönderir
  Serial.println(URETIM_HEDEFI); // Belirlenen üretim hedefi sayısını ekrana yazar ve alt satıra geçer

  sonUrunZamani = millis(); // Sistemin başladığı anki süreyi milisaniye cinsinden alıp başlangıç zamanı olarak kaydeder
} // setup() kurulum fonksiyonunu sonlandırır

void loop() { // setup() tamamlandıktan sonra Arduino açık kaldığı sürece sürekli tekrar eden ana döngüyü başlatır
  long mesafe = mesafeOlc(); // Kendi yazdığımız mesafeOlc() fonksiyonunu çağırarak ölçtüğü mesafeyi 'mesafe' değişkenine kaydeder

  bool suAnKutuVar = (mesafe > 0 && mesafe <= ALGI_MESAFESI); // Mesafe 0'dan büyük ve 15 cm'den küçük/eşitse sonucu 'true' (kutu var), değilse 'false' yapar

  // 1. DURUM DEĞİŞİMİ: Sensör önü boştu, şimdi kutu girdi
  if (suAnKutuVar && !kutuOnundeMi) { // Şu an kutu varsa VE bir önceki ölçümde kutu yoksa (yani kutu sensör önüne yeni girdiyse) bloğa girer
    kutuOnundeMi = true; // Kutu sensör önünden ayrılana kadar tekrar sayım yapılmasını kilitlemek için bayrağı 'true' yapar
    urunSayisi++; // Toplam ürün sayacını 1 artırır (urunSayisi = urunSayisi + 1 ile aynıdır)
    sonUrunZamani = millis(); // Yeni ürün geçtiği için bekleme süresi sayacını o anki zamanla günceller (sıfırlar)
    beklemeMesajiVerildi = false; // Üretim yeniden başladığı için bir sonraki duraksamada uyarı verebilmek adına bekleme bayrağını sıfırlar

    Serial.print("Urun Algilandi! Toplam Sayi: "); // Seri port ekranına ürün algılandığı bilgisini yazdırır
    Serial.print(urunSayisi); // Güncel sayılan toplam ürün miktarını aynı satıra yazdırır
    Serial.print(" / "); // Sayı ile hedef arasına bölü işareti koyar
    Serial.println(URETIM_HEDEFI); // Ulaşılması gereken hedef sayıyı yazdırıp bir alt satıra geçer

    // 2. ÜRETİM HEDEFİ KONTROLÜ
    if (urunSayisi >= URETIM_HEDEFI && !hedefUlasildi) { // Ürün sayısı hedefe ulaştıysa VE hedef uyarısı daha önce verilmediyse bloğa girer
      hedefUlasildi = true; // Hedef uyarısının sonraki ürünlerde tekrar çalmasını engellemek için bayrağı 'true' yapar
      digitalWrite(ledPin, HIGH); // Hedefe ulaşıldığını göstermek için LED'e 5V göndererek sürekli yanmasını sağlar

      Serial.println(">>> URETIM HEDEFINE ULASILDI! <<<"); // Seri port ekranına hedefin tamamlandığını bildiren uyarı mesajını yazar

      digitalWrite(buzzerPin, HIGH); // Buzzer'a 5V göndererek sesli uyarıyı başlatır
      delay(400); // Buzzer'ın 400 milisaniye (0.4 saniye) boyunca kısa bir bip sesi çıkarmasını bekler
      digitalWrite(buzzerPin, LOW); // Buzzer'ın elektriğini keserek sesi susturur
    } // Hedef kontrolü if bloğunu sonlandırır
  } // Yeni kutu algılama if bloğunu sonlandırır
  // 3. DURUM DEĞİŞİMİ: Kutu sensör önünden ayrıldı
  else if (!suAnKutuVar && kutuOnundeMi) { // Şu an sensör önünde kutu yoksa VE az önce kutu varsa (kutu geçip gittiyse) bloğa girer
    kutuOnundeMi = false; // Sensör önünün boşaldığını kaydeder; böylece sıradaki yeni kutunun sayılmasına izin verir
  } // Kutu ayrılma kontrolü else if bloğunu sonlandırır

  // 4. BEKLEME DURUMU KONTROLÜ
  if ((millis() - sonUrunZamani >= BEKLEME_SURESI) && !beklemeMesajiVerildi) { // Şu anki zamandan son ürün zamanı çıkarıldığında 10 sn geçtiyse VE mesaj henüz verilmediyse girer
    Serial.println("[BILGI] Uzun suredir urun gecmedi -> URETIMDE BEKLEME"); // Seri port ekranına hattın beklemeye geçtiğini bildiren mesajı yazar
    beklemeMesajiVerildi = true; // Mesajın ekranı sürekli doldurmaması için bu bekleme sürecinde mesaj verildi olarak işaretler
  } // Bekleme kontrolü if bloğunu sonlandırır

  delay(50); // Ultrasonik sensörün ses dalgalarının birbirine karışmaması için döngüyü 50 milisaniye bekletir
} // loop() ana döngüsünü sonlandırır ve başa döner

// HC-SR04 Mesafe Ölçüm Fonksiyonu
long mesafeOlc() { // Mesafeyi santimetre cinsinden hesaplayıp 'long' (uzun tam sayı) olarak geri döndüren özel fonksiyonu başlatır
  digitalWrite(trigPin, LOW); // Temiz bir sinyal göndermeden önce Trig pinini kesin olarak 0V (kapalı) yapar
  delayMicroseconds(2); // Sinyalin kararlı hale gelmesi için 2 mikrosaniye bekler
  digitalWrite(trigPin, HIGH); // Sensörün ultrasonik ses dalgası yayması için Trig pinine 5V gönderir
  delayMicroseconds(10); // Sensörün tetiklenmesi için gerekli olan 10 mikrosaniye boyunca sinyali yüksek tutar
  digitalWrite(trigPin, LOW); // Ses dalgası gönderimini tamamlamak için Trig pinini tekrar 0V yapar

  long sure = pulseIn(echoPin, HIGH, 30000); // Echo pininin HIGH (5V) kalma süresini mikrosaniye cinsinden ölçer; nesne yoksa 30000 µs (30 ms) sonra beklemeyi bırakır
  long hesaplananMesafe = sure * 0.034 / 2; // Ses hızını (0.034 cm/µs) gidiş-geliş olduğu için 2'ye bölerek süreyi santimetre cinsinden mesafeye çevirir
  return hesaplananMesafe; // Hesaplanan santimetre değerini fonksiyonun çağrıldığı loop() satırına geri gönderir
} // mesafeOlc() fonksiyonunu sonlandırır
