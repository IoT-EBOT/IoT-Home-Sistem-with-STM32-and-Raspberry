//CODIGO ESCLAVO CONTROL DISPENSADOR                                                        

#include "mbed.h"
#include "nRF24L01P.h"

#define RETARDO       2000

#define D_COMIDA    10
#define D_AGUA      5

#define MI_FREQ_MST 2400
#define DIR_MAESTRO 0x000002   //DIRECCION DE RECEPCION DEL MAESTRO


#define RF_DISPENSADOR 2420
#define DIR_DISPENSADOR 0x00000A
#define POTENCIA_T      0
#define VEL_T           250
#define TAMANO_DIR      3
  
#define TAMANO          4

Serial PC(PA_2,PA_3);//TX,RX

nRF24L01P RADIO(PB_5, PB_4, PB_3, PA_15, PA_12);    // MOSI, MISO, SCK, CSN, CE, IRQ----IRQ NO ESTA DEFINIDO NI CONECTADO, LA RECOMENDACION VIENE DADA POR LA LIBRERIA USADA

            //LSB  MSB
BusOut  FOOD (PA_9,PA_8);
BusOut  WATER(PA_11,PA_10);

// LOS POLOS POSITIVOS ESTÁN EN LAS SALIDAS PARES (OU2,OUT4)

Timeout DETENER_C;
Timeout DETENER_A;

void CONF_GENER (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION_RX, int TAMAÑO_D, int TUBERIA);
void CONF_RADIO (unsigned long long DIRECCION_TX, int TAM_INFO);
void RECIBIR (void);
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF);

void APAGAR_COMIDA (void);
void APAGAR_AGUA (void);

char RX_DATA [TAMANO];
char CONFIRMAR [TAMANO] = {'D','F','O','N'};
char CONFIRMAR_2 [TAMANO] = {'D','A','O','N'};

char COMIDA = 0;
char AGUA = 0;

char C_D = 0;
char C_A = 0;

int main ()
{
    RADIO.powerUp();    //Radio ENCENDIDO y en modo STANDBY
    CONF_GENER (RF_DISPENSADOR, POTENCIA_T, VEL_T, DIR_DISPENSADOR, TAMANO_DIR, NRF24L01P_PIPE_P0); //CONFIGURACION INICIAL radio
    CONF_RADIO (DIR_MAESTRO, TAMANO); 
    PC.printf("********************CONF_INICIAL********************\r\n");  //DIRECCION INICIAL de Transmision
    PC.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  RADIO.getRfFrequency() );
    PC.printf( "nRF24L01+ Output power : %d dBm\r\n",  RADIO.getRfOutputPower() );
    PC.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", RADIO.getAirDataRate() );
    PC.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", RADIO.getTxAddress() );
    PC.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", RADIO.getRxAddress() );
    PC.printf("****************************************************\r\n");
    RADIO.setReceiveMode(); //Modo de RECEPCION ACTIVADO
    RADIO.enable();
    
    while (1)
    {
        if(RADIO.readable())
        {
            PC.printf("ALGO LLEGO\r\n");
            RECIBIR();     
            if(RX_DATA [0] == 'F' && RX_DATA [1] == 'D' && RX_DATA [2] == 'O' && RX_DATA [3] == 'N')
            {
                COMIDA = 1;
                RADIO.setTransmitMode();
                char RESPUESTA = 0;
                while(RESPUESTA == 0)
                {
                    PREPARAR (TAMANO, DIR_MAESTRO, TAMANO_DIR, MI_FREQ_MST);
                    wait_ms(250);
                    RADIO.write(NRF24L01P_PIPE_P0, CONFIRMAR, TAMANO);
                    RESPUESTA = RESPUESTA + 1;
                    for(int i = 0; i<TAMANO; i++)
                    {
                        PC.printf("%c",CONFIRMAR [i]);
                    }
                    PC.printf("\r\n");
                }
                RESPUESTA = 0;
                RADIO.setRfFrequency(RF_DISPENSADOR);
                RADIO.setReceiveMode();
            }
            if(RX_DATA [0] == 'W' && RX_DATA [1] == 'R' && RX_DATA [2] == 'O' && RX_DATA [3] == 'N')
            {
                AGUA = 1;
                RADIO.setTransmitMode();
                char RESPUESTA = 0;
                while(RESPUESTA == 0)
                {
                    PREPARAR (TAMANO, DIR_MAESTRO, TAMANO_DIR, MI_FREQ_MST);
                    wait_ms(250);
                    RADIO.write(NRF24L01P_PIPE_P0, CONFIRMAR_2, TAMANO);
                    RESPUESTA = RESPUESTA + 1;
                    for(int i = 0; i<TAMANO; i++)
                    {
                        PC.printf("%c",CONFIRMAR_2 [i]);
                    }
                    PC.printf("\r\n");
                }
                RESPUESTA = 0;
                RADIO.setRfFrequency(RF_DISPENSADOR);
                RADIO.setReceiveMode();
            }
            for (int i = 0; i<4;i++)
            {
                RX_DATA[i] = ' ';
            }           
        }
        if (COMIDA == 1)
        {
            FOOD = 1;
            DETENER_C.attach(&APAGAR_COMIDA,D_COMIDA);
            COMIDA = 0;
        }
        if (AGUA == 1)
        {
            WATER = 1;
            DETENER_A.attach(&APAGAR_AGUA,D_AGUA);
            AGUA = 0;
        }
        if(C_D == 1)
        {
            FOOD = 0;
            C_D = 0;
        } 
        if(C_A == 1)
        {
            WATER = 0;
            C_A = 0;
        }
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
void APAGAR_COMIDA (void)
{
    C_D = 1;
}
void APAGAR_AGUA (void)
{
    C_A = 1;
}