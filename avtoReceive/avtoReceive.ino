
/*

Программа приемника!!!


Необходимо написать программу для светового модуля прицепа радиоуправляемого грузовика.
Изначальный сигнал берется с фар модели грузовика, 7 пар контактов (+ и -).
Необходимо снять с каждой пары контактов состояние включено/выключено и если возможно то напряжение.
Через ИК передатчик передать это на вторую плату.
На второй плате, получаем сигнал с ИК приемника и выводим на 7 выходов для подключения светодиодов и если возможно, то с таким же напряжением как на основной плате.
Вот прочитав про использование ШИМ в регулировке яркости диода подумал, что в блоке света на грузовике яркость регулируется также, но не уверен.
Опишу все подробнее:
Для диодов необходимо всего пять выходов (тормоз, габариты, задний ход, и 2 поворотника), остальные два выхода необходимы для замыкания
простых релюшек под ток 1-2А. (для подъема и опускание опор прицепа).
На световом модуле, как правило, плюс постоянный а размыкается минус, 5 таких выходов. Соответственно мы можем взять сигнал, грубо говоря, с диода. 
А остальные 2 просто на замыкание контакта (нормально они будут разомкнуты).
Использование ИК принципиально по конструкции. ИК передатчик устанавливается в центр седла в грузовике, а ИК приемник в крюке на прицепе. 
Таким образом когда прицеп зацеплен фары работают, если отцеплен не работают.
На втором контроллере, надо получить 5 выходов под свет (тормоз, габариты, задний ход, и 2 поворотника) и два под реле.
Для управления диодами и реле нужна еще микросхема драйвера типа ULN2003 или ULN2803.
Вместо драйвера можно применить транзисторы типа NPN.

 Тест передачи / приема функции IRremote, используя пару Arduinos.
 *
 * Ардуино # 1 должен иметь светодиодный ИК, подключенный к посыла штифтом (3).
 * Arduino # 2 должен иметь ИК-детектор / демодулятор, подключенного к разъему
 * Получить штифт (11) и видимый светодиод, подключенный к контакту 3.
 *
 * Цикл:
 * Arduino # 1 будет ждать 2 секунды, а затем запустить через испытания.
 * Она повторяет это навсегда.
 * Arduino # 2 будет ждать в течение по крайней мере одну секунду отсутствия сигнала
 * (Для синхронизации с # 1). Он будет ожидать того же теста
 * Сигналы. Он будет регистрировать все состояние к последовательному порту. Это будет
 * Также указать статус через светодиод, который будет мигать каждый раз, когда тест
 * выполнен. Если есть ошибка, то она будет гореть в течение 5 секунд.
 *
 * Тест проходит, если светодиод мигает 19 раз, делает паузу, а затем повторяется.
 * Тест терпит неудачу, если светодиод горит в течение 5 секунд.
 *
 * Тест программа автоматически определяет, какая плата является отправителем и который
 * Приемник, ища вход на передающем штифтом, который будет указывать
 * Отправитель. Вы должны подключить последовательный порт к приемнику для отладки.

 1 2FD807F
 2 2FD40BF
 3 2FDC03F
 4 2FD20DF
 5 2FDA05F
 6 2FD609F
 7 2FDE01F
 8 2FD10EF
 9 2FD906F
 0 2FD00FF
 +v 2FD58A7
 -v 2FD7887
 +p 2FDD827
 -p 2FDF807
 AV 2FD28D7
fav 2FD8877
 */

#include <IRremote.h>
#include <Wire.h>

#define out_run_stop   4          // Сигнал выходной STOP
#define out_fara_size  5          // Сигнал выходной Габариты
#define out_run_rear   6          // Сигнал выходной Назад 
#define out_fara_left  7          // Сигнал выходной Фара левая
#define out_fara_right 8          // Сигнал выходной Фара правая
#define out_rele1      9          // Сигнал выходной управления реле 1
#define out_rele2     10          // Сигнал выходной управления реле 2

#define in_test       12          // Сигнал входной кнопка тест 

bool stop = false;
bool size = false;
int stop_size = 60;

int RECV_PIN = 11;

IRrecv irrecv (RECV_PIN);

decode_results results;


