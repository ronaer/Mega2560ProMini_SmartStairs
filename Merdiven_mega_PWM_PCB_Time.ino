/*Basamak sayısı: 14  +1 trabzan; Sensör: MZ80; IRF3708 Logic Mosfet; Arduino Mega
  Authored by Dr.TRonik youTube @May/2021
  Açık kaynak lisansı altında kullanılabilir, dağıtılabilir, geliştirilebilir...
  Konu ile ilgili soru ve sorunlar için: bilgi@ronaer.com
*/
/*------------------------
   GlobalsGlobalsGlobals
  ------------------------
*/

//Kütüphaneler...
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"
#include <RCSwitch.h>

//Nesne Tanımları...
LiquidCrystal_I2C lcd(0x3F, 16, 2);
RTC_DS3231 rtc;
RCSwitch mySwitch = RCSwitch();

//Sabitler...
const int mz_pin_up = 34; // merdivenin üstündeki mz sensor pini tanımlama
const int mz_pin_down = 35; // merdivenin altındaki mz sensor pini tanımlama
const int trabzan_pin = 46; //merdiven trabzan pini
int mz_up_deger; //Üst sensör bilgisinin tutulacağı değişken
int mz_down_deger;//Alt sensör bilgisinin tutulacağı değişken

int merdiven_led_up[14] =  {45, 44, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,}; ; // 14 adet led için PWM pinlerden dizi tanımı _ string (Mega'daki 15 pwm pinlerden)
int merdiven_led_down[14] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, }; // 14 adet led için PWM dizi tanımı ters sırada_ reverse string

int b = 5000; // Tüm basamaklar yandıktan sonra geri sönmek için bekleme süresi 5 saniye sonra sönme başlar, değiştirebilirsiniz...
int pwm_sure_y = 2; // PWM yanma animasyon süresi, azalırsa çabuk yanar, artarsa yavaş yanar...
int pwm_sure_s = 8; // PWM sönme animasyon süresi, azalırsa çabuk söner, artarsa yavaş söner...

int tmp;
int LDR;
int pwm_time;
long zaman = 0;
long LDR_zaman = 0;
long lcd_light = 0;
String durum = "HAZIR";
int saat_;
boolean kilit = 0 ;
boolean hareket = 0;
boolean alrm = 0;

//LCD için ekran basamak görüntüleri...
byte sifir[8]   = {B10001,  B10001,  B11111,  B10001,  B10001,  B11111,  B10001,  B10101};
byte birinci[8] = {B10001,  B11111,  B10001,  B11111,  B10001,  B11111,  B10001,  B10101};
byte ikinci[8]  = {B10001,  B10001,  B11111,  B10001,  B10001,  B11111,  B10001,  B10101};
byte ucuncu[8]  = {B10001,  B11111,  B10001,  B11111,  B10001,  B11111,  B10001,  B10101};
byte dorduncu[8] = {B10001,  B10001,  B11111,  B10001,  B10001,  B11111,  B10001,  B10101};
byte besinci[8] = {B10001,  B11111,  B10001,  B11111,  B10001,  B11111,  B10001,  B10101};
byte altinci[8] = {B10001,  B10001,  B11111,  B10001,  B10001,  B11111,  B10001,  B10101};
byte yedinci[8] = {B10001,  B11111,  B10001,  B11111,  B10001,  B11111,  B10001,  B10101};


