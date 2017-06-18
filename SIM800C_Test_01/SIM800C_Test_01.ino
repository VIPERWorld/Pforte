/*
Программа передачи данных по каналу GPRS
31.03.2017г.



*/


#include "SIM800.h"
#include <SoftwareSerial.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include <avr/wdt.h>
#include <MsTimer2.h> 
#include <Wire.h>
#include "MCP23017.h"


MCP23017 mcp;

#define con Serial
#define speed_Serial 115200

#define PIN_TX           7                              // Подключить  к выводу 7 сигнал RX модуля GPRS
#define PIN_RX           8                              // Подключить  к выводу 8 сигнал TX модуля GPRS

SoftwareSerial SIM800CSS = SoftwareSerial(PIN_RX, PIN_TX);
SoftwareSerial *GPRSSerial = &SIM800CSS;
 

#define PWR_On           5                              // Включение питания модуля SIM800
#define SIM800_RESET_PIN 6                              // Сброс модуля SIM800
#define LED13           13                              // Индикация светодиодом
#define NETLIGHT         3                              // Индикация NETLIGHT
#define STATUS           9                              // Индикация STATUS
#define analog_dev1      A5                             // Аналоговый вход 1
#define digital_inDev2   12                             // Цифровой вход 12
#define digital_outDev3  A4                             // Цифровой выход управления внешним устройством


												
//#define COMMON_ANODE                                  // Если светодиод с общим катодом - раскомментировать
#define LED_RED      A0                                 // Индикация светодиодом RED
#define LED_GREEN    A1                                 // Индикация светодиодом GREEN
#define LED_BLUE     A2                                 // Индикация светодиодом BLUE

#define COLOR_NONE LOW, LOW, LOW                        // Отключить все светодиоды
#define COLOR_GREEN LOW, HIGH, LOW                      // Включить зеленый светодиод
#define COLOR_BLUE LOW, LOW, HIGH                       // Включить синий светодиод
#define COLOR_RED HIGH, LOW, LOW                        // Включить красный светодиод
volatile int stateLed = LOW;                            // Состояние светодиода при прерывистой индикации на старте
volatile int state_device = 0;                          // Состояние модуля при запуске 

														// 1 - Не зарегистрирован в сети, поиск
														// 2 - Зарегистрировано в сети
														// 3 - GPRS связь установлена
volatile int metering_NETLIGHT       = 0;
volatile unsigned long metering_temp = 0;
volatile int count_blink1            = 0;               // Счетчик попыток подключиться к базовой станции
volatile int count_blink2            = 0;               // Счетчик попыток подключиться к базовой станции
bool send_ok                         = false;           // Признак успешной передачи данных
bool count_All_reset                 = false;           // Признак выполнения команды сброса счетчика ошибок.
bool temp_dev3 = false;                                 // Переменная для временного хранения состояния исполнительного устройства
String imei = "";                        // Тест IMEI
String SIMCCID = "";

CGPRS_SIM800 gprs;
int count                     = 0;

unsigned long time             = 0;                     // Переменная для суточного сброса
unsigned long time_day         = 86400;                 // Переменная секунд в сутках
unsigned long previousMillis   = 0;                     //  
unsigned long interval         = 360;                   // Интервал передачи данных 5 минут
bool time_set                  = false;                 // Фиксировать интервал заданный СМС

bool watch_dog                 = false;                 // Признак проверки сторожевого таймера
unsigned long time_ping        = 380;                   // Интервал проверки ping 6 минут. 
unsigned long previousPing     = 0;                     // Временный Интервал проверки ping

int Address_tel1            = 100;                        // Адрес в EEPROM телефона 1
int Address_ssl             = 120;                        // Адрес в EEPROM признака шифрования

int Address_interval        = 200;                        // Адрес в EEPROM величины интервала

