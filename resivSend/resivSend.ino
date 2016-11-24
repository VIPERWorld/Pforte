
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

int RECV_PIN = 11;                // Сигнал определения приемник или передатчик
int LED_PIN  = 3;                 // Вход/выход фото- или светодиода

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

#define RECEIVER 1
#define SENDER 2
#define ERROR 3

int mode;

void set_pin()
{
                                       
	pinMode(out_run_stop,  OUTPUT);     // Сигнал выходной STOP
	pinMode(out_run_rear,  OUTPUT);     // Сигнал выходной Назад 
	pinMode(out_fara_size, OUTPUT);     // Сигнал выходной Габариты
	pinMode(out_fara_left, OUTPUT);     // Сигнал выходной Фара левая
	pinMode(out_fara_right,OUTPUT);     // Сигнал выходной Фара правая
	pinMode(out_rele1,     OUTPUT);     // Сигнал выходной управления реле 1
	pinMode(out_rele2,     OUTPUT);     // Сигнал выходной управления реле 2
	pinMode(LED_PIN,       OUTPUT);     // Сигнал определения приемник или передатчик

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
	digitalWrite(LED_PIN,       LOW);   // Сигнал определения приемник или передатчик отключить

	digitalWrite(in_run_stop,  HIGH);   // Сигнал входной STOP подключить резистор
	digitalWrite(in_run_rear,  HIGH);   // Сигнал входной Назад  подключить резистор
	digitalWrite(in_fara_size, HIGH);   // Сигнал входной Габариты подключить резистор
	digitalWrite(in_fara_left, HIGH);   // Сигнал входной Фара левая подключить резистор
	digitalWrite(in_fara_right,HIGH);   // Сигнал входной Фара правая подключить резистор
	digitalWrite(in_rele1,     HIGH);   // Сигнал входной управления реле 1 подключить резистор
	digitalWrite(in_rele2,     HIGH);   // Сигнал входной управления реле 2 подключить резистор
}


void setup()
{
  Serial.begin(9600);
  set_pin();
 
  if (digitalRead(RECV_PIN) == HIGH) // Проверка RECV_PIN, чтобы решить, если мы ПРИЕМНИК или ПЕРЕДАТЧИК
  {
    mode = RECEIVER;
    irrecv.enableIRIn();
    Serial.println("Receiver mode");
  } 
  else 
  {
    mode = SENDER;
    Serial.println("Sender mode");
  }
}

// Подождите, пока разрыв между тестами, для синхронизации с
// Отправитель.
// В частности, ждать сигнала с последующим разрывом в последние щелевых мс.
void waitForGap(int gap) 
{
  Serial.println("Waiting for gap");
  while (1) 
  {
    while (digitalRead(RECV_PIN) == LOW) {}
    unsigned long time = millis();
    while (digitalRead(RECV_PIN) == HIGH) 
	{
      if (millis() - time > gap) 
	  {
        return;
      }
    }
  }
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
void dump(decode_results *results) 
{
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) 
  {
    Serial.println("Could not decode message");
  } 
  else 
  {
    if (results->decode_type == NEC) 
	{
      Serial.print("Decoded NEC: ");
    } 
    else if (results->decode_type == SONY) 
	{
      Serial.print("Decoded SONY: ");
    } 
    else if (results->decode_type == RC5) 
	{
      Serial.print("Decoded RC5: ");
    } 
    else if (results->decode_type == RC6) 
	{
      Serial.print("Decoded RC6: ");
    }
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
  }
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}


// Проверка отправки или приема.
// Если режим ОТПРАВИТЕЛЬ, отправьте код указанного типа, значение и биты
// Если режим RECEIVER, получить код и убедитесь, что он имеет
// указанного типа, значение и биты. Для успеха, светодиод мигает;
// Для отказа, режим устанавливается на ERROR.
// Мотивацией этого метода заключается в том, что отправитель и получатель
// Может делать то же вызовы тестирования, а переменная режима указывает, является ли
// Для отправки или получения.
void test(char *label, int type, unsigned long value, int bits) 
{
  if (mode == SENDER) 
  {
    Serial.println(label);
    if (type == NEC) {
      irsend.sendNEC(value, bits);
    } 
    else if (type == SONY) 
	{
      irsend.sendSony(value, bits);
    } 
    else if (type == RC5) 
	{
      irsend.sendRC5(value, bits);
    } 
    else if (type == RC6) 
	{
      irsend.sendRC6(value, bits);
    } 
    else 
	{
      Serial.print(label);
      Serial.println("Bad type!");
    }
    delay(200);
  } 
  else if (mode == RECEIVER) 
  {
    irrecv.resume(); // Receive the next value
    unsigned long max_time = millis() + 30000;
    Serial.print(label);

    // Wait for decode or timeout
    while (!irrecv.decode(&results)) 
	{
      if (millis() > max_time) 
	  {
        Serial.println("Timeout receiving data");
        mode = ERROR;
        return;
      }
    }
    if (type == results.decode_type && value == results.value && bits == results.bits) 
	{
      Serial.println (": OK");
      digitalWrite(LED_PIN, HIGH);
      delay(20);
      digitalWrite(LED_PIN, LOW);
    } 
    else 
	{
      Serial.println(": BAD");
      dump(&results);
      mode = ERROR;
    }
  }
}

