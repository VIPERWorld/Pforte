
/*
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

#define in_run_stop   A0          // Сигнал входной STOP
#define in_run_rear   A1          // Сигнал входной Назад 
#define in_fara_size  A2          // Сигнал входной Габариты
#define in_fara_left  A3          // Сигнал входной Фара левая
#define in_fara_right A4          // Сигнал входной Фара правая
#define in_rele1      A5          // Сигнал входной управления реле 1
#define in_rele2      A6          // Сигнал входной управления реле 2

#define in_test   12              // Сигнал входной кнопка тест 
#define in_send_rec    2          // Сигнал входной определения приемник или передатчик


IRsend irsend;

int RECV_PIN = 11;

IRrecv irrecv (RECV_PIN);

decode_results results;

unsigned long AC_CODE_TO_SEND;

int o_r = 0;
int r_send = 20;

void ac_send_code(unsigned long code)
{
  Serial.print("code to send : ");
  Serial.print(code, BIN);
  Serial.print(" : ");
  Serial.println(code, HEX);
  irsend.sendLG(code, 28);
}

void set_pin()
{
                                       
    pinMode(out_run_stop,  OUTPUT);     // Сигнал выходной STOP
    pinMode(out_run_rear,  OUTPUT);     // Сигнал выходной Назад 
    pinMode(out_fara_size, OUTPUT);     // Сигнал выходной Габариты
    pinMode(out_fara_left, OUTPUT);     // Сигнал выходной Фара левая
    pinMode(out_fara_right,OUTPUT);     // Сигнал выходной Фара правая
    pinMode(out_rele1,     OUTPUT);     // Сигнал выходной управления реле 1
    pinMode(out_rele2,     OUTPUT);     // Сигнал выходной управления реле 2
 
    pinMode(in_run_stop,  INPUT);       // Сигнал входной STOP
    pinMode(in_run_rear,  INPUT);       // Сигнал входной Назад 
    pinMode(in_fara_size, INPUT);       // Сигнал входной Габариты
    pinMode(in_fara_left, INPUT);       // Сигнал входной Фара левая
    pinMode(in_fara_right,INPUT);       // Сигнал входной Фара правая
    pinMode(in_rele1,     INPUT);       // Сигнал входной управления реле 1
    pinMode(in_rele2,     INPUT);       // Сигнал входной управления реле 2
 	pinMode(in_test,      INPUT);       // Сигнал входной кнопка тест
	pinMode(in_send_rec,  INPUT);       // Сигнал входной  определения приемник или передатчик

    digitalWrite(out_run_stop,  LOW);   // Сигнал выходной STOP отключить
    digitalWrite(out_run_rear,  LOW);   // Сигнал выходной Назад отключить 
    digitalWrite(out_fara_size, LOW);   // Сигнал выходной Габариты отключить
    digitalWrite(out_fara_left, LOW);   // Сигнал выходной Фара левая отключить
    digitalWrite(out_fara_right,LOW);   // Сигнал выходной Фара правая отключить
    digitalWrite(out_rele1,     LOW);   // Сигнал выходной управления реле 1 отключить
    digitalWrite(out_rele2,     LOW);   // Сигнал выходной управления реле 2 отключить
 
    digitalWrite(in_run_stop,  HIGH);   // Сигнал входной STOP подключить резистор
    digitalWrite(in_run_rear,  HIGH);   // Сигнал входной Назад  подключить резистор
    digitalWrite(in_fara_size, HIGH);   // Сигнал входной Габариты подключить резистор
    digitalWrite(in_fara_left, HIGH);   // Сигнал входной Фара левая подключить резистор
    digitalWrite(in_fara_right,HIGH);   // Сигнал входной Фара правая подключить резистор
    digitalWrite(in_rele1,     HIGH);   // Сигнал входной управления реле 1 подключить резистор
    digitalWrite(in_rele2,     HIGH);   // Сигнал входной управления реле 2 подключить резистор
	digitalWrite(in_test,      HIGH);   // Сигнал входной кнопка тест подключить резистор
	digitalWrite(in_send_rec,  HIGH);   // Сигнал входной  определения приемник или передатчик
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
		}   
		else if (results.value == 0x2FD40BF) 
		{  
			digitalWrite(out_run_stop, LOW);         // 2 Сигнал выходной STOP off
		}   
//2
		else if (results.value == 0x2FDC03F) 
		{  
			analogWrite(out_fara_size, 60);       // 3 Сигнал выходной Габариты on 50%    
		}   
		else if (results.value == 0x2FD20DF) 
		{ 
			digitalWrite(out_fara_size, HIGH);       // 4 Сигнал выходной Габариты off 100%
		}  
		else if (results.value == 0x2FDA05F) 
		{  
			digitalWrite(out_fara_size, LOW);       // 5 Сигнал выходной Габариты off
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

void send_avto(int r)
{
  if ( r != o_r) 
  {
    switch (r) 
	{
		case 0:           //  
			AC_CODE_TO_SEND = 0x2FD807F; 
		break;
		case 1:           //  
			AC_CODE_TO_SEND = 0x2FD40BF;
		break;
		case 2:          //
			AC_CODE_TO_SEND = 0x2FDC03F;
		break;
		case 3:          // 
			AC_CODE_TO_SEND = 0x2FD20DF;
		break;
		case 4:         //
			AC_CODE_TO_SEND = 0x2FDA05F;
		break;
		case 5:         //
			AC_CODE_TO_SEND = 0x2FD609F;
		break;
		case 6:
			AC_CODE_TO_SEND = 0x2FDE01F;
		break;
		case 7:
			AC_CODE_TO_SEND = 0x2FD10EF;  
		break;
		case 8:
			AC_CODE_TO_SEND = 0x2FD906F; 
		break;
		case 9:
			AC_CODE_TO_SEND = 0x2FD00FF;
		break;
		case 10:
			AC_CODE_TO_SEND = 0x2FD58A7;
		break;
		case 11:
			AC_CODE_TO_SEND = 0x2FD7887;
		break;
		case 12:
			AC_CODE_TO_SEND = 0x2FDD827;
		break;
		case 13:
			AC_CODE_TO_SEND = 0x2FDF807;
		break;
		case 14:
			AC_CODE_TO_SEND = 0x2FD28D7;
		break;
		default:
		break;
    }
	 ac_send_code(AC_CODE_TO_SEND);
    o_r = r ;
  }
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
	if(digitalRead(in_send_rec) == false)          // Определен приемник
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
	else                                          // Определен передатчик                 
	{
        if(digitalRead(in_test) == false)         // Тестировать передатчик
		{
			for(int i=0;i<15;i++)
				{
					send_avto(i);
					delay(1000); 
				}
        }
		else                                  
		{
           if(digitalRead(in_run_stop) == false)     // 1
			   {
				   r_send = 0;
				   send_avto(r_send);
		       }
			   else
		   	   {
				   r_send = 1;
				   send_avto(r_send);
		       }
		  /* 
		   if(digitalRead(in_run_rear) == false)    // 2
			   {
				   r_send = 2;
				   send_avto(r_send);
				  // Serial.println();
		       }
	   		else
		   	   {
				   r_send = 3;
				   send_avto(r_send);
		       }
		   */
			   
          if(digitalRead(in_fara_size) == false)    // 3
			   {
				   r_send = 4;
				   send_avto(r_send);
		       }
		 	   else
		   	   {
				   r_send = 5;
				   send_avto(r_send);
		       }
		 /*
		  if(digitalRead(in_fara_left) == false)    // 4
			   {
				   r_send = 6;
				   send_avto(r_send);
		       }
			   else
		   	   {
				   r_send = 7;
				   send_avto(r_send);
		       }

          if(digitalRead(in_fara_right) == false)    // 5
			   {
				   r_send = 8;
				   send_avto(r_send);
		       }
		 	   else
		   	   {
				   r_send = 9;
				   send_avto(r_send);
		       }
		 if(digitalRead(in_rele1) == false)    // 6
			   {
				   r_send = 10;
				   send_avto(r_send);
		       }
		 	   else
		   	   {
				   r_send = 11;
				   send_avto(r_send);
		       }
         if(digitalRead(in_rele2) == false)    // 7
			   {
				   r_send = 12;
				   send_avto(r_send);
		       }
		 	   else
		   	   {
				   r_send = 0;
				   send_avto(r_send);
		       }

		 */
		 //if(digitalRead( == false))    // 8
			//   {
			//	   r_send = 13;
			//	   send_avto(r_send);
		 //      }
		 //	   else
		 //  	   {
			//	   r_send = 14;
			//	   send_avto(r_send);
		 //      }

		  delay(200);
		}
	}
}