void set_pin()
{
    pinMode(out_run_stop,  OUTPUT);     // Сигнал выходной STOP
    pinMode(out_fara_size, OUTPUT);     // Сигнал выходной Габариты
    pinMode(out_run_rear,  OUTPUT);     // Сигнал выходной Назад 
    pinMode(out_fara_left, OUTPUT);     // Сигнал выходной Фара левая
    pinMode(out_fara_right,OUTPUT);     // Сигнал выходной Фара правая
    pinMode(out_rele1,     OUTPUT);     // Сигнал выходной управления реле 1
    pinMode(out_rele2,     OUTPUT);     // Сигнал выходной управления реле 2

    digitalWrite(out_run_stop,  LOW);   // Сигнал выходной STOP отключить
    digitalWrite(out_fara_size, LOW);   // Сигнал выходной Габариты отключить
    digitalWrite(out_run_rear,  LOW);   // Сигнал выходной Назад отключить 
    digitalWrite(out_fara_left, LOW);   // Сигнал выходной Фара левая отключить
    digitalWrite(out_fara_right,LOW);   // Сигнал выходной Фара правая отключить
    digitalWrite(out_rele1,     LOW);   // Сигнал выходной управления реле 1 отключить
    digitalWrite(out_rele2,     LOW);   // Сигнал выходной управления реле 2 отключить
	
	pinMode(in_test,       INPUT);     // Сигнал входной кнопка тест
	digitalWrite(in_test,   HIGH);     // Сигнал входной кнопка тест подключить резистор
}

void recv_avto()      // Прием и расшифровка кода
{
	if (irrecv.decode(&results)) 
	{
		Serial.println(results.value, HEX);            //"показываем" принятый код

//1	
		if (results.value == 0x2FD807F) 
		{  
			digitalWrite(out_run_stop, HIGH);         // 1 Сигнал выходной STOP on
			stop = true;
			analogWrite(out_fara_size, 255);          // 3 Сигнал выходной Габариты on 
		}   
		else if (results.value == 0x2FD40BF)        // Сигнал выходной STOP off
		{  
			digitalWrite(out_run_stop, LOW);         // 1 Сигнал выходной STOP on
			stop = false;
			if(size)
			{
                analogWrite(out_fara_size, stop_size);          // 3 Сигнал выходной Габариты on 
			}
			else
			{
				analogWrite(out_fara_size, 0);          // Сигнал выходной STOP off
			}

		}   
//2
		else if (results.value == 0x2FDC03F) 
		{  
			stop_size = 60;
			size = true;
			if(!stop)
		    {
		    	analogWrite(out_fara_size, stop_size);          // 3 Сигнал выходной Габариты on 
	        }
		}   
		else if (results.value == 0x2FDA05F) 
		{ 
			size = false;
			if(!stop)
			{
				analogWrite(out_fara_size, 0);              // 5 Сигнал выходной Габариты off
			}
		}   
//3
		if (results.value == 0x2FD609F) 
		{  
			digitalWrite(out_run_rear, HIGH);       // 6 Сигнал выходной Назад on
		}   
		else if (results.value == 0x2FDE01F) 
		{  
			digitalWrite(out_run_rear, LOW);        // 7 Сигнал выходной Назад off
		}   
//4
		else if (results.value == 0x2FD10EF) 
		{  
			digitalWrite(out_fara_left, HIGH);     // 8 Сигнал выходной Фара левая on
		}   
		else if (results.value == 0x2FD906F) 
		{ 
			digitalWrite(out_fara_left, LOW);      // 9 Сигнал выходной Фара левая off
		}  
//5		
		else if (results.value == 0x2FD00FF) 
		{  
			digitalWrite(out_fara_right, HIGH);    // 0 Сигнал выходной Фара правая on
		}   
		else if (results.value == 0x2FD58A7) 
		{  
			digitalWrite(out_fara_right, LOW);     // v+ Сигнал выходной Фара правая off 
		}   
//6
		else if (results.value == 0x2FD7887) 
		{  
			digitalWrite(out_rele1, HIGH);        // v- Сигнал выходной управления реле 1 on
		}   
		else if (results.value == 0x2FDD827) 
		{ 
			digitalWrite(out_rele1, LOW);   // +p Сигнал выходной управления реле 1 off
		}  
//7
		else if (results.value == 0x2FDF807) 
		{  
			digitalWrite(out_rele2, HIGH);  // -p Сигнал выходной управления реле 2 on
		}   
		else if (results.value == 0x2FD28D7) 
		{  
			digitalWrite(out_rele2, LOW);   // AV Сигнал выходной управления реле 2 off
		}   

		irrecv.resume();                   // Receive the next value
	}
	delay(100);
}

void setup()
{
  Serial.begin(115200);
   set_pin();
  irrecv.enableIRIn();             // Start the receiver
    Serial.println("  - - - START - - -   ");
}

void loop()
{

	if(digitalRead(in_test) == false)          // Тестировать приемник
	{
		for(int i = 4; i<11;i++)
		{
			digitalWrite(i, HIGH);              // Сигнал  
			delay(300); 
			digitalWrite(i, LOW);               // Сигнал  
			delay(300);
		}
		for(int i = 0; i<256;i++)
		{
				analogWrite(5, i);              // Сигнал  
	  			delay(20);
		}
		delay(1000);
		analogWrite(5, 0);                    // Сигнал  
	}
	else
	{
		recv_avto();                          // Принять сигнал
		delay(100);
	}

}