// Проверка сырой отправить или получить. Это аналогично методу испытаний,
// За исключением того, отправка / получает исходные данные.
void testRaw(char *label, unsigned int *rawbuf, int rawlen) 
{
  if (mode == SENDER) 
  {
    Serial.println(label);
    irsend.sendRaw(rawbuf, rawlen, 38 /* kHz */);
    delay(200);
  } 
  else if (mode == RECEIVER ) 
  {
    irrecv.resume(); // Receive the next value
    unsigned long max_time = millis() + 30000;
    Serial.print(label);
    // Wait for decode or timeout
    while (!irrecv.decode(&results)) 
	{
      if (millis() > max_time)
	  {
        Serial.println("Timeout receiving data");
        mode = ERROR;
        return;
      }
    }

    // Received length has extra first element for gap
    if (rawlen != results.rawlen - 1) 
	{
      Serial.print("Bad raw length ");
      Serial.println(results.rawlen, DEC);
      mode = ERROR;
      return;
    }
    for (int i = 0; i < rawlen; i++) 
	{
      long got = results.rawbuf[i+1] * USECPERTICK;
      // Adjust for extra duration of marks
      if (i % 2 == 0) { 
        got -= MARK_EXCESS;
      } 
      else {
        got += MARK_EXCESS;
      }
      // See if close enough, within 25%
      if (rawbuf[i] * 1.25 < got || got * 1.25 < rawbuf[i]) 
	  {
        Serial.println(": BAD");
        dump(&results);
        mode = ERROR;
        return;
      }

    }
    Serial.println (": OK");
    digitalWrite(LED_PIN, HIGH);
    delay(20);
    digitalWrite(LED_PIN, LOW);
  }
}   

// This is the raw data corresponding to NEC 0x12345678
unsigned int sendbuf[] = { /* NEC format */
  9000, 4500,
  560, 560, 560, 560, 560, 560, 560, 1690, /* 1 */
  560, 560, 560, 560, 560, 1690, 560, 560, /* 2 */
  560, 560, 560, 560, 560, 1690, 560, 1690, /* 3 */
  560, 560, 560, 1690, 560, 560, 560, 560, /* 4 */
  560, 560, 560, 1690, 560, 560, 560, 1690, /* 5 */
  560, 560, 560, 1690, 560, 1690, 560, 560, /* 6 */
  560, 560, 560, 1690, 560, 1690, 560, 1690, /* 7 */
  560, 1690, 560, 560, 560, 560, 560, 560, /* 8 */
  560};

void loop() 
{
  if (mode == SENDER) 
  {
    delay(2000);  // Delay for more than gap to give receiver a better chance to sync.
  } 
  else if (mode == RECEIVER) 
  {
    waitForGap(1000);
  } 
  else if (mode == ERROR)
  {
    // Light up for 5 seconds for error
    digitalWrite(LED_PIN, HIGH);
    delay(5000);
    digitalWrite(LED_PIN, LOW);
    mode = RECEIVER;  // Try again
    return;
  }

  // The test suite.
  test("SONY1", SONY, 0x123, 12);
  test("SONY2", SONY, 0x000, 12);
  test("SONY3", SONY, 0xfff, 12);
  test("SONY4", SONY, 0x12345, 20);
  test("SONY5", SONY, 0x00000, 20);
  test("SONY6", SONY, 0xfffff, 20);
  test("NEC1", NEC, 0x12345678, 32);
  test("NEC2", NEC, 0x00000000, 32);
  test("NEC3", NEC, 0xffffffff, 32);
  test("NEC4", NEC, REPEAT, 32);
  test("RC51", RC5, 0x12345678, 32);
  test("RC52", RC5, 0x0, 32);
  test("RC53", RC5, 0xffffffff, 32);
  test("RC61", RC6, 0x12345678, 32);
  test("RC62", RC6, 0x0, 32);
  test("RC63", RC6, 0xffffffff, 32);

  // Tests of raw sending and receiving.
  // First test sending raw and receiving raw.
  // Then test sending raw and receiving decoded NEC
  // Then test sending NEC and receiving raw
  testRaw("RAW1", sendbuf, 67);
  if (mode == SENDER) 
  {
    testRaw("RAW2", sendbuf, 67);
    test("RAW3", NEC, 0x12345678, 32);
  } 
  else 
  {
    test("RAW2", NEC, 0x12345678, 32);
    testRaw("RAW3", sendbuf, 67);
  }
}