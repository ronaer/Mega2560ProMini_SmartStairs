/*Basamak sayısı: 14  +1 trabzan; Sensör: MZ80; IRF3708 Logic Mosfet; Arduino Mega
  Authored by Dr.TRonik youTube @Mart/2021
  Açık kaynak lisansı altında kullanılabilir, dağıtılabilir, geliştirilebilir...
  Konu ile ilgili soru ve sorunlar için: bilgi@ronaer.com
*/
/*------------
   Globals
  ------------
*/
const int mz_pin_up = 34; // merdivenin üstündeki mz sensor pini tanımlama
const int mz_pin_down = 35; // merdivenin altındaki mz sensor pini tanımlama
const int trabzan_pin = 46; //merdiven trabzan pini
int mz_up_deger; //Üst sensör bilgisinin tutulacağı değişken
int mz_down_deger;//Alt sensör bilgisinin tutulacağı değişken

int merdiven_led_up[14] =   { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 44, 45, }; // 14 adet led için PWM pinlerden dizi tanımı _ string (Mega'daki 15 pwm pinlerden)
int merdiven_led_down[14] = {45, 44, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,}; // 14 adet led için PWM dizi tanımı ters sırada_ reverse string

int b = 5000; // Tüm basamaklar yandıktan sonra geri sönmek için bekleme süresi 5 saniye sonra sönme başlar, değiştirebilirsiniz...
int pwm_sure_y = 2; // PWM yanma animasyon süresi, azalırsa çabuk yanar, artarsa yavaş yanar...
int pwm_sure_s = 8; // PWM sönme animasyon süresi, azalırsa çabuk söner, artarsa yavaş söner...


/*------------
   SetUp
  ------------
*/
void setup() {
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
   Loop
  ------------
*/
void loop()
{
  mz_up_deger = digitalRead(mz_pin_up); //üst mz pin değeri mz_up_deger e atandı
  mz_down_deger = digitalRead(mz_pin_down); //alt mz pin değeri mz_down_deger e atandı

  if (mz_up_deger == LOW) // eğer üst mz den sinyal gelirse (MZ80 aktif olduğunda LOW değeri döndürür)...
  {
    stairs_from_up(); // bu fonksiyonu çalıştır
  }


  if (mz_down_deger == LOW) // eğer alt mz den sinyal gelirse (MZ80 aktif olduğunda LOW değeri döndürür)...
  {
    stairs_from_down();  // bu fonksiyonu çalıştır
  }


  else // bu şartlardan hiçbiri olmazsa
  {
    kapat();  // bu fonksiyonu çalıştır
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

  for (int i = 0; i < 14; i++)   //14 adet basamak ledi...
  {
    for (int x = 0; x <= 255; x++) // pwm for döngüsü ile...
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
  }
  //en son trabzan pini de 5 saniye sonra pwm ile sönsün...

  delay(b);
  for (int x = 255; x >= 0; x--)
  {
    analogWrite(trabzan_pin, x);
    delay(pwm_sure_s);
  }
}

void stairs_from_down()  // alltan merdivene ayak basıldığında çalışacak fonksiyon
{
  digitalWrite(trabzan_pin, HIGH); //Önce trabzan aydınlatması yansın
  delay(100);
  
  for (int i = 0; i < 14; i++)  //14 adet basamak ledi...
  {
    for (int x = 0; x <= 255; x++) // pwm for döngüsü ile...
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
  }
  //en son trabzan pini de 5 saniye sonra pwm ile sönsün...

  delay(b);
  for (int x = 255; x >= 0; x--)
  {
    analogWrite(trabzan_pin, x);
    delay(pwm_sure_s);
  }

}

void kapat() // if komut şartlarının oluşmadığı durumda çalışacak fonksiyon
{
  for (int i = 0; i < 14; i++)
  {
    digitalWrite(merdiven_led_up[i], LOW);
    digitalWrite(merdiven_led_down[i], LOW);

  }
}
//YouTube>>>https://www.youtube.com/c/DrTRonik
