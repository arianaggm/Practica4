#include <xc.h>
#include "config.h"
#include "LCD_PORTD.h"

#define PWM_FREQ 2500
#define PWM1_out TRISCbits.RC2
#define PWM2_out TRISCbits.RC1
#define PWM3_out TRISCbits.RC0
#define PWM3_pin LATCbits.LC0

#define IN 1
#define OUT 0

void configADC (char CHANNELS);
unsigned int analogRead (char canal);
void configPWM1 (void);
void configPWM2 (void);
void configPWM3 (void);
void setDC1 (char x);
void setDC2 (char x);
void setDC3 (char x);

char duty3;
char HiLo;
double multiplier = _XTAL_FREQ/(PWM_FREQ*16.0*100.0);


void main(void) {
    OSCCON = 127; //0b01111111;
    iniLCD();
    LCDcommand(DispOn);
    LCDprint(9,"LECTURA:  ",100);
    configADC(3);
    unsigned int lec1;
    unsigned int lec2;
    unsigned int lec3;
    unsigned int percent1;
    unsigned int percent2;
    unsigned int percent3;
    configPWM1();
    configPWM2();
    configPWM3();
    
    while (1){
        MoveCursor(0,LINE_DOWN);
        lec1 = analogRead(0);
        lec2 = analogRead(1);
        lec3 = analogRead(2);
        LCDint(lec1);
        LCDchar(' ');
        LCDint(lec2);
        LCDchar(' ');
        LCDint(lec3);
        LCDprint(9,"        ",0);
        __delay_ms(400);
        
        MoveCursor(0,LINE_DOWN); 
        percent1= 100.0*lec1/1023.0;
        percent2= 100.0*lec1/1023.0;
        percent3= 100.0*lec1/1023.0;
        LCDint(percent1);
        LCDchar('%');
        LCDchar(' ');
        LCDint(percent2);
        LCDchar('%');
        LCDchar(' ');
        LCDint(percent3);
        LCDchar('%');
        LCDchar(' ');
        LCDprint(9,"        ",0);
        setDC1(percent1);
        setDC2(percent2);
        __delay_ms(400);
    }
    return;
}

unsigned int analogRead(char canal){
     ADCON0 = 1+canal*4;
     ADCON0bits.GO_DONE = 1; //start conversion
     while(ADCON0bits.GO_DONE == 1) {
          //wait for conversion to finish
      }
     char lowbits = ADRESL; //read results
     char highbits = ADRESH;
     unsigned int adresult = highbits*256+ lowbits;
     return adresult;
}

void configADC(char CHANNELS){
     char allchannelsoff = 0x0F;
     ADCON1 = (allchannelsoff-CHANNELS);
     ADCON2 = 0x80; //Right justified
     ADCON0bits.ADON = 1; //Turn on AD Port;
}


void configPWM1(){
    PWM1_out = OUT;
    PR2 =_XTAL_FREQ/(4.0*PWM_FREQ*16.0)-1;
    setDC1(50);
    T2CON = 0b00000111;
    CCP1CON = 0b00001100;
    return;
}

void configPWM2(){
    PWM2_out = OUT;
    setDC2(50);
    CCP2CON = 0b00001100;
    return;
}

void configPWM3 (){
    PWM3_out = OUT;
    setDC3(50);
    INTCONbits.TMR0IF = 0;
    INTCONbits.T0IE = 1;
    INTCONbits.GIE = 1;
    T0CON = 0b00001000;
    CCP2CON = 0b00001100; //Precarga = 65535 - DC*400; //(50Hz)
    HiLo = 0;
    PWM3_pin = HiLo; //genera la salida en el pin C0
    TMR0 = 45535; //DC 50%
    T0CONbits.TMR0ON = 1;
    return;
}

void setDC1(char x){
    if (x>100){
        x =100;
    }
    int val = x*multiplier;
    CCPR1L = (val - val%4)/4;
    CCP1CONbits.DC1B = val%4;
}

void setDC2(char x){
    if (x>100){
        x =100;
    }
    int val = x*multiplier;
    CCPR2L = (val - val%4)/4;
    CCP2CONbits.DC2B = val%4;
}

void setDC3(char x){
    if (x>100){
        x =100;
    }
    duty3 = x; //sirve para las interrupciones
}

void __interrupt () miISR (){
    if (INTCONbits.T0IF == 1){
        unsigned int preLoad;
        PWM3_pin = HiLo;
        if (HiLo ==1 ){
            preLoad = 65535 - (duty3)*400;
            HiLo = 0;
        }
        else{
            preLoad = 65535 - (100 - duty3)*400;
            HiLo = 1;
        }
        TMR0 = preLoad;
        INTCONbits.T0IF = 0; //baja la bandera del timer0
    }
}
