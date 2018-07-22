/*
 * Cassio Trindade Batista
 * https://www.itead.cc/wiki/Serial_Port_Bluetooth_Module_(Master/Slave)_:_HC-05#2._Reset

*/

#include <SoftwareSerial.h>
#include <IRremote.h>

/* Bluetooth pins */
#define HC05_CONFIG_PIN 9
#define HC05_RX_PIN 2 //digital pin to receiver
#define HC05_TX_PIN 5 //digital pin to transceiver

#define SAMSUNG_CANAL_MAIS       0xE0E048B7
#define SAMSUNG_CANAL_MENOS      0xE0E008F7
#define SAMSUNG_AUMENTAR_VOLUME  0xE0E0E01F
#define SAMSUNG_DIMINUIR_VOLUME  0xE0E0D02F
#define SAMSUNG_ON_OFF           0xE0E040BF

#define DEGUB 1

IRsend irsend;
SoftwareSerial serialBT(HC05_RX_PIN, HC05_TX_PIN); //BLUETOOTH: define pins to Rx and Tx

void setup()
{
	/* Define Rx as input and Tx as output */
	pinMode(HC05_RX_PIN, INPUT);
	pinMode(HC05_TX_PIN, OUTPUT);

	serialBT.begin(38400); //bauds to bluetooth
	Serial.begin(38400);

	//config_HC05();
}  

void loop()
{
	int i = 0; //index to str
	char str[9] = {0}; 
	char ch;

	#if DEGUB
		Serial.println("waiting for connection...");
	#endif

	if(serialBT.available()) {
		#if DEGUB
			Serial.print("\tConnection Found!!! -> ");
		#endif
		do {
			ch = (char) serialBT.read(); //get each char and fill string
			if (ch != '\n' && ch != '\r')
				str[i++] = ch;
			delay(2); 
		} while (serialBT.available() > 0); //while has char in SoftSerial

		#if DEGUB
			Serial.print("##");
			Serial.print(str);
			Serial.print("##");
			Serial.print(" ");
			Serial.print(i);
			Serial.print("\t ->");
		#endif

		/* YAW */
		if(str[0] == 'Y') {
			if (str[1] == 'L') {
				#if DEGUB
					Serial.println("canal menos!");
				#endif
				sendIR(SAMSUNG_CANAL_MENOS, 32); //yaw left -> canal menos
			} else if (str[1] == 'R') {
				#if DEGUB
					Serial.println("canal mais!");
				#endif
				sendIR(SAMSUNG_CANAL_MAIS, 32); //yaw right -> canal mais
			} else {
				#if DEGUB
					Serial.println("YAW ERROR");
				#endif
			}
		/* PITCH */
		} else if (str[0] == 'P') {
			if (str[1] == 'U') {
				#if DEGUB
					Serial.println("aumentar volume!");
				#endif
				for(i=0; i<6; i++) {
					sendIR(SAMSUNG_AUMENTAR_VOLUME, 32); //pitch up -> aumentar volume
					delay(200);
				}
			} else if (str[1] == 'D') {
				#if DEGUB
					Serial.println("diminuir volume!");
				#endif
				for(i=0; i<6; i++) {
					sendIR(SAMSUNG_DIMINUIR_VOLUME, 32); //pitch down -> diminuir volume
					delay(200);
				}
			} else {
				#if DEGUB
					Serial.println("PITCH ERROR");
				#endif
			}
		/* ROLL */
		} else if (str[0] == 'R') {
				#if DEGUB
					Serial.println("ligar/desligar!");
				#endif
			sendIR(SAMSUNG_ON_OFF, 32); //roll -> on/off
		} else {
			#if DEGUB
				Serial.println("MOVE ERROR");
			#endif
		}
	}//close if serial bluetooth available
	delay(200);
}

void sendIR(unsigned long hex, int nbits)
{
	for (int i=0; i<3; i++) {
		irsend.sendSAMSUNG(hex, nbits);
		delay(40);
	}
}

void config_HC05()
{
	#if DEGUB
		Serial.println("Configuring...");
	#endif

	pinMode(HC05_CONFIG_PIN, OUTPUT);
	digitalWrite(HC05_CONFIG_PIN, HIGH);

	/* Restore default status */
	serialBT.write("AT+ORGL\r\n");
	delay(300);

	/* Reset */
	serialBT.write("AT+RESET\r\n");
	delay(300);

	/* Delete all auth devices */
	serialBT.write("AT+RMAAD\r\n");
	delay(300);

	/* 0: slave */
	serialBT.write("AT+ROLE=0\r\n");
	delay(300);

	/* Connect any address */
	serialBT.write("AT+CMODE=1\r\n");
	delay(300);

	/* 0: PIO8 low drive LED (?) */
	/* 1: PIO9 high drive LED (?) */
	serialBT.write("AT+POLAR=0,1\r\n");
	delay(300);

	/* Reset */
	serialBT.write("AT+RESET\r\n");
	delay(300);

	/* Set PIN code */
	serialBT.write("AT+PSWD=1234\r\n");
	delay(300);

	/* Set default fixed address */
	serialBT.write("AT+BIND=0000,00,000000\r\n");
	delay(300);

	/* Initialize SPP (Serial Port Profile) lib */
	serialBT.write("AT+INIT\r\n");
	delay(300);

	/* Inquiry Bluetooth device (?) */
	serialBT.write("AT+INQ\r\n");
	delay(300);

	/* Set module name */
	serialBT.write("AT+NAME=UFPA Remote Control\r\n");
	delay(300);

	/* Set device class */
	serialBT.write("AT+CLASS=1\r\n");
	delay(300);

	digitalWrite(HC05_CONFIG_PIN, LOW);

	#if DEGUB
		Serial.println("Done! :)");
	#endif
}

//void config_HC05_manually()
//{
//	int i = 0; //iterator
//	char str[30] = {0}; 
//	char ch;
//
//	Serial.println("Digite o comando AT (sem CR e LF):");
//
//	if(Serial.available() > 0) {
//		while(Serial.available()) {
//			ch = (char) Serial.read();
//			if(ch != '\0') {
//				str[i++] = ch; // le char por char
//			} else {
//				str[i++] = '\r';
//				str[i++] = '\n';
//				str[i] = ch;
//				break;
//			}
//		}
//		serialBT.write(str);
//		Serial.println(str);
//	}
//
//	delay(500);
//}
