const unsigned int T_1s = 49910;        //Variavel auxiliar para criar base de tempo de 1 segundo (utilizada no Timer 1)
const unsigned int T_100us = 63935;     //Variavel auxiliar para criar base de tempo de 100us (utilizada nos Timers 3, 4 e 5)
unsigned char tempoRampa = 20;
unsigned char tempoDecorrido = 0;
unsigned char nPulsosFase1 = 0;
unsigned char nPulsosFase2 = 0;
unsigned char nPulsosFase3 = 0;
unsigned char pulsoDeDisparo=82;

//--------------------------------------Rotinas interrupção externa----------------------------------//
ISR(INT0_vect){
 TIMSK3 = 0x01;  // Habilita flag de interrupção por estouro do timer counter do timer 3
 nPulsosFase1 = 0;
}
ISR(INT1_vect){
 TIMSK4 = 0x01;  // Habilita flag de interrupção por estouro do timer counter do timer 4
}
ISR(INT2_vect){
 TIMSK5 = 0x01;  // Habilita flag de interrupção por estouro do timer counter do timer 5 
}
//---------------------------------------------------------------------------------------------------//
//--------------------------Rotinas de interrução por estouro do Timer-------------------------------//
ISR(TIMER1_OVF_vect)
{
   TCNT1 = T_1s;
   if(tempoDecorrido == tempoRampa){
      tempoDecorrido =0;
   }
   pulsoDeDisparo= 82 -(((float)tempoDecorrido/tempoRampa)*82);
   tempoDecorrido++;   
}
ISR(TIMER3_OVF_vect)
{
   TCNT3 = T_100us;
   nPulsosFase1++;
   if(nPulsosFase1 <= pulsoDeDisparo){
      digitalWrite(12,HIGH);
   } else if(nPulsosFase1 >pulsoDeDisparo+5){
      digitalWrite(12,LOW);
   }
}  
ISR(TIMER4_OVF_vect)
{
   TCNT4 = T_100us;
   nPulsosFase2++;
}
ISR(TIMER5_OVF_vect)
{
   TCNT5 = T_100us;
   nPulsosFase3++;
}
//---------------------------------------------------------------------------------------------------//

void setup() {
//-----------------------------------------------------------Configuração de GPIOS---------------------------------------------------------------------//
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
  
//---------------------------------------------------Bloco de config de interruções externas-----------------------------------------------------------//
  /*
   * Apesar de estarem configurados como pinos 2, 3 e 4 do PORTD as entradas reais ficam nos pinos D0,D1 e D2 respecitvamente. 
   * Provavelmente exista algum bug no compilador da IDE do arduino que gera esse "shift" nos bits de entrada.
  */
   //Configuração dos pinos como entradas digitais (Gatilhos da iterrupção externa)
   DDRD &= ~(1<<DDD2);  
   DDRD &= ~(1<<DDD3);
   DDRD &= ~(1<<DDD4);

   //Bloco para habilitar pull-up internos nas entradas. Desnecessário pois circuito possui pull-up externo com resistor de 10K em 5V
   /*
    * PORTD |= (1<<PORTD2);
    * PORTD |= (1<<PORTD3);
    * PORTD |= (1<<PORTD4);
   */
   EICRA |= (1<<ISC01)|(1<<ISC00);  //Configura iterrupcao externa 0 com borda de subida
   EIMSK |= (1<<INT0);              //Habilita interrupção externa no pino PD0
   
   EICRA |= (1<<ISC11)|(1<<ISC10);  //Configura iterrupcao externa 1 com borda de subida
   EIMSK |= (1<<INT1);              //Habilita interrupção externa no pino PD1
   
   EICRA |= (1<<ISC21)|(1<<ISC20);  //Configura iterrupcao externa 2 com borda de subida
   EIMSK |= (1<<INT2);              //Habilita interrupção externa no pino PD2
   
   sei();                           //Habilita as interruções em escobo global
   
//----------------------------------------------------------------------------------------------------------------------------------//
//--------------------------------------------------Bloco de configuração dos Timers------------------------------------------------//
// Criando bases de tempo de 1 segundo e 100 microsegundos
    TCCR1A = 0x00;      //Configura timer para operar em modo normal (estouro do timer counter)
    TCCR1B = 0x05;      //Configuração do preescaler 
    TCNT1  = T_1s;      //Init do timer counter 'TCN' para ajustar tempo de estouro
    TIMSK1 = 0x00;      //Flag para ativar rotina de interrupção a cada estouro do Timer Counter ('0x00 inicialmente desligada')
    
    TCCR3A = 0x00;
    TCCR3B = 0x01;
    TCNT3  = T_100us;
    TIMSK3 = 0x00;
    
    TCCR4A = 0x00;
    TCCR4B = 0x01;
    TCNT4  = T_100us;
    TIMSK4 = 0x00;
    
    TCCR5A = 0x00;
    TCCR5B = 0x01;
    TCNT5  = T_100us;
    TIMSK5 = 0x00;
//----------------------------------------------------------------------------------------------------------------------------------//
 Serial.begin(9600);    
 TIMSK1 = 0x01;
}

void loop() {
  delay(1000);
  Serial.println(pulsoDeDisparo);
}
