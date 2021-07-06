#include <LiquidCrystal.h>

//--------------------------------------Mapeamento de Hardware----------------------------------//
#define btSelecionaTensao   (1<<4)
#define btDiminuiTempo      (1<<5)
#define btAumentaTempo      (1<<6)
#define btStartStop         (1<<7)
//----------------------------------------------------------------------------------------------//
//--------------------------------------Variaveis Globais---------------------------------------//
const unsigned int T_1s = 49910;        //Variavel auxiliar para criar base de tempo de 1 segundo (utilizada no Timer 1)
const unsigned int T_100us = 63935;     //Variavel auxiliar para criar base de tempo de 100us (utilizada nos Timers 3, 4 e 5)
unsigned char tempoRampa = 20;
unsigned char tempoDecorrido = 0;
unsigned char nPulsosFase1 = 0;
unsigned char nPulsosFase2 = 0;
unsigned char nPulsosFase3 = 0;
unsigned char pulsoDeDisparo = 82;
boolean flagBtOnOff = 0x00;
boolean flagBtSelecionaTensao = 0x00;
boolean flagBtDiminuiTempo = 0x00;
boolean flagBtAumentaTempo = 0x00;
boolean flagBtStartStop = 0x00;
boolean flagRedeEletrica = 0;
boolean controleStartStop = 0;

LiquidCrystal lcd(22,23,24,25,26,27);

void atualizaDiplay();

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
   if(tempoDecorrido >= tempoRampa){
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
      digitalWrite(4,HIGH);
   } else if(nPulsosFase1 >pulsoDeDisparo+5){
      digitalWrite(4,LOW);
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
  lcd.begin(16,2);
//-----------------------------------------------------------Configuração de GPIOS---------------------------------------------------------------------//
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);

  //Configuração de input´s para botoes da IHM
  DDRB &= ~btSelecionaTensao;
  DDRB &= ~btDiminuiTempo;
  DDRB &= ~btAumentaTempo;
  DDRB &= ~btStartStop;
  
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
   while(1)
   {
      //Serial.println(pulsoDeDisparo);
      atualizaDiplay();

      if(PINB&btSelecionaTensao){
        flagRedeEletrica = 1;
      } else if(!(PINB&btSelecionaTensao)){
        flagRedeEletrica = 0;
      }
      
      if(!(PINB&btDiminuiTempo))   flagBtDiminuiTempo=0x01;
      if((PINB&btDiminuiTempo)&&flagBtDiminuiTempo){
          flagBtDiminuiTempo=0x00;
          if(tempoRampa>=10)
            tempoRampa--;
          Serial.println("Pressionou diminui tempo");
      }

      if(!(PINB&btAumentaTempo))   flagBtAumentaTempo=0x01;
      if((PINB&btAumentaTempo)&&flagBtAumentaTempo){
          flagBtAumentaTempo=0x00;
          if(tempoRampa<=90)
            tempoRampa++;
          Serial.println("Pressionou Aumenta tempo");
      }
      
      if(!(PINB&btStartStop))   flagBtStartStop=0x01;
      if((PINB&btStartStop)&&flagBtStartStop){
          flagBtStartStop=0x00;
          if(controleStartStop){
              //TIMSK1 = 0x01;  // Habilita interrupcao por tempo de estouro do timer 1
              controleStartStop = 0;
          } else if(!controleStartStop){
              controleStartStop = 1;
          }
          Serial.println("Pressionou Start Stop");
      }
   }
}

void atualizaDiplay(){
      static boolean limpa1 = flagRedeEletrica;
      static boolean limpa2= controleStartStop;

      if(limpa1!=flagRedeEletrica || limpa2!=controleStartStop){
        lcd.clear();
      }
      
      lcd.setCursor(0,0);
      lcd.print("ON");
      lcd.setCursor(0,1);
      if(!flagRedeEletrica)
        lcd.print("MONO");
      else if(flagRedeEletrica)
        lcd.print("TRI");
      lcd.setCursor(7,1);
      lcd.print("TEMPO:");
      lcd.print(tempoRampa);
      lcd.setCursor(7,0);
      if(!controleStartStop)
        lcd.print("STOP");
      else if(controleStartStop)
        lcd.print("START");

      limpa1 = flagRedeEletrica;
      limpa2 = controleStartStop;
}
