//CODIGO ESCLAVO CONTROL DE LUZ Y DIMMER
//EN RADIOS nRF24L01                                                         

#include "mbed.h"
#include "nRF24L01P.h"

#define RETARDO       2000

#define MI_FREQ_MST 2400
#define DIR_MAESTRO 0x000002   //DIRECCION DE RECEPCION DEL MAESTRO


#define RF_DIMMER 2460
#define DIR_DIMMER 0x00000C
#define POTENCIA_T      0
#define VEL_T           250
#define TAMANO_DIR      3
  
#define TAMANO          4

Serial PC(PA_2,PA_3);//TX,RX

nRF24L01P RADIO(PB_5, PB_4, PB_3, PA_15, PA_12);    // MOSI, MISO, SCK, CSN, CE, IRQ----IRQ NO ESTA DEFINIDO NI CONECTADO, LA RECOMENDACION VIENE DADA POR LA LIBRERIA USADA

InterruptIn F_SUBIDA(PB_0);
InterruptIn F_BAJADA(PB_1);
InterruptIn F_SUBIR(PA_4);
InterruptIn F_BAJAR(PA_5);
InterruptIn F_APAGAR(PA_6);
DigitalOut DISPARO(PA_8);
Timeout TM_OUT;
Timer timer;

void CONF_GENER (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION_RX, int TAMAÑO_D, int TUBERIA);
void CONF_RADIO (unsigned long long DIRECCION_TX, int TAM_INFO);
void RECIBIR (void);
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF);
void FLANCOS (void);
void DESACTIVAR (void);
void ENVIARC (void);

void AUMENTAR (void);
void DISMINUIR (void);
void OFF_DIMM (void);

char RX_DATA [TAMANO];
char TX_DATA [TAMANO];
char CU_DATA [TAMANO];
char CONFIRMAR [TAMANO] = {'U','U','U','U'};
char CONFIRMAR_2 [TAMANO] = {'O','O','U','U'};
char CICLO_R = 0;

int CENTENAS = 0;
int DECENAS = 0;
int UNIDADES = 0;
int PORCENTAJE = 0;
unsigned int T_ALTO = 0;
unsigned int T_BAJO = 0;

char LETRA = 0;
int TEMP;

float TIEMPO = 0;

