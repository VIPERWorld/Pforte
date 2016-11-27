
/*
���������� �������� ��������� ��� ��������� ������ ������� ����������������� ���������.
����������� ������ ������� � ��� ������ ���������, 7 ��� ��������� (+ � -).
���������� ����� � ������ ���� ��������� ��������� ��������/��������� � ���� �������� �� ����������.
����� �� ���������� �������� ��� �� ������ �����.
�� ������ �����, �������� ������ � �� ��������� � ������� �� 7 ������� ��� ����������� ����������� � ���� ��������, �� � ����� �� ����������� ��� �� �������� �����.
��� �������� ��� ������������� ��� � ����������� ������� ����� �������, ��� � ����� ����� �� ��������� ������� ������������ �����, �� �� ������.
����� ��� ���������:
��� ������ ���������� ����� ���� ������� (������, ��������, ������ ���, � 2 �����������), ��������� ��� ������ ���������� ��� ���������
������� ������� ��� ��� 1-2�. (��� ������� � ��������� ���� �������).
�� �������� ������, ��� �������, ���� ���������� � ����������� �����, 5 ����� �������. �������������� �� ����� ����� ������, ����� ������, � �����. 
� ��������� 2 ������ �� ��������� �������� (��������� ��� ����� ����������).
������������� �� ������������� �� �����������. �� ���������� ��������������� � ����� ����� � ���������, � �� �������� � ����� �� �������. 
����� ������� ����� ������ �������� ���� ��������, ���� �������� �� ��������.
�� ������ �����������, ���� �������� 5 ������� ��� ���� (������, ��������, ������ ���, � 2 �����������) � ��� ��� ����.
��� ���������� ������� � ���� ����� ��� ���������� �������� ���� ULN2003 ��� ULN2803.
������ �������� ����� ��������� ����������� ���� NPN.

 ���� �������� / ������ ������� IRremote, ��������� ���� Arduinos.
 *
 * ������� # 1 ������ ����� ������������ ��, ������������ � ������ ������� (3).
 * Arduino # 2 ������ ����� ��-�������� / �����������, ������������� � �������
 * �������� ����� (11) � ������� ���������, ������������ � �������� 3.
 *
 * ����:
 * Arduino # 1 ����� ����� 2 �������, � ����� ��������� ����� ���������.
 * ��� ��������� ��� ��������.
 * Arduino # 2 ����� ����� � ������� �� ������� ���� ���� ������� ���������� �������
 * (��� ������������� � # 1). �� ����� ������� ���� �� �����
 * �������. �� ����� �������������� ��� ��������� � ����������������� �����. ��� �����
 * ����� ������� ������ ����� ���������, ������� ����� ������ ������ ���, ����� ����
 * ��������. ���� ���� ������, �� ��� ����� ������ � ������� 5 ������.
 *
 * ���� ��������, ���� ��������� ������ 19 ���, ������ �����, � ����� �����������.
 * ���� ������ �������, ���� ��������� ����� � ������� 5 ������.
 *
 * ���� ��������� ������������� ����������, ����� ����� �������� ������������ � �������
 * ��������, ��� ���� �� ���������� �������, ������� ����� ���������
 * �����������. �� ������ ���������� ���������������� ���� � ��������� ��� �������.

 */



#include <IRremote.h>
#include <Wire.h>

#define out_run_stop   4          // ������ �������� STOP
#define out_run_rear   5          // ������ �������� ����� 
#define out_fara_size  6          // ������ �������� ��������
#define out_fara_left  7          // ������ �������� ���� �����
#define out_fara_right 8          // ������ �������� ���� ������
#define out_rele1      9          // ������ �������� ���������� ���� 1
#define out_rele2     10          // ������ �������� ���������� ���� 2

