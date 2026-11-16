# Konveyör Üzerinde Ürün Sayımı ve Üretim Hedefi Takibi

**Sektör:** Ambalajlama ve Üretim Hatları  
**Geliştirici:** Selim Can Sayılır  
**Algoritmik Odak:** Durum değişimiyle (State Change) sayım ve aynı olayın tekrar işlenmesinin önlenmesi

---

## Proje Hakkında

Bu proje, endüstriyel ambalajlama ve üretim hatlarındaki konveyör bantlarından geçen ürünleri ultrasonik sensör yardımıyla gerçek zamanlı olarak sayan, üretim hedefini takip eden ve hat duraksamalarını tespit eden mikrodenetleyici tabanlı bir otomasyon prototipidir.

### Temel Özellikler
* **Mesafe Tabanlı Algılama:** Sensöre belirlenen mesafeden (`15 cm` ve daha yakın) yaklaşan kutuları ürün olarak algılar.
* **Tekrar Sayım Koruması:** Kutu sensörün önünde beklese dahi sayacı yalnızca bir kez artırır.
* **Sıfırlama Şartı:** Yeni bir ürünün sayılabilmesi için önce mevcut kutunun sensör önünden ayrılmasını bekler.
* **Hedef Bildirimi:** Belirlenen üretim hedefine ulaşıldığında durum LED'ini sürekli yakar ve buzzer ile kısa sesli bildirim verir.
* **Üretimde Bekleme Tespiti:** Hattan belirli bir süre (`10 saniye`) boyunca ürün geçmezse Seri Port ekranında bekleme uyarısı gösterir.

---

## Gerekli Malzemeler

| Bileşen | Miktar | Açıklama |
| :--- | :---: | :--- |
| **Arduino Uno** | 1 Adet | Mikrodenetleyici geliştirme kartı + USB kablosu |
| **HC-SR04** | 1 Adet | Ultrasonik mesafe sensörü |
| **LED (5mm)** | 1 Adet | Hedef tamamlandı gösterge ışığı |
| **Direnç (220Ω - 330Ω)** | 1 Adet | LED akım sınırlama direnci |
| **5V Aktif Buzzer** | 1 Adet | Hedef uyarısı için sesli bildirim elemanı |
| **Breadboard & Jumper Kablo** | 1 Set | Devre kurulumu ve bağlantılar için |

---

## Devre Bağlantı Şeması

| Bileşen | Bileşen Bacağı | Arduino Uno Pini | Not |
| :--- | :--- | :--- | :--- |
| **HC-SR04 Sensör** | `VCC` | `5V` | Besleme gerilimi |
| **HC-SR04 Sensör** | `GND` | `GND` | Ortak toprak hattı |
| **HC-SR04 Sensör** | `Trig` | `Dijital 9` | Ses dalgası tetikleme pini |
| **HC-SR04 Sensör** | `Echo` | `Dijital 10` | Yankı okuma pini |
| **LED** | `+ (Uzun Bacak)` | `Dijital 6` | `220Ω / 330Ω` direnç üzerinden bağlanır |
| **LED** | `- (Kısa Bacak)` | `GND` | Ortak toprak hattı |
| **Aktif Buzzer** | `+ (Uzun Bacak)` | `Dijital 7` | Kısa sesli bildirim çıkışı |
| **Aktif Buzzer** | `- (Kısa Bacak)` | `GND` | Ortak toprak hattı |

---

## Algoritmik Çalışma Mantığı

1. **Durum Değişimi Kontrolü (`kutuOnundeMi` Bayrağı):**
   Sistem yalnızca o an kutu olup olmadığına bakmaz; bir önceki döngüdeki durumu da hafızasında tutar. Sayım işlemi yalnızca sensör önü **boşken dolu hale geçtiği anda** (yükselen kenar) gerçekleşir. Kutu sensör önünden çekilip bayrak sıfırlanmadan yeni sayım yapılmaz.
2. **Tek Seferlik Olay İşleme (`hedefUlasildi` ve `beklemeMesajiVerildi` Bayrakları):**
   Hedef uyarısının sonraki her üründe tekrar çalmasını ve bekleme mesajının ekranı sürekli doldurmasını önlemek için olay bayrakları kullanılır. Her olay yalnızca durum gerçekleştiğinde **1 kez** tetiklenir.
3. **Bloklamayan Zamanlayıcı (`millis()`):**
   Bekleme süresi ölçümünde sistemi donduran `delay()` yerine arka planda çalışan `millis()` fonksiyonu kullanılmıştır. Bu sayede bekleme süresi sayılırken konveyörden ani bir ürün geçerse sistem anında algılama yapabilir.
4. **Sıfır Okuma Filtresi (`mesafe > 0`):**
   HC-SR04 sensörünün karşısında cisim olmadığında zaman aşımına uğrayıp `0 cm` döndürmesi durumunda oluşabilecek hatalı sayımlar `mesafe > 0` koşulu ile filtrelenmiştir.

---

## Arduino Kaynak Kodu