/*-------------------
   SetUp SetUp SetUp
  -------------------
*/
void setup()
{
  lcd.begin();
  lcd.noBacklight();
  lcd.clear();

  // oluşturulan karekterlerin lcd kütüphanesine tanıtılması...
  lcd.createChar(0, sifir);
  lcd.createChar(1, birinci);
  lcd.createChar(2, ikinci);
  lcd.createChar(3, ucuncu);
  lcd.createChar(4, dorduncu);
  lcd.createChar(5, besinci);
  lcd.createChar(6, altinci);
  lcd.createChar(7, yedinci);

  //Serial.begin(9600);


  mySwitch.enableReceive(4); // Rf alıcı, mega kart 4.kesme pini olan D19'a bağlandı...

  // Yazılım ile saati ayarlama, aşağıdaki satırı aktifleştirip yükledikten sonra satırı pasifleştirip kodu tekrar yüklemek gerekli!
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  pinMode(mz_pin_up, INPUT); // üstteki mz sensor pini giriş tanımlandı
  pinMode(mz_pin_down, INPUT); // alttaki mz sensor pini giriş tanımlandı

  for (int i = 0; i < 14; i++)
  {
    pinMode(merdiven_led_up[i], OUTPUT); //dizideki tüm ledler çıkış olarak ayarlandı
    pinMode(merdiven_led_down[i], OUTPUT); //dizideki tüm ledler çıkış olarak ayarlandı
    digitalWrite(merdiven_led_up[i], LOW); //dizideki tüm ledler başlangıçta LOW olarak ayarlandı
    digitalWrite(merdiven_led_down[i], LOW); //dizideki tüm ledler başlangıçta LOW olarak ayarlandı
  }
  pinMode(trabzan_pin, OUTPUT); //trabzan pinimiz de çıkış olarak ayarlandı

}