int main ()
{
    DISPARO = 0;
    RADIO.powerUp();                                                                             //Radio ENCENDIDO y en modo STANDBY
    CONF_GENER (RF_DIMMER, POTENCIA_T, VEL_T, DIR_DIMMER, TAMANO_DIR, NRF24L01P_PIPE_P0);                     //CONFIGURACION INICIAL radio
    CONF_RADIO (DIR_MAESTRO, TAMANO); 
    PC.printf("********************CONF_INICIAL********************\r\n");                              //DIRECCION INICIAL de Transmision
    PC.printf( "nRF24L01+ Frequency    : %d MHz\r\n",  RADIO.getRfFrequency() );
    PC.printf( "nRF24L01+ Output power : %d dBm\r\n",  RADIO.getRfOutputPower() );
    PC.printf( "nRF24L01+ Data Rate    : %d kbps\r\n", RADIO.getAirDataRate() );
    PC.printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", RADIO.getTxAddress() );
    PC.printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", RADIO.getRxAddress() );
    PC.printf("****************************************************\r\n");
    RADIO.setReceiveMode(); //Modo de RECEPCION ACTIVADO
    RADIO.enable();
    
    F_BAJADA.fall(&FLANCOS);
    F_SUBIDA.rise(&FLANCOS);
    F_SUBIR.rise(&AUMENTAR);
    F_BAJAR.rise(&DISMINUIR);
    F_APAGAR.rise(&OFF_DIMM);
    
    while (1)
    {
        if(RADIO.readable())
        {
            PC.printf("ALGO LLEGO\r\n");
            RECIBIR();     
            if((RX_DATA [3] == 'C') /* (PORCENTAJE >= 0 && PORCENTAJE <= 100)*/)
            {
                CICLO_R = 1;
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
                        PC.printf("%c",CONFIRMAR[i]);
                    }
                    PC.printf("\r\n");
                    //wait_ms (RETARDO);
                }
                RESPUESTA = 0;
                RADIO.setRfFrequency(RF_DIMMER);
                RADIO.setReceiveMode();
            }
            if(RX_DATA [0] == 'L' && RX_DATA [1] == 'G' && RX_DATA [2] == 'O' && RX_DATA [3] == 'F')
            {
                CICLO_R = 1;
                CU_DATA [0] = CU_DATA [1] = CU_DATA [2] = '0';
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
                    //wait_ms (RETARDO);
                }
                RESPUESTA = 0;
                RADIO.setRfFrequency(RF_DIMMER);
                RADIO.setReceiveMode();
            }
            for (int i = 0; i<4;i++)
            {
                RX_DATA[i] = ' ';
            }           
        }
        if (CICLO_R == 1)
        {   
            PC.printf("ENTRO A HACER CALCULOS \r\n");
            CICLO_R = 0;
            CENTENAS = (CU_DATA [0] - 48) * 100;
            DECENAS = (CU_DATA [1] - 48) * 10;
            UNIDADES = (CU_DATA [2] - 48);
            PORCENTAJE = CENTENAS + DECENAS + UNIDADES;
            PC.printf("%d %d %d %d \r\n",CENTENAS,DECENAS,UNIDADES, PORCENTAJE);
            if(PORCENTAJE >= 0 && PORCENTAJE <= 100)
            {
                T_ALTO = 83.33 * PORCENTAJE;
                T_BAJO = 8333-T_ALTO;
                PC.printf("EL PORCENTAJE ESTA ENTRE 0 Y 100**** TIEMPO EN ALTO: %d \r\n",T_ALTO);
            }    
        }
        
        
        TIEMPO = timer.read();
        if(TIEMPO >= 5.0)
        {
            timer.stop();
            timer.reset();
            ENVIARC();
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
        CU_DATA [i] = RX_DATA[i];
    }
    PC.printf("\r\n");
}
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF)
{
    RADIO.setTransferSize (ANCHO);
    RADIO.setTxAddress (DIRECCION,TAM_DIR);
    RADIO.setRfFrequency (RF);
}
void FLANCOS (void)
{
    if(PORCENTAJE == 0)
    {
        DISPARO = 0;
    }
    else if(PORCENTAJE == 100)
    {
        DISPARO = 1;
    }
    else 
    {
        DISPARO = 0;
        TM_OUT.attach_us(&DESACTIVAR,T_BAJO);
    }    
}
void DESACTIVAR (void)
{
     DISPARO = 1;
}
void ENVIARC (void)
{
    TX_DATA [0] = ((CENTENAS / 100) + 48);
    TX_DATA [1] = ((DECENAS / 10) + 48);
    TX_DATA [2] = UNIDADES + 48;
    TX_DATA [3] = 'C';
    //
    char RESP = 0;     //UBICAR 
    while(RESP == 0)
    {
        RADIO.setTransmitMode();
        RADIO.setRfFrequency(MI_FREQ_MST);
        RADIO.write(NRF24L01P_PIPE_P0, TX_DATA, TAMANO);
        PC.printf("RADIO ENVIO MENSAJE \r\n");
        RADIO.setRfFrequency (RF_DIMMER);
        RADIO.setReceiveMode();
        wait_ms (RETARDO);
        if(RADIO.readable())
        {
            PC.printf("RADIO TIENE ALGO PARA LEER \r\n");
            RECIBIR();
            if(RX_DATA[0] == 'C' && RX_DATA[1] == 'U' && RX_DATA[2] == 'R' && RX_DATA[3] == 'R')
            {
                RESP = 1;
                RADIO.setRfFrequency (RF_DIMMER);
                RADIO.setReceiveMode();
            }
        }
    }
}
void AUMENTAR (void)
{
    CICLO_R = 1;
    PORCENTAJE = PORCENTAJE + 10;
    if(PORCENTAJE < 0)
    {
        PORCENTAJE = 0;
        CU_DATA [0] = CU_DATA [1] = CU_DATA [2] = '0';
        timer.stop();
        timer.reset();
        timer.start();
    }
    else if(PORCENTAJE > 100)
    {
        PORCENTAJE = 100;
        CU_DATA [0] = '1';
        CU_DATA [1] = CU_DATA [2] = '0';
        timer.stop();
        timer.reset();
        timer.start();
    }
    else
    {
        CU_DATA [0] = (PORCENTAJE / 100) + 48;
        TEMP = PORCENTAJE % 100;
        CU_DATA [1] = (TEMP / 10) + 48;
        CU_DATA [2] = (TEMP % 10) + 48;
        timer.stop();
        timer.reset();
        timer.start();
    }
}
void DISMINUIR (void)
{
    CICLO_R = 1;
    PORCENTAJE = PORCENTAJE - 10;
    if(PORCENTAJE < 0)
    {
        PORCENTAJE = 0;
        CU_DATA [0] = CU_DATA [1] = CU_DATA [2] = '0';
        timer.stop();
        timer.reset();
        timer.start();
    }
    else if(PORCENTAJE > 100)
    {
        PORCENTAJE = 100;
        CU_DATA [0] = '1';
        CU_DATA [1] = CU_DATA [2] = '0';
        timer.stop();
        timer.reset();
        timer.start();
    }
    else
    {
        CU_DATA [0] = (PORCENTAJE / 100) + 48;
        TEMP = PORCENTAJE % 100;
        CU_DATA [1] = (TEMP / 10) + 48;
        CU_DATA [2] = (TEMP % 10) + 48;
        timer.stop();
        timer.reset();
        timer.start();
    }
}
void OFF_DIMM (void)
{
    if(PORCENTAJE != 0)
    {
        CICLO_R = 1;
        CU_DATA [0] = CU_DATA [1] = CU_DATA [2] = '0';
        timer.stop();
        timer.reset();
        timer.start();
    }
}