```cpp
// --- PİN TANIMLAMALARI ---
const int trigPin = 9; // Ultrasonik sensörün ses dalgası göndereceği pini (9) tanımlar
const int echoPin = 10; // Ultrasonik sensörden dönen yankıyı okuyacak pini (10) tanımlar
const int ledPin = 6; // Hedef uyarısı verecek LED'in bağlı olduğu pini (6) tanımlar
const int buzzerPin = 7; // Sesli bildirim verecek buzzer'ın bağlı olduğu pini (7) tanımlar

// --- SİSTEM AYARLARI ---
const int ALGI_MESAFESI = 15; // Kutunun algılanması için maksimum mesafe (15 cm)
const int URETIM_HEDEFI = 5; // Uyarı verilmesi için gereken toplam ürün sayısı (5 adet)
const unsigned long BEKLEME_SURESI = 10000; // Ürün geçmediğinde bekleme uyarısı süresi (10000 ms = 10 sn)

// --- DURUM DEĞİŞKENLERİ ---
int urunSayisi = 0; // Geçen toplam ürün adedini tutan sayaç
bool kutuOnundeMi = false; // Sensör önünde kutu olup olmadığını tutan durum bayrağı
bool hedefUlasildi = false; // Hedef uyarısının sadece bir kez verilmesini sağlayan bayrak
bool beklemeMesajiVerildi = false; // Bekleme mesajının tek sefer yazılmasını sağlayan bayrak
unsigned long sonUrunZamani = 0; // Son ürünün sensör önünden geçtiği zaman (ms)

void setup() {
  pinMode(trigPin, OUTPUT); // Trig pinini çıkış olarak ayarlar
  pinMode(echoPin, INPUT); // Echo pinini giriş olarak ayarlar
  pinMode(ledPin, OUTPUT); // LED pinini çıkış olarak ayarlar
  pinMode(buzzerPin, OUTPUT); // Buzzer pinini çıkış olarak ayarlar

  digitalWrite(ledPin, LOW); // Başlangıçta LED kapalı
  digitalWrite(buzzerPin, LOW); // Başlangıçta buzzer sessiz

  Serial.begin(9600); // 9600 baud hızında seri haberleşmeyi başlatır
  Serial.println("=== KONVEYOR URUN SAYIM VE TAKIP SISTEMI BASLATILDI ===");
  Serial.print("Hedef Uretim: ");
  Serial.println(URETIM_HEDEFI);

  sonUrunZamani = millis(); // Başlangıç zamanını kaydeder
}

void loop() {
  long mesafe = mesafeOlc(); // Sensörden santimetre cinsinden mesafe bilgisini alır
  bool suAnKutuVar = (mesafe > 0 && mesafe <= ALGI_MESAFESI); // Geçerli algılama aralığı kontrolü

  // 1. DURUM DEĞİŞİMİ: Sensör önü boştu, şimdi kutu girdi
  if (suAnKutuVar && !kutuOnundeMi) {
    kutuOnundeMi = true; // Tekrar sayımı kilitle
    urunSayisi++; // Ürün sayacını 1 artır
    sonUrunZamani = millis(); // Bekleme zamanlayıcısını sıfırla
    beklemeMesajiVerildi = false; // Yeni ürün geldiği için bekleme bayrağını sıfırla

    Serial.print("Urun Algilandi! Toplam Sayi: ");
    Serial.print(urunSayisi);
    Serial.print(" / ");
    Serial.println(URETIM_HEDEFI);

    // 2. ÜRETİM HEDEFİ KONTROLÜ
    if (urunSayisi >= URETIM_HEDEFI && !hedefUlasildi) {
      hedefUlasildi = true; // Hedef uyarısını kilitle
      digitalWrite(ledPin, HIGH); // Hedef LED'ini sürekli yak

      Serial.println(">>> URETIM HEDEFINE ULASILDI! <<<");

      digitalWrite(buzzerPin, HIGH); // Kısa sesli bildirim başlat
      delay(400); // 400 ms ses ver
      digitalWrite(buzzerPin, LOW); // Sesi durdur
    }
  }
  // 3. DURUM DEĞİŞİMİ: Kutu sensör önünden ayrıldı
  else if (!suAnKutuVar && kutuOnundeMi) {
    kutuOnundeMi = false; // Sensör önü boşaldı, yeni kutu sayılabilir
  }

  // 4. BEKLEME DURUMU KONTROLÜ
  if ((millis() - sonUrunZamani >= BEKLEME_SURESI) && !beklemeMesajiVerildi) {
    Serial.println("[BILGI] Uzun suredir urun gecmedi -> URETIMDE BEKLEME");
    beklemeMesajiVerildi = true; // Mesajın tekrarlanmasını engelle
  }

  delay(50); // Ultrasonik yankıların karışmaması için kısa kararlılık gecikmesi
}

// HC-SR04 Mesafe Ölçüm Fonksiyonu
long mesafeOlc() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long sure = pulseIn(echoPin, HIGH, 30000); // 30 ms zaman aşımı ile yankı süresini ölçer
  long hesaplananMesafe = sure * 0.034 / 2; // Süreyi santimetre (cm) cinsinden mesafeye çevirir
  return hesaplananMesafe;
}
```

---

## Kurulum ve Çalıştırma

1. Devre bağlantı şemasına göre bileşenleri breadboard ve Arduino Uno üzerine yerleştirin.
2. Arduino Uno kartını USB kablosu ile bilgisayara bağlayın.
3. **Arduino IDE** programını açın, `Araçlar > Kart` menüsünden **Arduino Uno**'yu ve ilgili **COM Port**'u seçin.
4. Kaynak kodu karta yükleyin (**Yükle / Upload**).
5. Sağ üst köşedeki **Seri Port Ekranı (Serial Monitor)** simgesine tıklayın ve hız ayarını **9600 baud** olarak seçin.

---

## Örnek Seri Port Çıktısı

```text
=== KONVEYOR URUN SAYIM VE TAKIP SISTEMI BASLATILDI ===
Hedef Uretim: 5
Urun Algilandi! Toplam Sayi: 1 / 5
Urun Algilandi! Toplam Sayi: 2 / 5
[BILGI] Uzun suredir urun gecmedi -> URETIMDE BEKLEME
Urun Algilandi! Toplam Sayi: 3 / 5
Urun Algilandi! Toplam Sayi: 4 / 5
Urun Algilandi! Toplam Sayi: 5 / 5
>>> URETIM HEDEFINE ULASILDI! <<<
```