/*------------
   Loooooooop
  ------------
*/
void loop() {
  LDR = analogRead(A4);
  hareket = digitalRead(A0);
  alrm = digitalRead(A1);
  if (hareket == 1) {
    tam_yak();
  }
  if ((alrm == 1) && (LDR < 600)) {
    test_2(); //PIR sensor den HIGH gelirse VE ışık değeri düşüklüğü...
  }
  LDR_t ();
  basamak_parlaklik ();

  DateTime now = rtc.now();
  tmp =  (rtc.getTemperature() - 2);

  saat_ = now.hour(); // saat değişkenimiz saat modülünün saatinde,

  char saat[] = "hh:mm:ss"; //Saat kütüphane özelliği... Kullanalım...
  char tarih[] = "DD/MM/YY";

  if (mySwitch.available()) {
    //Seriali aktifleştirerek uzaktan kumandanın değerlerini serial porttan okumak için:
    Serial.print("Received ");
    Serial.print( mySwitch.getReceivedValue() );
    Serial.print(" / ");
    Serial.print( mySwitch.getReceivedBitlength() );
    Serial.print("bit ");
    Serial.print("Protocol: ");
    Serial.println( mySwitch.getReceivedProtocol() );
// Okuduğumuz kumanda değerlerini if bloklarına yazarak istediğimiz işlemi yaptırabilmek için:
    if (mySwitch.getReceivedValue() == 14478497) stairs_from_up();                  // 1.Tuş
    if (mySwitch.getReceivedValue() == 14478498) stairs_from_down();                // 2
    if (mySwitch.getReceivedValue() == 14478499) alt_ust();                         // 3
    if (mySwitch.getReceivedValue() == 14478500) test();                            // 4
    if (mySwitch.getReceivedValue() == 14478501) test_2();                          // 5
    if (mySwitch.getReceivedValue() == 14478502) tam_yak();                         // 6
    if (mySwitch.getReceivedValue() == 14478503) {
      kilit = 1;  // 7
      durum = "KiLiT+" ;
    }
    if (mySwitch.getReceivedValue() == 14478504) {
      kilit = 0;  // 8
      durum = "HAZIR" ;
    }


    mySwitch.resetAvailable();
  }
  Serial.println(LDR);

  mz_up_deger = digitalRead(mz_pin_up); //üst mz pin değeri mz_up_deger e atandı
  mz_down_deger = digitalRead(mz_pin_down); //alt mz pin değeri mz_down_deger e atandı

  if ((mz_up_deger == LOW) && (kilit == 0) && (((saat_ >= 17) && (saat_ <= 23)) || ((saat_ >= 0) && (saat_ <= 8))) ) // eğer üst mz den sinyal gelirse (MZ80 aktif olduğunda LOW değeri döndürür)...
  {
    stairs_from_up(); // bu fonksiyonu çalıştır
  }
  if ((mz_up_deger == LOW) && (kilit == 1)) {
    for (int i = 0; i < 14; i++)
    {
      digitalWrite(merdiven_led_up[i], LOW);


    }
  }

  if ((mz_down_deger == LOW) && (kilit == 0) && (((saat_ >= 17) && (saat_ <= 23)) || ((saat_ >= 0) && (saat_ <= 8))) ) // eğer alt mz den sinyal gelirse (MZ80 aktif olduğunda LOW değeri döndürür)...
  {
    stairs_from_down();  // bu fonksiyonu çalıştır
  }
  if ((mz_down_deger == LOW) && (kilit == 1)) {
    for (int i = 0; i < 14; i++)
    {
      digitalWrite(merdiven_led_up[i], LOW);


    }
  }


  //Her saniyede LCD saati ve durum bilgisini yazdırsın...
  if (millis() - zaman >= 1000)
  {
    zaman = millis() ;
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(now.toString(saat));
    lcd.setCursor(0, 1); lcd.print(now.toString(tarih));
    lcd.setCursor(12, 0); lcd.print(tmp); lcd.print("'C");
    lcd.setCursor(10, 1); lcd.print(durum);
  }
}
/*------------
   Fonksiyonlar
  ------------
*/
void stairs_from_up() // üstten merdivene ayak basıldığında çalışacak fonksiyon
{
  digitalWrite(trabzan_pin, HIGH); //Önce trabzan aydınlatması PWM olmadan yansın
  delay(100);

  lcd.backlight();
  lcd.clear();
  durum = "UST Sensor Aktif";
  lcd.setCursor(0, 0);
  lcd.print(durum);
  lcd.setCursor(15, 1); lcd.print("T");

  for (int i = 0; i < 14; i++)   //14 adet basamak ledi...
  {
    lcd.setCursor(13 - i, 1);    lcd.write(i);
    for (int x = 0; x <= pwm_time; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_up[i], x); //pwm ile 255 değerinden başlayarak  sıra ile...
      delay(pwm_sure_y);  // pwm_sure gecikmesi ile yansın.
    }

  }
  delay(b);
  //sıra ile ve PWM ile sönsün...
  for (int i = 0; i < 14; i++) // sıra for döngüsü
  {
    for (int x = 255; x >= 0; x--) // pwm for döngüsü
    {
      analogWrite(merdiven_led_up[i], x);
      delay(pwm_sure_s);
    }
    lcd.setCursor(13 - i, 1);    lcd.print(" ");
  }
  //en son trabzan pini de 5 saniye sonra pwm ile sönsün...

  delay(b);
  for (int x = 255; x >= 0; x--)
  {
    analogWrite(trabzan_pin, x);
    delay(pwm_sure_s);
  }
  //durum = "HAZIR";
  if (kilit == 0) durum = "HAZIR";
  if (kilit == 1) durum = "KiLiT+";
  lcd.noBacklight();
}

void stairs_from_down()  // alltan merdivene ayak basıldığında çalışacak fonksiyon
{
  digitalWrite(trabzan_pin, HIGH); //Önce trabzan aydınlatması yansın
  delay(100);
  lcd.backlight();
  lcd.clear();
  durum = "ALT Sensor Aktif";
  lcd.setCursor(0, 0); lcd.print(durum); lcd.setCursor(15, 1); lcd.print("T");

  for (int i = 0; i < 14; i++)  //14 adet basamak ledi...
  {
    lcd.setCursor(i, 1);    lcd.write(i);

    for (int x = 0; x <= pwm_time; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_down[i], x);//pwm ile 0 değerinden başlayarak  sıra ile...
      delay(pwm_sure_y);  // pwm_sure gecikmesi ile yansın.
    }
  }
  delay(b);
  //sıra ile PWM ile sönsün...
  for (int i = 0; i < 14; i++)
  {



    for (int x = 255; x >= 0; x--)
    {
      analogWrite(merdiven_led_down[i], x);
      delay(pwm_sure_s);
    }
    lcd.setCursor(i, 1);    lcd.print(" ");
  }

  //en son trabzan pini de 5 saniye sonra pwm ile sönsün...

  delay(b);
  for (int x = 255; x >= 0; x--)
  {
    analogWrite(trabzan_pin, x);
    delay(pwm_sure_s);
  }
  //durum = "HAZIR";
  if (kilit == 0) durum = "HAZIR";
  if (kilit == 1) durum = "KiLiT+";
  lcd.noBacklight();
}