#define in_run_stop   A0          // ������ ������� STOP
#define in_run_rear   A1          // ������ ������� ����� 
#define in_fara_size  A2          // ������ ������� ��������
#define in_fara_left  A3          // ������ ������� ���� �����
#define in_fara_right A4          // ������ ������� ���� ������
#define in_rele1      A5          // ������ ������� ���������� ���� 1
#define in_rele2      A6          // ������ ������� ���������� ���� 2
#define in_test       12          // ������ ������� ������ ����



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
                                       
    pinMode(out_run_stop,  OUTPUT);     // ������ �������� STOP
    pinMode(out_run_rear,  OUTPUT);     // ������ �������� ����� 
    pinMode(out_fara_size, OUTPUT);     // ������ �������� ��������
    pinMode(out_fara_left, OUTPUT);     // ������ �������� ���� �����
    pinMode(out_fara_right,OUTPUT);     // ������ �������� ���� ������
    pinMode(out_rele1,     OUTPUT);     // ������ �������� ���������� ���� 1
    pinMode(out_rele2,     OUTPUT);     // ������ �������� ���������� ���� 2
  //  pinMode(LED_PIN,       OUTPUT);     // ������ ����������� �������� ��� ����������

    pinMode(in_run_stop,  INPUT);       // ������ ������� STOP
    pinMode(in_run_rear,  INPUT);       // ������ ������� ����� 
    pinMode(in_fara_size, INPUT);       // ������ ������� ��������
    pinMode(in_fara_left, INPUT);       // ������ ������� ���� �����
    pinMode(in_fara_right,INPUT);       // ������ ������� ���� ������
    pinMode(in_rele1,     INPUT);       // ������ ������� ���������� ���� 1
    pinMode(in_rele2,     INPUT);       // ������ ������� ���������� ���� 2
 	pinMode(in_test,      INPUT);       // ������ ������� ������ ����



    digitalWrite(out_run_stop,  LOW);   // ������ �������� STOP ���������
    digitalWrite(out_run_rear,  LOW);   // ������ �������� ����� ��������� 
    digitalWrite(out_fara_size, LOW);   // ������ �������� �������� ���������
    digitalWrite(out_fara_left, LOW);   // ������ �������� ���� ����� ���������
    digitalWrite(out_fara_right,LOW);   // ������ �������� ���� ������ ���������
    digitalWrite(out_rele1,     LOW);   // ������ �������� ���������� ���� 1 ���������
    digitalWrite(out_rele2,     LOW);   // ������ �������� ���������� ���� 2 ���������
  //  digitalWrite(LED_PIN,       LOW);   // ������ ����������� �������� ��� ���������� ���������

    digitalWrite(in_run_stop,  HIGH);   // ������ ������� STOP ���������� ��������
    digitalWrite(in_run_rear,  HIGH);   // ������ ������� �����  ���������� ��������
    digitalWrite(in_fara_size, HIGH);   // ������ ������� �������� ���������� ��������
    digitalWrite(in_fara_left, HIGH);   // ������ ������� ���� ����� ���������� ��������
    digitalWrite(in_fara_right,HIGH);   // ������ ������� ���� ������ ���������� ��������
    digitalWrite(in_rele1,     HIGH);   // ������ ������� ���������� ���� 1 ���������� ��������
    digitalWrite(in_rele2,     HIGH);   // ������ ������� ���������� ���� 2 ���������� ��������
	digitalWrite(in_test,      HIGH);   // ������ ������� ������ ���� ���������� ��������
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
  set_pin();
//  delay(1000);
  Wire.begin(7);
  Wire.onReceive(receiveEvent);

  Serial.println("  - - - T E S T - - -   ");

 //  test
 /*   ac_activate(25, 1);
    delay(5000);
    ac_activate(27, 2);
    delay(5000);*/

  
}

void loop()
{

	if(digitalRead(in_test) == false)
	{
		for(int i = 4; i<11;i++)
		{

		  digitalWrite(i, HIGH);   // ������  
		  delay(300); 
		  digitalWrite(i, LOW);    // ������  
		  delay(300);
		}

		for(int i = 0; i<256;i++)
		{

			  analogWrite(5, i);   // ������  
	  		  delay(20);

		}
		delay(1000);
		analogWrite(5, 0);         // ������  
	}
	else
	{
		  ac_activate(25, 1);
		  delay(100);
		  ac_activate(27, 0);
		  delay(100);


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

			switch (a) 
			{
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
}



void receiveEvent(int howMany)
{
  a = Wire.read();
  b = Wire.read();
  r = !r ;
}
