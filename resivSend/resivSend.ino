
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

 */



#include <IRremote.h>
#include <Wire.h>

#define out_run_stop   4          // Сигнал выходной STOP
#define out_run_rear   5          // Сигнал выходной Назад 
#define out_fara_size  6          // Сигнал выходной Габариты
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



IRsend irsend;
// not used
int RECV_PIN = 11;
IRrecv irrecv (RECV_PIN);

const int AC_TYPE  = 0;
// 0 : TOWER
// 1 : WALL
//

int AC_HEAT = 0;
// 0 : cooling
// 1 : heating

int AC_POWER_ON    = 0;
// 0 : off
// 1 : on

int AC_AIR_ACLEAN  = 0;
// 0 : off
// 1 : on --> power on

int AC_TEMPERATURE = 27;
// temperature : 18 ~ 30

int AC_FLOW        = 1;
// 0 : low
// 1 : mid
// 2 : high
// if AC_TYPE =1, 3 : change
//


const int AC_FLOW_TOWER[3] = {0, 4, 6};
const int AC_FLOW_WALL[4]  = {0, 2, 4, 5};

unsigned long AC_CODE_TO_SEND;

int r = LOW;
int o_r = LOW;

byte a, b;


void set_pin()
{
                                       
    pinMode(out_run_stop,  OUTPUT);     // Сигнал выходной STOP
    pinMode(out_run_rear,  OUTPUT);     // Сигнал выходной Назад 
    pinMode(out_fara_size, OUTPUT);     // Сигнал выходной Габариты
    pinMode(out_fara_left, OUTPUT);     // Сигнал выходной Фара левая
    pinMode(out_fara_right,OUTPUT);     // Сигнал выходной Фара правая
    pinMode(out_rele1,     OUTPUT);     // Сигнал выходной управления реле 1
    pinMode(out_rele2,     OUTPUT);     // Сигнал выходной управления реле 2
  //  pinMode(LED_PIN,       OUTPUT);     // Сигнал определения приемник или передатчик

    pinMode(in_run_stop,  INPUT);       // Сигнал входной STOP
    pinMode(in_run_rear,  INPUT);       // Сигнал входной Назад 
    pinMode(in_fara_size, INPUT);       // Сигнал входной Габариты
    pinMode(in_fara_left, INPUT);       // Сигнал входной Фара левая
    pinMode(in_fara_right,INPUT);       // Сигнал входной Фара правая
    pinMode(in_rele1,     INPUT);       // Сигнал входной управления реле 1
    pinMode(in_rele2,     INPUT);       // Сигнал входной управления реле 2

    digitalWrite(out_run_stop,  LOW);   // Сигнал выходной STOP отключить
    digitalWrite(out_run_rear,  LOW);   // Сигнал выходной Назад отключить 
    digitalWrite(out_fara_size, LOW);   // Сигнал выходной Габариты отключить
    digitalWrite(out_fara_left, LOW);   // Сигнал выходной Фара левая отключить
    digitalWrite(out_fara_right,LOW);   // Сигнал выходной Фара правая отключить
    digitalWrite(out_rele1,     LOW);   // Сигнал выходной управления реле 1 отключить
    digitalWrite(out_rele2,     LOW);   // Сигнал выходной управления реле 2 отключить
  //  digitalWrite(LED_PIN,       LOW);   // Сигнал определения приемник или передатчик отключить

    digitalWrite(in_run_stop,  HIGH);   // Сигнал входной STOP подключить резистор
    digitalWrite(in_run_rear,  HIGH);   // Сигнал входной Назад  подключить резистор
    digitalWrite(in_fara_size, HIGH);   // Сигнал входной Габариты подключить резистор
    digitalWrite(in_fara_left, HIGH);   // Сигнал входной Фара левая подключить резистор
    digitalWrite(in_fara_right,HIGH);   // Сигнал входной Фара правая подключить резистор
    digitalWrite(in_rele1,     HIGH);   // Сигнал входной управления реле 1 подключить резистор
    digitalWrite(in_rele2,     HIGH);   // Сигнал входной управления реле 2 подключить резистор
}

 
void ac_send_code(unsigned long code)
{
  Serial.print("code to send : ");
  Serial.print(code, BIN);
  Serial.print(" : ");
  Serial.println(code, HEX);

  irsend.sendLG(code, 28);
}