int Address_Dev3            = 260;                        // Адрес в EEPROM состояния исполнительного устройства Dev
int Address_Dev3_ind        = 264;                        // Адрес в EEPROM признак управления сполнительного устройства Dev
int Address_num_site_ping   = 268;                        // Адрес в EEPROM признак управления сполнительного устройства Dev
int Address_watchdog        = 270;                        // Адрес в EEPROM счетчик проверки Watchdog
int Address_EEPROM_off      = 280;                        // Адрес в EEPROM запрет записи в EEPROM
byte dev3                   = 0;                          // признак управления сполнительного устройства Dev3



void flash_time()                                       // Программа обработчик прерывистого свечения светодиодов при старте
{
	if (state_device == 0)
	{
		setColor(COLOR_RED);
	}
	if (state_device == 1)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_RED);
		}
		else
		{
			setColor(COLOR_NONE);
		}
	}

	if (state_device == 2)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_NONE);
		}
		else
		{
			setColor(COLOR_BLUE);
		}
	}

	if (state_device == 3)
	{
		stateLed = !stateLed;
		if (!stateLed)
		{
			setColor(COLOR_NONE);
		}
		else
		{
			setColor(COLOR_GREEN);
		}
	}
}

 void setColor(bool red, bool green, bool blue)        // Включение цвета свечения трехцветного светодиода.
 {
	  #ifdef COMMON_ANODE                              // Если светодиод с общим катодом
		red = !red;
		green = !green;
		blue = !blue;
	  #endif 
	  digitalWrite(LED_RED, red);
	  digitalWrite(LED_GREEN, green);
	  digitalWrite(LED_BLUE, blue);    
 }
 


void connect_internet_HTTP()
{
	bool setup_ok = false;
	int count_init = 0;
	do
	{
		Serial.println(F("\nInternet HTTP connect .."));
		
		byte ret = gprs.connect_GPRS();                                              // Подключение к GPRS
		if (ret == 0)
		{
			while (state_device != 3)                            // Ожидание регистрации в сети
			{
				delay(50);
			}

			con.println(F("Wait IP"));
			gprs.connect_IP_GPRS();                             // Получить IP адрес
			Serial.println(F("Internet connect OK!-"));
			setup_ok = true;
		}
		else                                                    // Модуль не подключиля к интернету
		{
			count_init++;                                        // Увеличить счетчик количества попыток
			if (count_init > 10)
			{
				gprs.reboot(gprs.errors);                       // вызываем reset после 10 ошибок
			}

			Serial.println(F("\nFailed connect internet"));
			delay(1000);
			if (state_device == 3)                               // Модуль одумался и все таки подключиля к интернету
			{
				con.println(F("Wait IP"));
				gprs.connect_IP_GPRS();                          // Получить IP адрес
				Serial.println(F("Internet connect OK!-"));
				setup_ok = true;
			}
		}
	} while (!setup_ok);             // 
}

