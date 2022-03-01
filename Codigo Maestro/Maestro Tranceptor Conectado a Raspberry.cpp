//CODIGO MAESTRO-TRANSCEPTOR RASPBERRY PI Y PERISFERICOS
//DANIEL FELIPE GOMEZ BERNAL
//CRISTIAN DAVID COBOS SARTA      

/*
 DISPOSITIVO | FRECUENCIA DE TRABAJO | DIRECCION |  USO |
-------------|-----------------------|-----------|------|
RASPBERRY    |        2400           | 0x000002  |  YA  |
DISPENSADOR  |        2420           | 0x00000A  |  YA  |
TOMA CORR    |        2440           | 0x00000B  |  YA  |
DIMMER       |        2460           | 0x00000C  |  YA  |
CAMARA-PUERTA|        2480           | 0x00000D  |  YA  |
*/   

#include "mbed.h"
#include "nRF24L01P.h"

#define MI_FREQ 2400
#define POT  0
#define VELO 250
#define DIR_MAESTRO 0x000002   //DIRECCION DE RECEPCION DEL MAESTRO

#define RF_DISPENSADOR 2420
#define DIR_DISPENSADOR 0x00000A      //DIRECCION DE RECEPCION EXCLAVO 1

#define RF_TOMA 2440
#define DIR_TOMA 0x00000B      //DIRECCION DE RECEPCION EXCLAVO 2

#define RF_DIMMER 2460
#define DIR_DIMMER 0x00000C

#define RF_PUERTA 2480
#define DIR_PUERTA 0x00000D

#define TAM_DIRECCIONES 3
#define TAM_TX 4

#define RETARDO 2000


nRF24L01P RADIO(PB_5, PB_4, PB_3, PA_15, PA_12);    // MOSI, MISO, SCK, CSN, CE, IRQ
Serial RASPBERRY(PA_9,PA_10);  //TX,RX
Serial PC(PA_2,PA_3);  //TX,RX
DigitalOut ON(PC_13);

void CONF_INIC (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION, int ANCHO, int TUBERIA);
void ESTADO_I (void);
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF);
void RECIBIR (void);
void ENVIAR_CICLO (void);

char DATA_TX [TAM_TX];
char RX_DATA [TAM_TX];
int  TX_CONT = 0;
int RECIBO  = 1;

char OPCION = 0;

char FG_DIMMER = 0;
char DATA_CICLO [TAM_TX];
char FG_INTERRUPTOR_ON = 0;
char FG_INTERRUPTOR_OFF = 0;

char INTE_OFF [TAM_TX] = {'L','G','O','F'};
char CONF_CIC [TAM_TX] = {'C','U','R','R'};
char FOOD_ON [TAM_TX] = {'F','D','O','N'};
char WATE_ON [TAM_TX] = {'W','R','O','N'};
char CONF_COR [TAM_TX] = {'A','M','P','R'};
char TOMA_SI [TAM_TX] = {'L','D','O','N'};
char TOMA_NO [TAM_TX] = {'L','D','O','F'};

int CENTENAS = 0;
int DECENAS = 0;
int UNIDADES = 0;
int PORCENTAJE = 0;

int DECIMAL  = 0;
int UNIDAD_1 = 0;
int UNIDAD_2 = 0;
float VALOR_COR = 0.0;

char FG_COMIDA = 0;
char FG_AGUA = 0;
char TOMA_ON = 0;
char TOMA_OFF = 0;

char LETRA = ' ';

char CONFIRMA_GV = 0;