void alt_ust() //
{ if (kilit == 0) durum = "HAZIR";
  if (kilit == 1) durum = "KiLiT+";
  analogWrite(trabzan_pin, 25);
  analogWrite(merdiven_led_up[0], 25);
  analogWrite(merdiven_led_up[13], 25);

}

void LDR_t() {
  if ((millis() - LDR_zaman) >= 30000)
  {
    LDR_zaman = millis() ;
    //LDR = analogRead(A4);
    //Serial.println(LDR);
    if (LDR < 650) alt_ust();

    else {
      digitalWrite (2 , LOW);
      digitalWrite (45, LOW);
      digitalWrite (46, LOW);

    }
  }
}
void basamak_parlaklik() {

  //LDR = analogRead(A4);
  if ((millis() - lcd_light) >= 2000)
  {
    lcd_light = millis();
    if (LDR < 650) {
      pwm_time = 30;
      pwm_sure_y = 8;
      lcd.backlight();
    }
    else {
      pwm_time = 255;
      pwm_sure_y = 2;
      lcd.noBacklight();
    }
  }
}
//-----------TEST Fonksiyonu----------
void test () {
  digitalWrite(trabzan_pin, HIGH); //Önce trabzan aydınlatması PWM olmadan yansın
  delay(100);

  lcd.backlight();
  lcd.clear();
  durum = "TEST MOD AKTiF";
  lcd.setCursor(1, 0);
  lcd.print(durum);
  lcd.setCursor(15, 1); lcd.print("T");

  for (int i = 0; i < 5; i++)   //14 adet basamak ledi...
  {
    lcd.setCursor(13 - i, 1);    lcd.write(i);
    for (int x = 0; x <= 25; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_up[i], x); //pwm ile 255 değerinden başlayarak  sıra ile...
      delay(1);  // pwm_sure gecikmesi ile yansın.
    }

  }
  for (int i = 0; i < 8; i++)   //14 adet basamak ledi...
  {
    lcd.setCursor(13 - i, 1);    lcd.write(i);
    for (int x = 25; x <= 150; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_up[i], x); //pwm ile 255 değerinden başlayarak  sıra ile...
      delay(1);  // pwm_sure gecikmesi ile yansın.
    }

  }

  for (int i = 0; i < 14; i++)   //14 adet basamak ledi...
  {
    lcd.setCursor(13 - i, 1);    lcd.write(i);
    for (int x = 150; x <= 255; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_up[i], x); //pwm ile 255 değerinden başlayarak  sıra ile...
      delay(1);  // pwm_sure gecikmesi ile yansın.
    }

  }

  delay(100);
  //sıra ile ve PWM ile sönsün...
  for (int i = 0; i < 5; i++) // sıra for döngüsü
  {
    for (int x = 255; x >= 150; x--) // pwm for döngüsü
    {
      analogWrite(merdiven_led_up[i], x);
      delay(1);
    }
    lcd.setCursor(13 - i, 1);    lcd.print(" ");
  }
  ////
  for (int i = 0; i < 8; i++) // sıra for döngüsü
  {
    for (int x = 150; x >= 50; x--) // pwm for döngüsü
    {
      analogWrite(merdiven_led_up[i], x);
      delay(1);
    }
    lcd.setCursor(13 - i, 1);    lcd.print(" ");
  }
  ///
  for (int i = 0; i < 14; i++) // sıra for döngüsü
  {
    for (int x = 50; x >= 0; x--) // pwm for döngüsü
    {
      analogWrite(merdiven_led_up[i], x);
      delay(3);
    }
    lcd.setCursor(13 - i, 1);    lcd.print(" ");
  }

  delay(100);

  //////////////////////////////

  digitalWrite(trabzan_pin, HIGH); //Önce trabzan aydınlatması yansın
  delay(100);

  for (int i = 0; i < 7; i++)  //14 adet basamak ledi...
  {
    lcd.setCursor(i, 1);    lcd.write(i);

    for (int x = 0; x <= 10; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_down[i], x);//pwm ile 0 değerinden başlayarak  sıra ile...
      delay(5);
    }
  }
  delay(500);

  for (int i = 7; i < 14; i++)  //14 adet basamak ledi...
  {
    lcd.setCursor(i, 1);    lcd.write(i);

    for (int x = 0; x <= 50; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_down[i], x);//pwm ile 0 değerinden başlayarak  sıra ile...
      delay(5);
    }
  }
  delay(200);
  for (int i = 0; i < 14; i++)  //14 adet basamak ledi...
  {
    lcd.setCursor(i, 1);    lcd.write(i);

    for (int x = 100; x <= 150; x++) // pwm for döngüsü ile...
    {
      analogWrite(merdiven_led_down[i], x);//pwm ile 0 değerinden başlayarak  sıra ile...
      delay(2);
    }
  }
  //sıra ile PWM ile sönsün...
  for (int i = 0; i < 14; i++)
  {

    for (int x = 255; x >= 0; x--)
    {
      analogWrite(merdiven_led_down[i], x);
      delay(1);
    }
    lcd.setCursor(i, 1);    lcd.print(" ");
  }

  //en son trabzan pini de 5 saniye sonra pwm ile sönsün...

  delay(100);
  for (int x = 255; x >= 0; x--)
  {
    analogWrite(trabzan_pin, x);
    delay(1);
  }

  //durum = "HAZIR";
  if (kilit == 0) durum = "HAZIR";
  if (kilit == 1) durum = "KiLiT+";
  lcd.noBacklight();
}

