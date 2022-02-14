//CODIGO ESCLACO CONTROL PUERTA Y CAMARA
//EN RADIOS nRF24L01   
//otra prueba de esta joda 

#include "mbed.h"
#include "nRF24L01P.h"

#define RETARDO       2000

#define MI_FREQ_MST 2400
#define DIR_MAESTRO 0x000002   //DIRECCION DE RECEPCION DEL MAESTRO

#define RF_CAMARA 2480
#define DIR_CAMARA 0x00000D
#define POTENCIA_T      0
#define VEL_T           250
#define TAMANO_DIR      3
  
#define TAMANO          4

Serial PC(PA_2,PA_3);//TX,RX

nRF24L01P RADIO(PB_5, PB_4, PB_3, PA_15, PA_12);    // MOSI, MISO, SCK, CSN, CE, IRQ----IRQ NO ESTA DEFINIDO NI CONECTADO, LA RECOMENDACION VIENE DADA POR LA LIBRERIA USADA

DigitalOut      ON(PC_13);
DigitalIn       SENSOR_PUERTA (PB_10); //(PULL UP) |PIN EN ALTO: PUERTA ABIERTA, PIN EN BAJO PUERTA CERRADA  
DigitalOut      INDICADOR (PA_10);     //INDICADOR LED PUERTA ABIERTA -- POSIBLE INDICADOR CUANDO CORREO SEA ENVIADO O GRABACION TOMADA 

void CONF_GENER (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION_RX, int TAMAÑO_D, int TUBERIA);
void CONF_RADIO (unsigned long long DIRECCION_TX, int TAM_INFO);
void RECIBIR (void);
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF);
void ENVIAR_A(void);

char RX_DATA [TAMANO];
char TX_DATA [TAMANO];

//-------VARiABLES PARA LA CAMARA---------
char ALERTA  = 0;
char PERMISO = 0; 

int main ()
{   
    ON = 0;
    RADIO.powerUp();                                                                             //Radio ENCENDIDO y en modo STANDBY
    CONF_GENER (RF_CAMARA, POTENCIA_T, VEL_T, DIR_CAMARA, TAMANO_DIR, NRF24L01P_PIPE_P0);                     //CONFIGURACION INICIAL radio
    CONF_RADIO (DIR_MAESTRO, TAMANO); 
    PC.printf("***CONF_INICIAL**\r\n");                              //DIRECCION INICIAL de Transmision
    PC.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  RADIO.getRfFrequency() );
    PC.printf( "nRF24L01+ Output power : %d dBm\r\n",  RADIO.getRfOutputPower() );
    PC.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", RADIO.getAirDataRate() );
    PC.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", RADIO.getTxAddress() );
    PC.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", RADIO.getRxAddress() );
    PC.printf("*******\r\n");
    //RADIO.setReceiveMode(); //Modo de RECEPCION ACTIVADO
    RADIO.setTransmitMode();    // MANDA TODO EL TIEMPO EL ESTADO 
    RADIO.enable();
    
    INDICADOR = 0; 
    
    while (1)
    {             
        while(SENSOR_PUERTA == 1)
        {
            PC.printf("PUERTA ABIERTA \r\n");  
            INDICADOR = 1; 
            wait_ms (1000);  
            ALERTA = 1;
            if(ALERTA == 1 && PERMISO == 0)
            {
                PC.printf("ENTRO AL IF DE ALERTA \r\n");
                for (int i = 0; i<4;i++)        // LIMPIA POR SI QUEDO DE LA ANTERIOR ALERTA 
                {
                    TX_DATA[i] = ' ';
                }  
                ENVIAR_A();                     //ENVIA                  
            }                       
        }
        while(SENSOR_PUERTA == 0)
        {
            PC.printf("PUERTA CERRADA\r\n");  
            INDICADOR = 0;  
            wait_ms (1000);  
            //ALERTA = 1;     
            PERMISO = 0;                 
        }
        
        
        /*
        if(SENSOR_PUERTA == 1)
        {
            PC.printf("PUERTA ABIERTA \r\n");   
            wait_ms (RETARDO);             
            INDICADOR = 1;  
            ALERTA = 1;                         
                                    
            while(ALERTA == 1 && PERMISO == 0)
            {
                for (int i = 0; i<4;i++)        // LIMPIA POR SI QUEDO DE LA ANTERIOR ALERTA 
                {
                    TX_DATA[i] = ' ';
                }  
                ENVIAR_A();                     //ENVIA                      
                //ALERTA = 0;                     //ENVIA UNA SOLA ALERTA                           
            }               
        }
        
        if(SENSOR_PUERTA == 0)// NO HACE NADA ** PONER POSIBLE BOTON EN UBIDOTS PARA GRABAR CON PUERTA CERRADA 
        {
            PC.printf("PUERTA CERRADA  \r\n"); 
            ALERTA = 1;    
            INDICADOR = 0;  
            PERMISO = 0;
            wait_ms (RETARDO);
        } 
        */           
    }                                                                        
}

