//////////////////////////////////////////////////////////////////////
////                                                              ////
////                   DIGITAL - THEREMIN                         ////
////                        soc-hw.c                              ////
////                                                              ////
////  Este archivo hace parte del trabajo realizado para el       ////
////  proyecto de curso de Electronica Digital II		  ////
////  Universidad Nacional de Colombia - 2015 -2                  ////
////                                                              ////
////  Autores:                                                    ////
////    *Nelson Leonardo Tovar (nltovar@unal.edu.co)		  ////
////    *Manuel Alejandro Barreto (mabarretor@unal.edu.co)        ////
////    *Daniel Camilo Beltrán (dcbeltrans@unal.edu.co)           ////
////                                                              ////
////  Información adicional en: 				  ////
////  https://sites.google.com/site/edigital2unal/proyectos/      ////
////  	     proyectosii-015/grupo02                              ////
////                                                              ////
//////////////////////////////////////////////////////////////////////


#include "soc-hw.h"

// Direcciones de los perifericos utilizados

uart_t  *uart0  = (uart_t *)   0x20000000;
timer_t *timer0 = (timer_t *)  0x30000000;
gpio_t  *gpio0  = (gpio_t *)   0x40000000;
digpot_t *digpot0 = (digpot_t *)    0x50000000;
trigger_t *trigger0 = (trigger_t *) 0x60000000;

isr_ptr_t isr_table[32];

void tic_isr();
/***************************************************************************
 * IRQ handling
 */
void isr_null()
{
}

void irq_handler(uint32_t pending)
{
	int i;

	for(i=0; i<32; i++) {
		if (pending & 0x01) (*isr_table[i])();
		pending >>= 1;
	}
}

void isr_init()
{
	int i;
	for(i=0; i<32; i++)
		isr_table[i] = &isr_null;
}

void isr_register(int irq, isr_ptr_t isr)
{
	isr_table[irq] = isr;
}

void isr_unregister(int irq)
{
	isr_table[irq] = &isr_null;
}


//*************************************************************************
// Estructura del periferico GPIO
//*************************************************************************

void gpio_init(uint32_t k)
{
	gpio0->dir=k; //todo lo que sea 0 es puerto lectura, 1 es puerto escritura o salida
}

uint32_t gpio_read()
{
	return gpio0->read;
}

void gpio_write(uint32_t k)
{
	gpio0->write=k;
}


void set_pin(uint8_t value, uint8_t npin)
{
     if (value)
		gpio0->write = gpio0->read | npin;
	else
		gpio0->write = gpio0->read & (~npin);

}

void pin_inv(uint8_t npin)
{
	uint32_t val;
	val = (~gpio0->read) & npin;
    set_pin(val,npin);
}




//***************************************************************************
// Estructura del periferico TIMER
//**************************************************************************
int counter()
{
	uint32_t tcr;
	int msec=0xFFFFFFFF;
	timer0->compare1 = (FCPU/1000)*msec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;

	return timer0->counter1;
}


void msleep(uint32_t msec)
{
	uint32_t tcr;

	// Use timer0.1
	timer0->compare1 = (FCPU/1000)*msec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;

	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}

void nsleep(uint32_t nsec)
{
	uint32_t tcr;

	// Use timer0.1
	timer0->compare1 = (FCPU/1000000)*nsec;
	timer0->counter1 = 0;
	timer0->tcr1 = TIMER_EN;

	do {
		//halt();
 		tcr = timer0->tcr1;
 	} while ( ! (tcr & TIMER_TRIG) );
}


uint32_t tic_msec;

void tic_isr()
{
	tic_msec++;
	timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;
}

void tic_init()
{
	tic_msec = 0;

	// Setup timer0.0
	timer0->compare0 = (FCPU/10000);
	timer0->counter0 = 0;
	timer0->tcr0     = TIMER_EN | TIMER_AR | TIMER_IRQEN;

	isr_register(1, &tic_isr);
}


//***************************************************************************
// Estructura del periferico UART
//***************************************************************************
void uart_init(){}

char uart_getchar()
{
	while (! (uart0->ucr & UART_DR)) ;
	return uart0->rxtx;
}