int freeRam()                                   // Определить свободную память
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTime(String val, String f_phone)
{
  if (val.indexOf(F("Timeset")) > -1)                   // 
  {
	 interval = 40;                                     // Установить интервал 40 секунд
	 time_set = true;                                   // Установить фиксацию интервала заданного СМС
	 Serial.println(interval);
	 gprs.send_sms(val, f_phone);                         //процедура отправки СМС
  } 
  else if (val.indexOf(F("Restart")) > -1) 
  {
	  Serial.print(f_phone);
	  Serial.print("..");
	  Serial.println(F("Restart"));
	  gprs.send_sms(val, f_phone);                      //процедура отправки СМС
	  delay(2000);
	  gprs.reboot(gprs.errors);                         // вызываем reset
  } 
  else if (val.indexOf(F("Timeoff")) > -1) 
  {
	 time_set = false;                                  // Снять фиксацию интервала заданного СМС
	 Serial.println(F("Timeoff"));
	 gprs.send_sms(val, f_phone);                       //процедура отправки СМС
  } 
  else if (val.indexOf(F("Eon")) > -1)
  {
	  EEPROM.write(Address_EEPROM_off, true);           // Включить 
	  Serial.println(F("EEPROM ON"));
	  gprs.send_sms("EEPROM ON", f_phone);              //процедура отправки СМС
  }
  else if (val.indexOf(F("Eoff")) > -1)
  {
	  EEPROM.write(Address_EEPROM_off, false);          // Отключить
	  Serial.println(F("EEPROM OFF"));
	  gprs.send_sms("EEPROM OFF", f_phone);             //процедура отправки СМС
  }
  else if (val.indexOf(F("Devon")) > -1)
  {
	  EEPROM.write(Address_Dev3, 1);                   // Включить исполнительное устройство
	  EEPROM.write(Address_Dev3_ind, 1);
	  con.println(F("Device ON"));
	  gprs.send_sms("Device ON", f_phone);             //процедура отправки СМС
  }
  else if (val.indexOf(F("Devoff")) > -1)
  {
	  EEPROM.write(Address_Dev3, 0);                   // Отключить исполнительное устройство
	  EEPROM.write(Address_Dev3_ind, 0);               // 
	  con.println(F("Device OFF"));
	  gprs.send_sms("Device OFF", f_phone);            //процедура отправки СМС
  }
  else
  {
	  Serial.println(F("Unknown command"));            // Serial.println("Unknown command");
	  gprs.send_sms("Unknown command "+ val, f_phone); //процедура отправки СМС
  }
}

void check_blink()
{
	unsigned long current_M = millis();
	wdt_reset();
	metering_NETLIGHT = current_M - metering_temp;                            // переделать для  
	metering_temp = current_M;

	if (metering_NETLIGHT > 3055 && metering_NETLIGHT < 3070)
	{
		state_device = 2;                                                     // 2 - Зарегистрировано в сети
		count_blink2++;
		if (count_blink2 > 250)
		{
			state_device = 0;
			MsTimer2::stop();                                                 // Включить таймер прерывания
			gprs.reboot(gprs.errors);                                                    // Что то пошло не так с регистрацией на станции
		}
	}
	else if (metering_NETLIGHT > 855 && metering_NETLIGHT < 870)
	{
		state_device = 1;                                                     // 1 Не зарегистрирован в сети, поиск
		count_blink1++;
		if (count_blink1 > 250) 
		{
			state_device = 0;
			MsTimer2::stop();                                                 // Включить таймер прерывания
			gprs.reboot(gprs.errors);                                                    // Что то пошло не так с регистрацией на станции
		}
	}
	else if (metering_NETLIGHT > 350 && metering_NETLIGHT < 370)
	{
		state_device = 3;                                                     // 3 - GPRS связь установлена
									  
	}
}


void check_SMS()
{

	if (gprs.checkSMS())
	{
		con.print(F("SMS:"));
		con.println(gprs.val);

		if (gprs.val.indexOf("REC UNREAD") > -1)  //если обнаружена новая  СМС 
		{
			//------------- поиск кодового слова в СМС 
			char buf[16];

			EEPROM.get(Address_tel1, buf);                                         // Восстановить телефон хозяина 1
			String master_tel1(buf);


			if (gprs.deleteSMS(1))
			{
				con.println(F("SMS delete"));                    //      con.print("SMS:");
			}

			if (gprs.val.indexOf(master_tel1) > -1)                              //если СМС от хозяина 1
			{
				con.println(F("Commanda tel1"));
				setTime(gprs.val, master_tel1);
			}
			else
			{
				con.println(F("Phone ignored"));
				gprs.send_sms("Phone ignored !! " + gprs.val, master_tel1);              //процедура отправки СМС Изменить в библиотеке
			}
		}

		if (gprs.val.indexOf("REC READ") > -1)                   //если обнаружена старая  СМС 
		{
			if (gprs.deleteSMS(0))
			{
				con.println(F("SMS delete"));                    //  con.print("SMS:");
			}
		}
		gprs.val = "";
	}
}