//__________VOİID TEST_2________________
void test_2 () {
  for (int i = 0; i < 14; i++)
  {

    digitalWrite(merdiven_led_up[i], LOW);
  }
  lcd.backlight();
  lcd.clear();
  durum = "HAREKET!";
  lcd.setCursor(0, 0);
  lcd.print(durum);
  lcd.setCursor(6, 1);
  lcd.print("ALGILANDI!");
  int i = 200;
  while (i != 0)
  {

    digitalWrite (2, HIGH); digitalWrite (4, HIGH); digitalWrite (6, HIGH); digitalWrite (8, HIGH);
    digitalWrite (9, HIGH); digitalWrite (11, HIGH); digitalWrite (13, HIGH); digitalWrite (45, HIGH);
    digitalWrite (trabzan_pin, LOW);
    delay(i);
    digitalWrite (2, LOW); digitalWrite (4, LOW); digitalWrite (6, LOW); digitalWrite (8, LOW);
    digitalWrite (9, LOW); digitalWrite (11, LOW); digitalWrite (13, LOW); digitalWrite (45, LOW);
    digitalWrite (trabzan_pin, HIGH);
    delay(i);
    i -= 5;
  }

  digitalWrite (trabzan_pin, LOW);
  //durum = "HAZIR";
  if (kilit == 0) durum = "HAZIR";
  if (kilit == 1) durum = "KiLiT+";
  lcd.noBacklight();
}

//__________VOID TAM_YAK__________
void tam_yak () {

  lcd.backlight();
  lcd.clear();
  durum = "TUM LEDLER ACIK";
  lcd.setCursor(0, 0);
  lcd.print(durum);

  digitalWrite (trabzan_pin, HIGH);
  delay(100);

  for (int i = 0; i < 14; i++)
  {

    digitalWrite(merdiven_led_up[i], HIGH);
  }
  delay(2000);
  durum = "DiKKAT";
}



//YouTube>>>https://www.youtube.com/c/DrTRonik
