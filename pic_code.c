unsigned int seed;
unsigned short kp;
char code1[10], user1[4];
char msg1[20] = "Initializing...";
char msg2[12] = "Enter Code";
char msg3[15] = "Access Granted";
char msg4[15] = "Access Denied";
char msg5[15] = "Enter New Code";
int i = 0, j, cnt, recVal = 0;
volatile unsigned int elapsedTime = 0;

void interrupt() {
    if (TMR1IF_bit) {
        TMR1IF_bit = 0;
        TMR1H = 0xFC;
        TMR1L = 0x18;
        elapsedTime++;
    }
}

void initializeTimer1() {
    T1CON = 0x01;
    TMR1H = 0xFC;
    TMR1L = 0x18;
    TMR1IE_bit = 1;
    PEIE_bit = 1;
    GIE_bit = 1;
    TMR1ON_bit = 1;
}

void buzzer() {
    PORTA.B2 = 1;
    Delay_ms(200);
    PORTA.B2 = 0;
    Delay_ms(200);
    PORTA.B2 = 1;
    Delay_ms(200);
    PORTA.B2 = 0;
    Delay_ms(200);
    PORTA.B2 = 1;
    Delay_ms(200);
    PORTA.B2 = 0;
    Delay_ms(200);
}

// Keypad module connections
char keypadPort at PORTD;

// LCD module connections
sbit LCD_RS at RB1_bit;
sbit LCD_EN at RB0_bit;
sbit LCD_D4 at RB4_bit;
sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;
sbit LCD_D7 at RB7_bit;
sbit LCD_RS_Direction at TRISB1_bit;
sbit LCD_EN_Direction at TRISB0_bit;
sbit LCD_D4_Direction at TRISE4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;

void code_enter() {
    kp = 0;
    do {
        kp = Keypad_Key_Click();
    } while (!kp);

    switch (kp) {
        case 1: kp = '1'; break;
        case 2: kp = '2'; break;
        case 3: kp = '3'; break;
        case 5: kp = '4'; break;
        case 6: kp = '5'; break;
        case 7: kp = '6'; break;
        case 9: kp = '7'; break;
        case 10: kp = '8'; break;
        case 11: kp = '9'; break;
        case 13: kp = '*'; break;
        case 14: kp = '0'; break;
        case 15: kp = '#'; break;
        default: break;
    }
    code1[i] = kp;
    Lcd_Chr(2, i + 1, kp);
    i++;
}

void delay_20ms() {
    Delay_ms(20);
}

void code_read() {
    delay_20ms();
    user1[0] = EEPROM_Read(0x00);
    delay_20ms();
    user1[1] = EEPROM_Read(0x01);
    delay_20ms();
    user1[2] = EEPROM_Read(0x02);
    delay_20ms();
    user1[3] = EEPROM_Read(0x03);
    delay_20ms();
}

void code_write() {
    delay_20ms();
    EEPROM_Write(0x00, code1[0]);
    delay_20ms();
    EEPROM_Write(0x01, code1[1]);
    delay_20ms();
    EEPROM_Write(0x02, code1[2]);
    delay_20ms();
    EEPROM_Write(0x03, code1[3]);
}

unsigned int pinValue;

void change_code() {
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Out(1, 1, "Enter New Code");
    i = 0;
    code_enter();
    code_enter();
    code_enter();
    code_enter();
    code_write();
    code_read();
}

void main() {
    ADCON1 = 0x07;
    TRISA = 0x00;
    TRISC.F0 = 1; // IR Sensor
    TRISC.F1 = 0; // ESP Com (PIC to ESP)
    TRISC.F2 = 1; // ESP Com (ESP to PIC)
    TRISC.F3 = 1; // ESP Com2 (ESP to PIC)

    PORTA.B0 = 0;
    PORTA.B2 = 0;
    Keypad_Init();
    Lcd_Init();
    code_read();

    if (user1[0] == 0xFF && user1[1] == 0xFF && user1[2] == 0xFF && user1[3] == 0xFF) {
        EEPROM_Write(0x00, '2');
        delay_20ms();
        EEPROM_Write(0x01, '3');
        delay_20ms();
        EEPROM_Write(0x02, '4');
        delay_20ms();
        EEPROM_Write(0x03, '5');
        code_read();
    }

    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Lcd_Out(1, 1, msg1);
    Delay_ms(500);
    Lcd_Cmd(_LCD_CLEAR);

    cnt = 0;
    do {
        Lcd_Cmd(_LCD_CLEAR);
        PORTC.F1 = 0;
        Delay_ms(200);
        if (cnt < 1) {
            i = 0;
            delay_ms(500);
            Lcd_Out(2, 3, "Press *");
            Delay_ms(200);
        } else {
            i = 0;
            delay_ms(500);
            Lcd_Out(1, 4, "Try Again ");
            Lcd_Out(2, 5, "Press *");
            Delay_ms(200);
        }

        i = 0;
        code_enter();
        if (code1[0] == '*') {
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, msg2);
            i = 0;
            code_enter();
            code_enter();
            code_enter();
            code_enter();
            code_enter();

            if (code1[0] == '2' && code1[1] == '3' && code1[2] == '4' && code1[3] == '5' && code1[4] == '*') {
                code_enter();
                if (code1[5] == '#') {
                    change_code();
                    cnt = 0;
                    Lcd_Cmd(_LCD_CLEAR);
                    Lcd_Out(1, 3, "Code Changed");
                    Delay_ms(1500);
                    continue;
                }
            }
        }

        if (cnt < 3 && code1[0] == user1[0] && code1[1] == user1[1] && code1[2] == user1[2] && code1[3] == user1[3] && code1[4] == '#') {
            cnt = 0;
            PORTC.F1 = 1;
            PORTA.B0 = 0;
            recVal = 1;
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, "Please wait");
            Delay_ms(800);
        } else {
            cnt++;
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, msg4);
        }

        while (recVal == 1) {
            if (PORTC.F2 == 1 && PORTC.F3 == 1) {
                PORTC.F1 = 0;
                Lcd_Cmd(_LCD_CLEAR);
                Delay_ms(50]; // Fixed syntax
                Lcd_Out(1, 1, "Granted");
                PORTA.B0 = 1;
                recVal = 0;
                buzzer();
                Delay_ms(100);
                initializeTimer1();
                Delay_ms(1000);
                elapsedTime = 0;

                while (PORTC.F0 == 1 && elapsedTime < 20000) {
                    PORTA.B0 = 1;
                    pinValue = PORTC.F0;
                }
                PORTB.F0 = 0;
                TMR1ON_bit = 0;
                PORTA.B0 = 0;
                Delay_ms(100);
            }
            if (PORTC.F2 == 1 && PORTC.F3 == 0) {
                PORTC.F1 = 0;
                PORTA.B0 = 0;
                Lcd_Cmd(_LCD_CLEAR);
                Delay_ms(2000);
                Lcd_Out(1, 1, "Access Denied");
                Delay_ms(2000);
                recVal = 0;
            }
        }

        if (cnt >= 3) {
            do {
                PORTA.B2 = 1;
            } while (1);
        }
    } while (1);
}