int main (void)
{
    ON = 1;
    RADIO.powerUp();
    CONF_INIC (MI_FREQ, POT, VELO, DIR_MAESTRO, TAM_DIRECCIONES, NRF24L01P_PIPE_P0);
    RADIO.setReceiveMode();
    RADIO.enable();
    PC.printf("Maestro encendido\r\n");
    PC.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  RADIO.getRfFrequency() );
    PC.printf( "nRF24L01+ Output power : %d dBm\r\n",  RADIO.getRfOutputPower() );
    PC.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", RADIO.getAirDataRate() );
    PC.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", RADIO.getTxAddress() );
    PC.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", RADIO.getRxAddress() );
    while (1)
    {
        if(RASPBERRY.readable() == 1) //EVALUA SI HAY ALGUNA ACCION POR REALIZAR SEGUN LO QUE LA RASPBERRY ENVIA
        {
            PC.printf("Hay algo para leer\r\n");
            OPCION = RASPBERRY.getc();
            switch (OPCION) //DEFINE CUÁL ACCION SE REQUIERE REALIZAR (QUE CAMBIO SE REALIZO EN UBIDOTS)
            {
                case 'A':
                {
                    FG_DIMMER = 1;
                    PC.printf("A: CAMBIO DIMMER\r\n");
                    break;
                }
                case 'B':
                {
                    FG_INTERRUPTOR_ON = 1;
                    PC.printf("B: DIMMER EN ON\r\n");
                    break;
                }
                case 'C':
                {
                    FG_INTERRUPTOR_OFF = 1;
                    PC.printf("C: DIMMER EN OFF\r\n");
                    break;
                }
                case 'D':
                {
                    FG_COMIDA = 1;
                    PC.printf("D\r\n");
                    break;
                }
                case 'E':
                {
                    FG_AGUA = 1;
                    PC.printf("E\r\n");
                    break;
                }
                case 'F':
                {
                    TOMA_ON = 1;
                    PC.printf("F:\r\n");
                    break;
                }
                case 'G':
                {
                    TOMA_OFF = 1;
                    PC.printf("G:\r\n");
                    break;
                }
                    
            }    
        }
        //TOMAR ACCIONES
        if(FG_DIMMER == 1)              //OPTIMIZAR
        {
            PC.printf("FUNCION CAMBIAR DIMMER\r\n");
            ENVIAR_CICLO();         //LE SOLICITA A LA RASPBERRY EL VALOR DEL DIMMER, Y LO ENVIA AL ESCLAVO CORRESPONDIENTE, ADEMAS DA ESPERA A UNA RESPUESTA DE QUE LA INFORMACION FUE RECIBIDA.
            RASPBERRY.putc('R');
            PC.printf("RESPONDIO CON UNA R \r\n");
            FG_DIMMER = 0;    
        }
        if(FG_INTERRUPTOR_ON == 1)      //OPTIMIZAR
        {
            PC.printf("FUNCION ENCENDER INTERRUPTOR\r\n");
            ENVIAR_CICLO();         //LE SOLICITA A LA RASPBERRY EL VALOR DEL DIMMER, Y LO ENVIA AL ESCLAVO CORRESPONDIENTE, ADEMAS DA ESPERA A UNA RESPUESTA DE QUE LA INFORMACION FUE RECIBIDA.         
            RASPBERRY.putc('R');
            PC.printf("RESPONDIO CON UNA R \r\n");
            FG_INTERRUPTOR_ON = 0;
        }
        if(FG_INTERRUPTOR_OFF == 1)
        {
            PC.printf("FUNCION APAGAR INTERRUPTOR\r\n");
            char RESP = 0;     //UBICAR 
            while(RESP == 0)
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_DIMMER, TAM_DIRECCIONES, RF_DIMMER);
                RADIO.write(NRF24L01P_PIPE_P0, INTE_OFF, TAM_TX);
                PC.printf("RADIO ENVIO MENSAJE \r\n");
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                wait_ms (RETARDO);
                if(RADIO.readable())
                {
                    PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
                    RECIBIR();
                    if(RX_DATA[0] == 'O' && RX_DATA[1] == 'O' && RX_DATA[2] == 'U' && RX_DATA[3] == 'U')
                    {
                        RESP = 1;
                        RADIO.setRfFrequency (MI_FREQ);
                        RADIO.setReceiveMode();
                        RASPBERRY.putc('I');    //RESPONDER QUE SE RECIBIO LA ORDEN
                        PC.printf("RESPONDIO CON UNA I \r\n");
                    }    
                }    
            }
            FG_INTERRUPTOR_OFF = 0;
        }
        if(FG_COMIDA == 1)
        {
            PC.printf("FUNCION DISPENSAR COMIDA\r\n");
            char RESP_1 = 0;     //UBICAR 
            while(RESP_1 == 0)
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_DISPENSADOR, TAM_DIRECCIONES, RF_DISPENSADOR);
                RADIO.write(NRF24L01P_PIPE_P0, FOOD_ON, TAM_TX);
                PC.printf("RADIO ENVIO MENSAJE \r\n");
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                wait_ms (RETARDO);
                if(RADIO.readable())
                {
                    PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
                    RECIBIR();
                    if(RX_DATA[0] == 'D' && RX_DATA[1] == 'F' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
                    {
                        RESP_1 = 1;
                        RADIO.setRfFrequency (MI_FREQ);
                        RADIO.setReceiveMode();
                        RASPBERRY.putc('G');    //RESPONDER QUE SE RECIBIO LA ORDEN
                        PC.printf("RESPONDIO CON UNA G \r\n");
                    }    
                }    
            }
            FG_COMIDA = 0;
        }
        if(FG_AGUA == 1)
        {
            PC.printf("FUNCION DISPENSAR AGUA\r\n");
            char RESP_2 = 0;     //UBICAR 
            while(RESP_2 == 0)
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_DISPENSADOR, TAM_DIRECCIONES, RF_DISPENSADOR);
                RADIO.write(NRF24L01P_PIPE_P0, WATE_ON, TAM_TX);
                PC.printf("RADIO ENVIO MENSAJE \r\n");
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                wait_ms (RETARDO);
                if(RADIO.readable())
                {
                    PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
                    RECIBIR();
                    if(RX_DATA[0] == 'D' && RX_DATA[1] == 'A' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
                    {
                        RESP_2 = 1;
                        RADIO.setRfFrequency (MI_FREQ);
                        RADIO.setReceiveMode();
                        RASPBERRY.putc('H');    //RESPONDER QUE SE RECIBIO LA ORDEN
                        PC.printf("RESPONDIO CON UNA H \r\n");
                    }    
                }    
            }
            FG_AGUA = 0;
        }
        if(TOMA_ON == 1)
        {
            PC.printf("FUNCION ENCENDER TOMA\r\n");
            char RESP_3 = 0;     //UBICAR 
            while(RESP_3 == 0)
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_TOMA, TAM_DIRECCIONES, RF_TOMA);
                RADIO.write(NRF24L01P_PIPE_P0, TOMA_SI, TAM_TX);
                PC.printf("RADIO ENVIO MENSAJE \r\n");
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                wait_ms (RETARDO);
                if(RADIO.readable())
                {
                    PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
                    RECIBIR();
                    if(RX_DATA[0] == 'T' && RX_DATA[1] == 'M' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
                    {
                        RESP_3 = 1;
                        RADIO.setRfFrequency (MI_FREQ);
                        RADIO.setReceiveMode();
                        RASPBERRY.putc('F');    //RESPONDER QUE SE RECIBIO LA ORDEN
                        PC.printf("RESPONDIO CON UNA F \r\n");
                    }    
                }    
            }
            TOMA_ON = 0;
        }    
        if(TOMA_OFF == 1)
        {
            PC.printf("FUNCION APAGAR TOMA\r\n");
            char RESP_4 = 0;     //UBICAR 
            while(RESP_4 == 0)
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_TOMA, TAM_DIRECCIONES, RF_TOMA);
                RADIO.write(NRF24L01P_PIPE_P0, TOMA_NO, TAM_TX);
                PC.printf("RADIO ENVIO MENSAJE \r\n");
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                wait_ms (RETARDO);
                if(RADIO.readable())
                {
                    PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
                    RECIBIR();
                    if(RX_DATA[0] == 'T' && RX_DATA[1] == 'M' && RX_DATA[2] == 'O' && RX_DATA[3] == 'F')
                    {
                        RESP_4 = 1;
                        RADIO.setRfFrequency (MI_FREQ);
                        RADIO.setReceiveMode();
                        RASPBERRY.putc('N');    //RESPONDER QUE SE RECIBIO LA ORDEN
                        PC.printf("RESPONDIO CON UNA N \r\n");
                    }    
                }    
            }
            TOMA_OFF = 0;
        } 
        if(RADIO.readable())
        {           
            PC.printf("ALGO LLEGO\r\n");
            RECIBIR();                    
            if(RX_DATA[0] == 'S' && RX_DATA[1] == 'P' && RX_DATA[2] == 'A' && RX_DATA[3] == 'D')
            {
                PC.printf("LA PUERTA SE ABRIO\r\n"); 
                RASPBERRY.putc('P');  
                                                         
                for (int i = 0; i<4;i++)        // LIMPIA  BASURA POR SI QUEDO DE LA ANTERIOR ALERTA 
                {
                    DATA_TX[i] = ' ';
                } 
                
                DATA_TX [0] = 'S';
                DATA_TX [1] = 'G';
                DATA_TX [2] = 'Y';
                DATA_TX [3] = 'E';
                
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_PUERTA, TAM_DIRECCIONES, RF_PUERTA);
                wait_ms(250);
                RADIO.write(NRF24L01P_PIPE_P0, DATA_TX, TAM_TX);
                PC.printf("SE LE RESPONDIO QUE LA ALERTA SE RECIBIO \r\n"); 
                RADIO.setRfFrequency(MI_FREQ);
                RADIO.setReceiveMode(); 
                
                CONFIRMA_GV = RASPBERRY.getc();                            
                if (CONFIRMA_GV == 'T') // CONFIRMA RESPUESTA DE RBP
                {
                    PC.printf("SE COMPLETO LA GRABACION Y EL ENVIO \r\n");                                                                        
                }                                                           
            }
                   
            if(RX_DATA [3] == 'C')
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_DIMMER, TAM_DIRECCIONES, RF_DIMMER);
                wait_ms(250);
                RADIO.write(NRF24L01P_PIPE_P0, CONF_CIC, TAM_TX);
                PC.printf("SE RESPONDIO \r\n");
                for(int i = 0; i<TAM_TX; i++)
                {
                    PC.printf("%c",CONF_CIC[i]);
                }
                PC.printf("\r\n");
                RADIO.setRfFrequency(MI_FREQ);
                RADIO.setReceiveMode();
                
                CENTENAS = (RX_DATA [0] - 48) * 100;
                DECENAS = (RX_DATA [1] - 48) * 10;
                UNIDADES = (RX_DATA [2] - 48);
                PORCENTAJE = CENTENAS + DECENAS + UNIDADES;
               
                PC.printf("%d %d %d %d \r\n",CENTENAS,DECENAS,UNIDADES, PORCENTAJE);
                 
                RASPBERRY.putc('C');
                char E_CICLO = 0;
                while (E_CICLO == 0)
                {
                    if(RASPBERRY.readable() == 1)
                    {
                        LETRA = RASPBERRY.getc();
                        if(LETRA == 'C')
                        {
                            RASPBERRY.printf("%d",PORCENTAJE);
                            PC.printf("%d",PORCENTAJE);
                            E_CICLO = 1;
                            char CONFIRMAR = 0;
                            while (CONFIRMAR == 0)
                            {
                                if(RASPBERRY.readable() == 1)
                                {
                                    LETRA = RASPBERRY.getc();
                                    if(LETRA == 'c')
                                    {
                                        CONFIRMAR = 1;
                                    }
                                }
                            }
                        }
                        
                    }
                }
            }
            if(RX_DATA [3] == 'Z')
            {
                RADIO.setTransmitMode();
                PREPARAR(TAM_TX, DIR_TOMA, TAM_DIRECCIONES, RF_TOMA);
                wait_ms(250);
                RADIO.write(NRF24L01P_PIPE_P0, CONF_COR, TAM_TX);
                PC.printf("SE RESPONDIO \r\n");
                for(int i = 0; i<TAM_TX; i++)
                {
                    PC.printf("%c",CONF_COR[i]);
                }
                PC.printf("\r\n");
                RADIO.setRfFrequency(MI_FREQ);
                RADIO.setReceiveMode();
 
                DECIMAL  = (RX_DATA [0] - 48) * 100;
                UNIDAD_1 = (RX_DATA [1] - 48) *10;
                UNIDAD_2 = (RX_DATA [2] - 48);  
                
                VALOR_COR = DECIMAL + UNIDAD_1 + UNIDAD_2;
                              
                PC.printf("%f \r\n",VALOR_COR);                                  
                RASPBERRY.putc('Z');

                char E_COR = 0;                                                      
                while (E_COR == 0)
                {
                    if(RASPBERRY.readable() == 1)
                    {
                        LETRA = RASPBERRY.getc();
                        if(LETRA == 'Z')
                        {
                            RASPBERRY.printf("%f",VALOR_COR);
                            E_COR = 1;
                            char CONFIRMAR = 0;
                            while (CONFIRMAR == 0)
                            {
                                if(RASPBERRY.readable() == 1)
                                {
                                    LETRA = RASPBERRY.getc();
                                    if(LETRA == 'z')
                                    {
                                        CONFIRMAR = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }     
    }
}

void CONF_INIC (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION, int ANCHO, int TUBERIA)
{
    RADIO.setRfFrequency(FRECUENCIA);
    RADIO.setRfOutputPower(POTENCIA);
    RADIO.setAirDataRate(VELOCIDAD);
    RADIO.setRxAddress(DIRECCION, ANCHO, TUBERIA);
}
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF)
{
    RADIO.setTransferSize (ANCHO);
    RADIO.setTxAddress (DIRECCION,TAM_DIR);
    RADIO.setRfFrequency (RF);
}
void RECIBIR (void)
{
    int rxDataCnt = 0;
    RADIO.read(NRF24L01P_PIPE_P0, RX_DATA, TAM_TX);
    for(int i = 0; i<=TAM_TX; i++)
    {
        PC.printf("%c",RX_DATA[i]);
    }
    PC.printf("\r\n");
}
void ENVIAR_CICLO (void)
{
    RASPBERRY.putc('U');    //RESPONDER QUE SE ESTA LISTO PARA RECIBIR EL ARREGLO
    PC.printf("RESPONDIO CON UNA U \r\n");
    char ESPERANDO = 1;     //UBICAR
    char POS = 0;           //UBICAR
    while(ESPERANDO == 1)
    {
        if(RASPBERRY.readable() == 1)
        {
            DATA_CICLO [POS] = RASPBERRY.getc();
            POS = POS + 1;
        }
        if(POS > 3)
        {
            POS = ESPERANDO = 0;
        }        
    }  
    char RESPUESTA = 0;     //UBICAR 
    while(RESPUESTA == 0)
    {
        RADIO.setTransmitMode();
        PREPARAR(TAM_TX, DIR_DIMMER, TAM_DIRECCIONES, RF_DIMMER);
        RADIO.write(NRF24L01P_PIPE_P0, DATA_CICLO, TAM_TX);
        PC.printf("+++RADIO ENVIO INFORMACION+++ \r\n");
        RADIO.setRfFrequency (MI_FREQ);
        RADIO.setReceiveMode();
        wait_ms (RETARDO);
        if(RADIO.readable())
        {
            PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
            RECIBIR();
            if(RX_DATA[0] == 'U' && RX_DATA[1] == 'U' && RX_DATA[2] == 'U' && RX_DATA[3] == 'U')
            {
                RESPUESTA = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
            }
        }
    }
}