void CONF_GENER (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION_RX, int TAMAÑO_D, int TUBERIA)
{
    RADIO.setRfFrequency(FRECUENCIA);                        //FRECUENCIA de TRANSMISION en MHz  (2400-2525)
    RADIO.setRfOutputPower(POTENCIA);                        //POTENCIA DE SALIDA EN dBm
    RADIO.setAirDataRate(VELOCIDAD);                         //Velocidad de TRASNFERENCIA de Datos en KBTS/S
    RADIO.setRxAddress(DIRECCION_RX, TAMAÑO_D, TUBERIA);     //Configuracion de DIRECCION de RECEPCION (DIRECCION, TAMAÑO de la DIRECCION en bytes, TUBERIA 0-5) LAS TUBERIAS 0 Y 1 admiten tamaños de 3,4,5 bytes. Las Demas por defecto solo tienen un byte de tamaño para la direccion
}
void CONF_RADIO (unsigned long long DIRECCION_TX, int TAM_INFO)
{
    RADIO.setTxAddress(DIRECCION_TX, TAMANO_DIR);              //Configuracion de DIRECCION de TRANSMISION (DIRECCION, TAMAÑO de la DIRECCION en bytes) LA TUBERIA va directamente LIGADA a la configurada en la RECEPCION
    RADIO.setTransferSize(TAM_INFO);                           //ESTABLECER el TAMAÑO en BYTES de la TRANSFERENCIA 
}    
void RECIBIR (void)
{
    int rxDataCnt = 0;   
    RADIO.read(NRF24L01P_PIPE_P0, RX_DATA, TAMANO);
    for(int i = 0; i<=TAMANO; i++)
    {
        PC.printf("%c",RX_DATA[i]);       
    }
    PC.printf("\r\n");
}
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF)
{
    RADIO.setTransferSize (ANCHO);
    RADIO.setTxAddress (DIRECCION,TAM_DIR);
    RADIO.setRfFrequency (RF);
}

void ENVIAR_A(void)
{
    TX_DATA [0] = 'S';          //ENVIA LAERTA QUE SE ABRIO LA PUERTA AL MAESTRO 
    TX_DATA [1] = 'P';
    TX_DATA [2] = 'A';
    TX_DATA [3] = 'D';
    //
    char RESP = 0;    
    char CONT = 0; 
     
    while(RESP == 0)
    {
        RADIO.setTransmitMode();
        RADIO.setRfFrequency(MI_FREQ_MST);
        RADIO.write(NRF24L01P_PIPE_P0, TX_DATA, TAMANO);
        PC.printf("RADIO ENVIO MENSAJE \r\n");
        RADIO.setRfFrequency (RF_CAMARA);
        RADIO.setReceiveMode();
        wait_ms (RETARDO);  
        
        CONT = CONT + 1;  
        if(CONT == 5)
        {
            PERMISO = 1;
            RESP = 1;           
            PC.printf("CONTADOR = %d\r\n",CONT);  
        }  
                            
        if(RADIO.readable())
        {
            PC.printf("RADIO TIENE ALGO PARA LEER \r\n"); 
            RECIBIR();
            if(RX_DATA[0] == 'S' && RX_DATA[1] == 'G' && RX_DATA[2] == 'Y' && RX_DATA[3] == 'E') //confirma la recepcion de datos 
            {
                PC.printf("SE CONFIRMO LA ALERTA, INICIO GRABACION Y EL ENVIO \r\n");
                RESP = 1;
                ALERTA = 0;
                PERMISO = 1;                           
                RADIO.setTransmitMode();
                RADIO.setRfFrequency(MI_FREQ_MST);
            }               
        }           
    }   
}