void ping()
{
	int count_ping = 0;
	int count_wait = 0;                                               // Счетчик количества попыток проверки подключения Network registration (сетевому оператору)
	byte ret = gprs.ping_connect_internet();

	if (ret != 0)
	{
		digitalWrite(PWR_On, HIGH);                                   // отключаем питание модуля GPRS
		gprs.reboot(gprs.errors);                                                // Что то пошло не так с интернетом
	}

	while (state_device != 3)                                         // Ожидание подключения к интернету
	{
		delay(50);
		count_wait++;
		if (count_wait > 3000)
		{
			digitalWrite(PWR_On, HIGH);                               // отключаем питание модуля GPRS
			gprs.reboot(gprs.errors);                                            //вызываем reset при отсутствии доступа к  интернету
		}
	}

	while (1)
	{

		if (state_device != 3)
		{
			digitalWrite(PWR_On, HIGH);                             //  отключаем питание модуля GPRS 
			gprs.reboot(gprs.errors);                                          //вызываем reset при отсутствии доступа к  интернету
		}
		if (check_ping())
		{
			return;
		}
		else
		{
			count_ping++;
			if (count_ping > 7)
			{
				digitalWrite(PWR_On, HIGH);                         //  отключаем питание модуля GPRS
				gprs.reboot(gprs.errors);                                      // 7 попыток. Что то пошло не так с интернетом
			}
		}
		delay(1000);
	}
}


bool check_ping()
{
	static const char* url_ping1 = "www.ya.ru";
	static const char* url_ping2 = "www.google.com";

	con.print(F("Ping -> "));
	con.println(url_ping1);
	if (gprs.ping(url_ping1))
	{
		con.println(F(".. Ok!"));
		return true;
	}
	else
	{
		con.print(F("\nPing -> "));
		con.println(url_ping2);
		if (gprs.ping(url_ping2))
		{
			con.println(F(".. Ok!"));
			return true;
		}
	}
	con.println(F(".. false!"));
	return false;
}

void setup_MCP23017()
{
	for (int i = 8; i < 16; i++)
	{
		mcp.pinMode(i, INPUT);
		mcp.digitalWrite(i, HIGH);
	}
	for (int i = 0; i < 8; i++)
	{
		mcp.pinMode(i, OUTPUT);
		mcp.digitalWrite(i, LOW);
	}

}

void test_MCP23017()
{
	for (int i = 8; i < 16; i++)
	{
		Serial.println(mcp.digitalRead(i));

	}
	for (int i = 0; i < 8; i++)
	{
		mcp.digitalWrite(i, HIGH);
		delay(2000);
		mcp.digitalWrite(i, LOW);
		delay(2000);
	}

}