void ac_activate(int temperature, int air_flow)
{

  int AC_MSBITS1 = 8;
  int AC_MSBITS2 = 8;
  int AC_MSBITS3 = 0;
  int AC_MSBITS4 ;
  if ( AC_HEAT == 1 ) 
  {
    // heating
    AC_MSBITS4 = 4;
  } 
  else 
  {
    // cooling
    AC_MSBITS4 = 0;
  }
  int AC_MSBITS5 = temperature - 15;
  int AC_MSBITS6 ;

  if ( AC_TYPE == 0) {
    AC_MSBITS6 = AC_FLOW_TOWER[air_flow];
  } else {
    AC_MSBITS6 = AC_FLOW_WALL[air_flow];
  }

  int AC_MSBITS7 = (AC_MSBITS3 + AC_MSBITS4 + AC_MSBITS5 + AC_MSBITS6) & B00001111;

  AC_CODE_TO_SEND =  AC_MSBITS1 << 4 ;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS2) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS3) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS4) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS5) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS6) << 4;
  AC_CODE_TO_SEND =  (AC_CODE_TO_SEND + AC_MSBITS7);

  ac_send_code(AC_CODE_TO_SEND);

  AC_POWER_ON = 1;
  AC_TEMPERATURE = temperature;
  AC_FLOW = air_flow;
}

void ac_change_air_swing(int air_swing)
{
  if ( AC_TYPE == 0) {
    if ( air_swing == 1) {
      AC_CODE_TO_SEND = 0x881316B;
    } else {
      AC_CODE_TO_SEND = 0x881317C;
    }
  } else {
    if ( air_swing == 1) {
      AC_CODE_TO_SEND = 0x8813149;
    } else {
      AC_CODE_TO_SEND = 0x881315A;
    }
  }

  ac_send_code(AC_CODE_TO_SEND);
}

void ac_power_down()
{
  AC_CODE_TO_SEND = 0x88C0051;

  ac_send_code(AC_CODE_TO_SEND);

  AC_POWER_ON = 0;
}

void ac_air_clean(int air_clean)
{
  if ( air_clean == 1) {
    AC_CODE_TO_SEND = 0x88C000C;
  } else {
    AC_CODE_TO_SEND = 0x88C0084;
  }

  ac_send_code(AC_CODE_TO_SEND);

  AC_AIR_ACLEAN = air_clean;
}

void setup()
{
  Serial.begin(38400);
  delay(1000);
  Wire.begin(7);
  Wire.onReceive(receiveEvent);

  Serial.println("  - - - T E S T - - -   ");

 //  test
    ac_activate(25, 1);
    delay(5000);
    ac_activate(27, 2);
    delay(5000);

  
}

void loop()
{


  ac_activate(25, 1);
  delay(5000);
  ac_activate(27, 0);
  delay(5000);


  if ( r != o_r) 
  {

    /*
    # a : mode or temp    b : air_flow, temp, swing, clean, cooling/heating
    # 18 ~ 30 : temp      0 ~ 2 : flow // on
    # 0 : off             0
    # 1 : on              0
    # 2 : air_swing       0 or 1
    # 3 : air_clean       0 or 1
    # 4 : air_flow        0 ~ 2 : flow
    # 5 : temp            18 ~ 30
    # + : temp + 1
    # - : temp - 1
    # m : change cooling to air clean, air clean to cooling
    */
    Serial.print("a : ");
    Serial.print(a);
    Serial.print("  b : ");
    Serial.println(b);

    switch (a) {
      case 0: // off
        ac_power_down();
        break;
      case 1: // on
        ac_activate(AC_TEMPERATURE, AC_FLOW);
        break;
      case 2:
        if ( b == 0 | b == 1 ) {
          ac_change_air_swing(b);
        }
        break;
      case 3: // 1  : clean on, power on
        if ( b == 0 | b == 1 ) {
          ac_air_clean(b);
        }
        break;
      case 4:
        if ( 0 <= b && b <= 2  ) {
          ac_activate(AC_TEMPERATURE, b);
        }
        break;
      case 5:
        if (18 <= b && b <= 30  ) {
          ac_activate(b, AC_FLOW);
        }
        break;
      case '+':
        if ( 18 <= AC_TEMPERATURE && AC_TEMPERATURE <= 29 ) {
          ac_activate((AC_TEMPERATURE + 1), AC_FLOW);
        }
        break;
      case '-':
        if ( 19 <= AC_TEMPERATURE && AC_TEMPERATURE <= 30 ) {
          ac_activate((AC_TEMPERATURE - 1), AC_FLOW);
        }
        break;
      case 'm':
        /*
          if ac is on,  1) turn off, 2) turn on ac_air_clean(1)
          if ac is off, 1) turn on,  2) turn off ac_air_clean(0)
        */
        if ( AC_POWER_ON == 1 ) {
          ac_power_down();
          delay(100);
          ac_air_clean(1);
        } else {
          if ( AC_AIR_ACLEAN == 1) {
            ac_air_clean(0);
            delay(100);
          }
          ac_activate(AC_TEMPERATURE, AC_FLOW);
        }
        break;
      default:
        if ( 18 <= a && a <= 30 ) {
          if ( 0 <= b && b <= 2 ) {
            ac_activate(a, b);
          }
        }
    }

    o_r = r ;
  }
  delay(100);
}



void receiveEvent(int howMany)
{
  a = Wire.read();
  b = Wire.read();
  r = !r ;
}

