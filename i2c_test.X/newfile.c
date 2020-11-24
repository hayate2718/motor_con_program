
// PIC16F1827 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable (Brown-out Reset disabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = OFF       // Internal/External Switchover (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = OFF     // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#define _XTAL_FREQ 8000000
void I2C_Master_Wait();
unsigned long pwm_freq = 35000; //7900～100000Hz

void picinit(){
    OSCCON = 0b01110010;
    ANSELA = 0b00000100;
    ANSELB = 0;
    TRISA = 0b00000111;
    TRISB = 0b00010010;
    SSP1STAT = 0b00000000;
    SSP1CON1 = 0b00101000;
    SSP1CON3 = 0b00000000;
    SSP1ADD = (_XTAL_FREQ/(4*100000))-1;
    ADCON0 = 0b00001001;
    ADCON1 = 0b11000000;
    CCP3CON = 0b00001100;
    CCPTMRS =0;
    T2CON = 0b00000100;
    PR2 = 2000000/pwm_freq-1;
    CCPR3L = 0;
}

int i2c_start(){
    SSP1IF = 0;
    SSPCON2bits.SEN = 1;
    while(SSP1IF == 0);
    SSP1IF = 0;
    return 0;
}

int i2c_stop(){
    SSP1IF = 0;
    SSPCON2bits.PEN = 1;
    while(SSP1IF == 0);
    SSP1IF = 0;
    return 0;
}

int i2c_send(char data){
    SSP1IF = 0;
    SSP1BUF = data;
    while(SSP1IF == 0);
    SSP1IF = 0;
    return 0;
}

#define I2C_ACK 0x00
#define I2C_NACK 0xff

char i2c_ack(){
    char ackstat = 0;
    if(SSP1CON2bits.ACKSTAT){
        ackstat = I2C_NACK;
    }else{
        ackstat = I2C_ACK;
    }
    return ackstat;
}

#define lcd_adres 0x7c

int lcd_i2c(char con,char data){
    i2c_start();
    i2c_send(lcd_adres);
    i2c_send(con);
    i2c_send(data);
    i2c_stop();
    return 0;
}

int lcd_send_com(char command){
    lcd_i2c(0x00,command);
    __delay_ms(1);
    return 0;
}

int lcd_send_char(char data){
    lcd_i2c(0x40,data);
    __delay_ms(1);
    return 0;
}

void lcd_init(){
    lcd_send_com(0x38);
    lcd_send_com(0x39);
    lcd_send_com(0x14);
    lcd_send_com(0x73);
    lcd_send_com(0x52);
    lcd_send_com(0x6c);
    __delay_ms(200);
    lcd_send_com(0x38);
    lcd_send_com(0x01);
    lcd_send_com(0x0c);
    return;
}

int lcd_pos(char pos_x,char pos_y){
    lcd_send_com(0x80 + 0x40 * pos_y + pos_x);
    return 0;
}

void putch(char character){
    lcd_send_char(character);
    return;
}

int adc_h(){
    unsigned int adc = 0;
    GO = 1;
    while(GO);
    adc = ADRES;
    return adc;
}

int pwm_h(unsigned long volt){
    unsigned int pwm = 0;
    unsigned int poripori = 0;
    poripori = volt * (PR2+1);
    pwm = poripori / 1024;
    return pwm;
}

void pwm_lcd(unsigned int duty){
    printf("PWM_Duty:%d ",duty);
}

void dir_o(){
    if(RA1 == 1){
        printf("Dir:F");
        RA4 =1;
    }else{
        printf("Dir:B");
        RA4 = 0;
    }
}

void reset_o(){
    if(RA0 == 1){
        printf("Reset:L ");
        RB0 = 0;
    } else{
        printf("Reset:H ");
        RB0 = 1;
    }
}

int main(){
    picinit();
    __delay_ms(100);
    lcd_init();
    lcd_pos(0,0);
    while(1){
        CCPR3L = pwm_h(adc_h());
        lcd_pos(0,1);
        pwm_lcd(100*CCPR3L/PR2);
        lcd_pos(0,0);
        dir_o();
        lcd_pos(7,0);
        reset_o();
    }
    return 0;
}