void start_init()
{
	bool setup_ok = false;
	int count_init = 0;
	MsTimer2::start();                                                 // Включить таймер прерывания
	do
	{
		con.println(F("Initializing....(May take 5-10 seconds)"));

		digitalWrite(SIM800_RESET_PIN, LOW);                      // Сигнал сброс в исходное состояние
		digitalWrite(LED13, LOW);
		digitalWrite(PWR_On, HIGH);                               // Кратковременно отключаем питание модуля GPRS
		while (digitalRead(STATUS) != LOW) 
		{
			delay(100);
		}
		delay(1000);
		digitalWrite(LED13, HIGH);
		digitalWrite(PWR_On, LOW);
		delay(1000);                                              // 
		digitalWrite(SIM800_RESET_PIN, HIGH);                     // Производим сброс модема после включения питания
		delay(1200);
		digitalWrite(SIM800_RESET_PIN, LOW);
		int count_status = 0;
		while (digitalRead(STATUS) == LOW)
		{
			count_status++;
			if (count_status > 100)
			{
				gprs.reboot(gprs.errors);                                     // 100 попыток. Что то пошло не так программа перезапуска  если модуль не включился
			}
			delay(100);
		}
	
		con.println(F("Power SIM800 On"));
		GPRSSerial->begin(19200);                               // Скорость обмена с модемом SIM800C

		while (!gprs.begin(*GPRSSerial))                        // Настройка модуля SIM800C
		{
			Serial.println(F("Couldn't find module GPRS"));
			while (1);
		}
		con.println(F("OK"));
		Serial.print(F("\nSetting up mobile network..."));
		while (state_device != 2)                                // Ожидание регистрации в сети
		{
			Serial.print(F("."));
			delay(1000);
		}
		delay(1000);

		if (gprs.getIMEI())                                     // Получить IMEI
		{
			con.print(F("\nIMEI:"));
			imei = gprs.buffer;                                 // Отключить на время отладки
			gprs.cleanStr(imei);                                // Отключить на время отладки
			con.println(imei);
		}

		if (gprs.getSIMCCID())                               // Получить Номер СИМ карты
		{
			con.print(F("\nSIM CCID:"));
			SIMCCID = gprs.buffer1;
			gprs.cleanStr(SIMCCID);
			con.println(SIMCCID);
		}


		char n = gprs.getNetworkStatus();
		
		Serial.print(F("\nNetwork status "));
		Serial.print(n);
		Serial.print(F(": "));
		if (n == '0') Serial.println(F("\nNot registered"));                      // 0 – не зарегистрирован, поиска сети нет
		if (n == '1') Serial.println(F("\nRegistered (home)"));                   // 1 – зарегистрирован, домашняя сеть
		if (n == '2') Serial.println(F("\nNot registered (searching)"));          // 2 – не зарегистрирован, идёт поиск новой сети
		if (n == '3') Serial.println(F("\nDenied"));                              // 3 – регистрация отклонена
		if (n == '4') Serial.println(F("\nUnknown"));                             // 4 – неизвестно
		if (n == '5') Serial.println(F("\nRegistered roaming"));                  // 5 – роуминг

		if (n == '1' || n == '5')                                                 // Если домашняя сеть или роуминг
		{
			if (state_device == 2)                                                // Проверить аппаратно подключения модема к оператору
			{
				do
				{
					byte signal = gprs.getSignalQuality();
					Serial.print(F("rssi ..")); Serial.println(signal);
					delay(1000);
					Serial.println(F("GPRS connect .."));
					gprs.getOperatorName();
					setup_ok = true;

				} while (!setup_ok);              
			}
		}
	} while (count_init > 30 || setup_ok == false);    // 30 попыток зарегистрироваться в сети
}

