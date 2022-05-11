#include <avr/io.h>
#include <avr/interrupt.h>


// ENUM DE LOS ESTADOS DE LA MAQUINA DE ESTADOS
typedef enum{
	     GREEN_RED,
	     YELLOW_RED,
	     RED_RED_A,
	     RED_GREEN,
	     RED_YELLOW,
	     RED_RED_B } State;

// VALORES DE CONTEO PARA LOS TIEMPOS DE PERMANENCIA EN LOS ESTADOS
// ESTOS SE CONSIGUIERON CON PRUEBA Y ERROR.
#define GREEN_RED_MAX 39
#define YELLOW_RED_MAX 50
#define RED_RED_A_MAX 54
#define RED_GREEN_MAX 93
#define RED_YELLOW_MAX 104
#define RED_RED_B_MAX 108
#define COUNT_MAX 150

// VARIABLE QUE MANEJARA PARTE DEL FUNCIONAMIENTO DEL CIRCUITO, ESTA
// SERA ACTIVADA POR EL PULSADOR, MEDIANTE UNA INTERRUPCION.
// CUANDO ESTA VARIABLE ES CERO SIGNIFICA QUE NO SE HA PRESIONADO
// EL PULSADOR Y POR LO TANTO NO HAY PASO A LOS PEATONES Y LOS CARROS
// TIENEN LUZ VERDE PERMANENTE. CUANDO EL VALOR ES 1, ENTONCES BAJO
// CIERTAS CONDICIONES SE ACTIVARA EL PASO PEATONAL EN EL MOMENTO
// INDICADO.
volatile int flag = 0;

// COMPORTAMIENTO DE LA INTERRUPCION, ACTIVA LA VARIABLE QUE HACE
// QUE LA MAQUINA DE ESTADOS PERMITA EL PASO PEATONAL EN EL MOMENTO
// INDICADO.
ISR( INT0_vect ){
    flag = 1;
}

// FUNCION QUE HABILITA LA INTERRUPCION EN INT0.
void CONFIG_INTERRUPTION( void ){
  PCMSK |= (1<<PIND2);              // HACE QUE EL PIND2 SEA LA FUENTE DE LA INTERRUPCION
  MCUCR |= (1<<ISC01) | (1<<ISC00); // LA INTERRUPCION SE DETECTARA EN LOS FLANCOS POSITIVOS.
  GIMSK |= (1<<INT0);               // ACTIVA LA INTERRUPCION EN INT0.
  sei();
}

// ESTA FUNCION MANEJA TANTO EL DELAY DE TIEMPO PARA EL WHILE COMO EL CAMBIO EN EL CONTADOR
// QUE GOBIERNA LA MAQUINA DE ESTADOS.
void Delay( int* counter ){
    volatile unsigned int del = 20000;
    while(del--);
   
    if( flag == 0 && *counter == GREEN_RED_MAX ){
      *counter = GREEN_RED_MAX;
    }
    else{
      *counter = *counter + 1;
    }
}

int main(void)
{
    int counter = 0;
    State state = GREEN_RED;

    // CONFIGURE OUTPUT PINS
    DDRD |= (1 << PD0);
    DDRD |= (1 << PD1);
    DDRD |= (1 << PD6);
    DDRD |= (1 << PD3);
    DDRD |= (1 << PD4);
    DDRD |= (1 << PD5);

    // ALL LEDS TURN OFF.
    PORTD &= ~(1 << PD0); // PEDESTRIAN GREEN LIGHT
    PORTD &= ~(1 << PD1); // PEDESTRIAN YELLOW LIGHT
    PORTD &= ~(1 << PD6); // PEDESTRIAN RED LIGHT
    PORTD &= ~(1 << PD3); // VEHICULAR GREEN LIGHT
    PORTD &= ~(1 << PD4); // VEHICULAR YELLOW LIGHT
    PORTD &= ~(1 << PD5); // VEHICULAR RED LIGHT

    // LLAMA A LA FUNCION PARA CONFIGURAR LA INTERRUPCION
    CONFIG_INTERRUPTION();

    while(1)
    {
      // LLAMA A LA FUNCION PARA AGREGAR EL DELAY Y CAMBIAR EL VALOR
      // DE LA VARIABLE COUNTER.
      Delay( &counter );
      
      switch( state ){
        case GREEN_RED:
	  if( counter <= GREEN_RED_MAX ){
	    state = GREEN_RED;
	    PORTD &= ~(1 << PD5); // RED LED VEHICULAR TURN OFF
	    PORTD |=  (1 << PD3); // GREEN LED VEHICULAR TURN ON
	    PORTD |=  (1 << PD6); // RED LED PEDESTRIAN TURN ON
	    break;
	  }
	  else{
	    if( flag == 0 ){
	      state = GREEN_RED;
	      break;
	    }
	    else{
	      state = YELLOW_RED;
	      break;
	    }
	  }
        case YELLOW_RED:
	  if( counter <= YELLOW_RED_MAX ){
	    state = YELLOW_RED;
	    PORTD &=  ~(1 << PD3); // GREEN LED VEHICULAR TURN OFF
	    PORTD |= (1 << PD4); // YELLOW LED VEHICULAR TURN ON
	    break;
	  }
	  else{
	    state = RED_RED_A;
	    break;
	  }
        case RED_RED_A:
	  if( counter <= RED_RED_A_MAX ){
	    state = RED_RED_A;
	    PORTD &= ~(1 << PD4); // YELLOW LED VEHICULAR TURN OFF
	    PORTD |= (1 << PD5); // RED LED VEHICULAR TURN ON
	    break;
	  }
	  else{
	    state = RED_GREEN;
	    break;
	  }
        case RED_GREEN:
	  if( counter <= RED_GREEN_MAX ){
	    state = RED_GREEN;
	    PORTD &= ~(1 << PD6); // RED LED PEDESTRIAN TURN OFF
	    PORTD |=  (1 << PD0); // GREEN LED PEDESTRIAN TURN ON
	    flag = 0;
	    break;
	  }
	  else{
	    state = RED_YELLOW;
	    break;
	  }
        case RED_YELLOW:
	  if( counter <= RED_YELLOW_MAX ){
	    state = RED_YELLOW;
	    PORTD &= ~(1 << PD0); // GREEN LED PEDESTRIAN TURN OFF
	    PORTD |=  (1 << PD1); // YELLOW LED PEDESTRIAN TURN ON
	    break;
	  }
	  else{
	    state = RED_RED_B;
	    break;
	  }
        case RED_RED_B:
	  if( counter <= RED_RED_B_MAX ){
	    state = RED_RED_B;
	    PORTD &= ~(1 << PD1); // YELLOW LED PEDESTRIAN TURN OFF
	    PORTD |=  (1 << PD6); // RED LED PEDESTRIAN TURN ON
	    break;
	  }
	  else{
	    state = GREEN_RED;
	    counter = 0;
	    break;
	  }	  
      }
    }
}