void uart_putchar(char c)
{
	while (uart0->ucr & UART_BUSY) ;
	uart0->rxtx = c;
}

void uart_putstr(char *str)
{
	char *c = str;
	while(*c) {
		uart_putchar(*c);
		c++;
	}
}


//***************************************************************************
// Estructura del periferico TRIGGER
//***************************************************************************
void trigger_set(uint32_t st)
{
    trigger0->set_trig = st;
}

uint32_t read_count()
{
    return trigger0->c_out;
}

//***************************************************************************
// Estructura del periferico DIGPOT
//***************************************************************************

void set_digpot(uint32_t dp)
{
    digpot0->set_digpot = dp;

}

//***************************************************************************
// Estructura del periferico PWM
//***************************************************************************
/*
void asig_pwm ()

{

    if (p == 1)								//selección tercera octava 
    {	
	if (c1 > 2 && c1 < 10){b1 = 170; c3 = 132; c2 = 22452;}		//selección de la nota dependiendo la distancia
        else if (c1 > 10 && c1 < 20){b1 = 170; c3 = 118; c2 = 20008;}
	else if (c1 > 20 && c1 < 30){b1 = 170; c3 = 105; c2 = 17825;}
	else if (c1 > 30 && c1 < 40){b1 = 170; c3 = 99; c2 = 16855;}
	else if (c1 > 40 && c1 < 50){b1 = 170; c3 = 88; c2 = 15006;}
	else if (c1 > 50 && c1 < 60){b1 = 170; c3 = 79; c2 = 13369;}
        else if (c1 > 60 && c1 < 70){b1 = 170; c3 = 70; c2 = 11908;}
    

    	for (i=0; i<63;i++)
	    {
	        b2 = b1+1;

	        for (j=0; j<b2; j++)					//recorre cada una de las muestras (b1 tercera octava)
                {
                    c4 = j*c3;
                    c5 = c2-c4;

    	            set_pin(1,1);					//se asigna el ciclo util por tiempo
   	            nsleep(c4);
	            set_pin(0,1);
	            nsleep(c5);
                }
            }
    }

    else if ( p == 2)							//selección cuarta octava
    {
        if (c1 > 2 && c1 < 8){b1 = 170; c3 = 132; c2 = 22452;}		//selección de la nota dependiendo la distancia
        else if (c1 > 8 && c1 < 14){b1 = 170; c3 = 118; c2 = 20008;}
	else if (c1 > 14 && c1 < 20){b1 = 170; c3 = 105; c2 = 17825;}
	else if (c1 > 20 && c1 < 26){b1 = 170; c3 = 99; c2 = 16855;}
	else if (c1 > 26 && c1 < 32){b1 = 170; c3 = 88; c2 = 15006;}
	else if (c1 > 32 && c1 < 38){b1 = 170; c3 = 79; c2 = 13369;}
        else if (c1 > 38 && c1 < 44){b1 = 170; c3 = 70; c2 = 11908;}
	else if (c1 > 44 && c1 < 50){b1 = 170; c3 = 33; c2 = 5613;}
        else if (c1 > 50 && c1 < 56){b1 = 170; c3 = 29; c2 = 5002;}
	else if (c1 > 56 && c1 < 62){b1 = 170; c3 = 26; c2 = 4456;}
	else if (c1 > 62 && c1 < 68){b1 = 170; c3 = 25; c2 = 4214;}
	else if (c1 > 68 && c1 < 74){b1 = 170; c3 = 22; c2 = 3752;}
	else if (c1 > 74 && c1 < 80){b1 = 170; c3 = 20; c2 = 3342;}
        else if (c1 > 80 && c1 < 86){b1 = 170; c3 = 18; c2 = 2977;}
    

    	for (i=0; i<63;i++)
	    {
	        b2 = b1+1;

	        for (j=0; j<b2; j++)					//recorre cada una de las muestras (b1 tercera octava)
                {
                    c4 = j*c3;
                    c5 = c2-c4;

    	            set_pin(1,1);					//se asigna el ciclo util por tiempo
   	            nsleep(c4);
	            set_pin(0,1);
	            nsleep(c5);
                }
            }
    }
}
*/