void setup()
{
	wdt_disable(); // бесполезна¤ строка до которой не доходит выполнение при bootloop Не уверен!!
	con.begin(speed_Serial);
	con.println(F("\nSIM800 setup start"));     
	
	pinMode(SIM800_RESET_PIN, OUTPUT);
	digitalWrite(SIM800_RESET_PIN, LOW);            // Сигнал сброс в исходное состояние
	pinMode(LED13, OUTPUT);
	pinMode(PWR_On, OUTPUT);

	pinMode(LED_RED,  OUTPUT);
	pinMode(LED_BLUE, OUTPUT);
	pinMode(LED_GREEN,OUTPUT);
	pinMode(NETLIGHT ,INPUT);                      // Индикация NETLIGHT
	pinMode(STATUS ,INPUT);                        // Индикация STATUS

	pinMode(digital_inDev2, INPUT);                // Цифровой вход 12
	digitalWrite(digital_inDev2, HIGH);            // Цифровой вход 12
	pinMode(digital_outDev3, OUTPUT); 	           // Цифровой выход
	digitalWrite(digital_outDev3, HIGH); 	       // Цифровой выход

	setColor(COLOR_RED);
	delay(300);
	setColor(COLOR_GREEN); 
	delay(300);
	setColor(COLOR_BLUE);
	delay(300); 
	setColor(COLOR_RED);

	mcp.begin(1);      // use default address 0
	setup_MCP23017();

	wdt_enable(WDTO_8S);                                       // Для тестов не рекомендуется устанавливать значение менее 8 сек.
	
	int setup_EEPROM = 42;                                     // Произвольное число. Программа записи начальных установок при первом включении устройства после монтажа.
	if(EEPROM.read(0)!= setup_EEPROM)                          // Программа записи начальных установок при первом включении устройства после монтажа.
	{
		con.println (F("Start clear EEPROM"));                 //  
		for(int i = 0; i<1023;i++)
		{
			EEPROM.write(i,0); 
		}
		EEPROM.write(0, setup_EEPROM);                         //Записать для предотвращения повторной записи установок
		EEPROM.put(Address_interval, interval);                // строка начальной установки интервалов
		EEPROM.put(Address_tel1, "+79162632701");
	
		con.println(F("Test Watchdog"));
		for (int i = 0; i < 15; i++)                         // Если счетчик досчитает больше 9, значит  Watchdog не работает
		{
			EEPROM.write(Address_watchdog, i);               // Если произойдет перезагрузка до 9 - Watchdog работает
			con.println(i);
			delay(1000);
		}
		con.println (F("Clear EEPROM End"));                              
	}
	
	byte watchdog = EEPROM.read(Address_watchdog);
	if (watchdog > 10)  con.println(F("Watchdog off"));
	else  con.println(F("\n** Watchdog on **"));
	
	attachInterrupt(1, check_blink, RISING);                     // Включить прерывания. Индикация состояния модема
	EEPROM.get(Address_interval, interval);                      // Получить из EEPROM интервал

	con.print(F("Interval sec:"));
	con.println(interval);
	con.print(F("\nfree memory: "));
	con.println(freeRam());

	
	
	MsTimer2::set(300, flash_time);                            // 300ms период таймера прерывани
	start_init();
	
	int count_init = 0;                                        // Счетчик количества попыток подключиться к HTTP
	con.println(F("OK"));
	
		if (gprs.deleteSMS(0))
		{
			con.println(F("All SMS delete"));                    // 
		}
	//	ping();
 	MsTimer2::stop();
	setColor(COLOR_GREEN);                                      // Включить зеленый светодиод
	time = millis();                                            // Старт отсчета суток
	
	con.println(F("\nSIM800 setup end"));
}

void loop()
{

	if (digitalRead(STATUS) == LOW)
	{
		gprs.reboot(gprs.errors);                                     // Что то пошло не так программа перезапуска  если модуль не включился
	}

	test_MCP23017();
    check_SMS();                   // Проверить приход новых СМС

 unsigned long currentMillis = millis();
	if(!time_set)                                                               // 
	{
		 EEPROM.get( Address_interval, interval);                               // Получить интервал из EEPROM Address_interval
	}
	if ((unsigned long)(currentMillis - previousMillis) >= interval*1000) 
	{
		con.print(F("Interval sec:"));                                       
		con.println((currentMillis-previousMillis)/1000);
		previousMillis = currentMillis;
		ping();
		con.print(F("\nfree memory: "));                                 
		con.println(freeRam());
	}

	currentMillis = millis();

	if ((unsigned long)(currentMillis - previousPing) >= time_ping * 1000)
	{
		con.print(F("Interval ping sec:"));
		con.println((currentMillis - previousPing) / 1000);
		setColor(COLOR_BLUE);
		previousPing = currentMillis;
		setColor(COLOR_GREEN);
	}

	dev3 = EEPROM.read(Address_Dev3);

	if (dev3 != temp_dev3)
	{
		temp_dev3 = dev3;
		if (dev3 == 0)
		{
			con.println(F("Device OFF"));
			digitalWrite(digital_outDev3, HIGH); 	       // Цифровой выход
		}
		else
		{
			con.println(F("Device ON"));
			digitalWrite(digital_outDev3, LOW); 	       // Цифровой выход
		}
	}

	if(millis() - time > time_day*1000)
	{
		gprs.reboot(gprs.errors);                         // вызываем reset интервалом в сутки
	}
	delay(500);
}
