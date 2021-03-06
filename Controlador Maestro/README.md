# Diseño Controlador Maestro
### Tarjeta de circuito impreso módulo controlador maestro
![Módulo Controlador Maestro](Imagenes/FOTO_CONTROLADOR_MAESTRO.png)
### Foto conexión módulo controlador maestro y Raspberry Pi
![Controlador Maestro y Raspberry](Imagenes/FOTO_CONTROLADOR_RASPBERRY.png)

Si bien la SBC tiene la capacidad de enviar y recibir información desde y hacia Ubidots, es necesario implementar un radio NRF24L01 con un microcontrolador como etapa previa entre la Raspberry y los módulos de control. La Raspberry Pi cuenta con una interfaz de comunicación SPI, por lo que la conexión con el radio podría ser directa, sin embargo, para facilitar el desarrollo del controlador maestro, se decide [habilitar Puerto Serie de la Raspberry Pi](https://github.com/IoT-EBOT/IoT-Home-Sistem-with-STM32-and-Raspberry/tree/main/Controlador%20Maestro/Raspberry%20Pi/Configuracion%20Puerto%20Serie%20Raspberry%20Pi). De esta forma la etapa del microcontrolador y el radio conforman una etapa que puede modelarse como un buffer, el cual evitará pérdida de datos teniendo en cuenta que el sistema operativo que corre la Raspberry no es un sistema en tiempo real. Por otra parte, ya existen librerías que facilitan el desarrollo de la comunicación entre los radios y el microcontrolador.

### Diagrama esquemático Módulo Controlador Maestro
![ESQUEMÁTICO](Imagenes/ESQUEMATICO.png)

Por lo tanto, el módulo del controlador maestro cuenta con un conector GPIO macho de 40 pines (J1) que permite la comunicación serial entre la Raspberry pi y el microcontrolador, y además, proporciona la alimentación necesaria para los componentes de la PCB (microcontrolador y radio). Se adicionó un led que permite reconocer si el programa se está ejecutando en la Raspberry Pi, y unos filtros pasivos (condensadores) para intentar minimizar el ripple en la salidas de 3.3V y 5V.

## Código Python Controlador Mestro (Raspberry Pi 4)
```python
# -------------------------------------------------LIBRERIAS USADAS------------------------------------------

from ast import Try
import requests
import serial
import time
import os
import RPi.GPIO as gpio

gpio.setwarnings(False)
gpio.setmode(gpio.BOARD)
gpio.setup(36, gpio.OUT)


# -------------------------------------------------Configuración Puerto Serie------------------------------------------

SERIAL = serial.Serial('/dev/ttyAMA0', 9600, timeout=3.0, write_timeout=3.0)  # TTL se_port

# ---------------------------------------------Parametros  y variables en Ubidots-------------------------------------------

TOKEN = "__TOKEN_DE_UBIDOTS__"   # TOKEN de ubidots 
DEVICE_LABEL = "raspberry"                      # Nombre de dispositivo en Ubidots

CICLO_UTIL = "dimmer"                           # VARIABLE: Ciclo util para disparo de dimmer
INTERRUPTOR = "interruptor"                     # VARIABLE: Activar o desactivar bombillo

COMIDA = "dispensador_com"                      # VARIABLE: Control para activación de dispensador de comida
AGUA = "dispensador_agua"                       # VARIABLE: Control para activación de dispensador de agua

CORRIENTE = "sen_cor"                           # VARIABLE: Valor de corriente sensada
TOMA_CORRIENTE = "toma_cor"                     # VARIABLE: On/off para toma corriente

# -------------Variables locales usadas para almacenar los valores rescatados y a enviar a Ubidots----------

DIMMER = 0
INTERRUP = 0
DISP_COMIDA = 0
DISP_AGUA = 0
CURRENT = 0.0
TOMA = 0

# --------------------------VARIABLES TEMPORALES PARA ALMACENAR DATOS Y COMPARARLOS--------------------------

TEMP_CICLO = 0.0
TEMP_INTERRUPTOR = 0.0
TEMP_COMIDA = 0.0
TEMP_AGUA = 0.0
TEMP_CORRIENTE = 0.0
TEMP_TOMACORRIENTE = 0.0

# --------------------------VARIABLES DE CONTROL DE FLUJO PARA ATENDER ALERTAS DE Y PARA CON EL MICROCONTROLADOR--------------------------

ESPERAR_1 = 0
ESPERAR_2 = 0
ESPERAR_3 = 0
ESPERAR_4 = 0
ESPERAR_5 = 0
ESPERAR_6 = 0

# -----------FUNCION PARA EL ENVIO DE DATOS A UBIDOTS-----------

def ENVIAR_DATO(VARIABLE, VALOR):
    #
    # CONTRUIR DICCIONARIO TEMPORAL PARA EL ENVíO DE ENFORMACIÓN
    payload = {VARIABLE: VALOR}

    # Creates the headers for the HTTP requests
    url = "http://industrial.api.ubidots.com"
    url = "{}/api/v1.6/devices/{}".format(url, DEVICE_LABEL)
    headers = {"X-Auth-Token": TOKEN, "Content-Type": "application/json"}

    # Makes the HTTP requests
    status = 400
    attempts = 0
    while status >= 400 and attempts <= 5:
        req = requests.post(url=url, headers=headers, json=payload)
        status = req.status_code
        attempts += 1
        time.sleep(1)

    # Processes results
    print(req.status_code, req.json())
    if status >= 400:
        print("[ERROR] No se pudo enviar información despues de realizar 5 intentos")
        return False

    print("[INFO] Variable actualizada")
    return True

#

# -----------FUNCION PARA EL RESCATE DE DATOS DESDE UBIDOTS-----------

def OBTENER_DATO(device, variable):
    #    
    try:
        url = "http://industrial.api.ubidots.com/"
        url = url + \
            "api/v1.6/devices/{0}/{1}/".format(device, variable)
        headers = {"X-Auth-Token": TOKEN, "Content-Type": "application/json"}
        req = requests.get(url=url, headers=headers)
        return req.json()['last_value']['value']
    except:
        pass

 

# -----------FUNCION PARA ATENDER ALERTAS INTERNAS Y EXTERNAS DERIVADAS DE LA COMUNICACION CON EL MICROCONTROLADOR-----------

def LEER_MICRO():
    #
    global DIMMER
    global INTERRUP
    global DISP_COMIDA
    global DISP_AGUA 
    global CURRENT 
    global TOMA

    global TEMP_CICLO 
    global TEMP_INTERRUPTOR
    global TEMP_COMIDA
    global TEMP_AGUA
    global TEMP_CORRIENTE 
    global TEMP_TOMACORRIENTE 

    global ESPERAR_1 
    global ESPERAR_2 
    global ESPERAR_3 
    global ESPERAR_4 
    global ESPERAR_5 
    global ESPERAR_6 

    if SERIAL.readable() == True:
        LEER = SERIAL.read()
        if LEER != bytes(''.encode()):
            print(LEER)
            # ------CAMARA--------
            if LEER == bytes('P'.encode()):
                print('PUERTA SE ABRIO')
                temp = 'T'
                SERIAL.write(temp.encode())
                os.system('python3 CAPTURA_CAMARA.py')  #Llama programa de captura de video y envio por correo electronico                
                print('GRABACION Y ENVIO COMPLETADO')
            #-------------------------------------ALERTAS EXTERNAS (ALERTAS PROBOCADAS POR MENSAJES ENVIADOS DESDE LOS MÓDULOS DE CONTROL)------------------------------------
            if LEER == bytes('C'.encode()):
                print('EL MAESTRO TIENE UN CILO PARA ENVIARME')
                temp = 'C'
                SERIAL.write(temp.encode())
                CICLO = 0
                while CICLO == 0:
                    if SERIAL.readable() == True:
                        SERIAL.flush()
                        DUTY = SERIAL.readline()
                        if DUTY != bytes(''.encode()):
                            print('El ciclo recibido es: ' + str(DUTY) + '    ' + str(type(DUTY)))
                            CICLO = 1
                            temp = 'c'
                            SERIAL.write(temp.encode())
                            DUTY = int(DUTY)
                            ENVIAR_DATO(CICLO_UTIL, DUTY)
                            if DUTY != 0:
                                ENVIAR_DATO(INTERRUPTOR, 1.0)
                                INTERRUP = TEMP_INTERRUPTOR = 1.0
                                DIMMER = TEMP_CICLO = float(DUTY)
                            elif DUTY == 0:
                                ENVIAR_DATO(INTERRUPTOR, 0.0)
                                INTERRUP = TEMP_INTERRUPTOR = 0.0
                                DIMMER = TEMP_CICLO = float(DUTY)
            # ------SENSOR--------
            if LEER == bytes('Z'.encode()):
                print('EL MAESTRO TIENE UNA CORRIENTE PARA ENVIAR')
                temp = 'Z'
                SERIAL.write(temp.encode())
                COR = 0
                while COR == 0:
                    if SERIAL.readable() == True:
                        SERIAL.flush()
                        corriente = SERIAL.readline()
                        if corriente != bytes(''.encode()):
                            print('LA CORRIENTE RECIBIDA ES : ' + str(corriente) + '   ' + str(type(corriente)))
                            COR = 1
                            temp = 'z'
                            SERIAL.write(temp.encode())
                            COR_ENV = float(corriente)
                            COR_ENV = COR_ENV / 100
                            print('LA CORRIENTE EN MEMORIA ES: ' + str(COR_ENV) + '   ' + str(type(COR_ENV)))
                            ENVIAR_DATO(CORRIENTE, COR_ENV)
                        
            if LEER == bytes('Q'.encode()):
                temp = 'Q'
                SERIAL.write(temp.encode())
                ENVIAR_DATO(TOMA_CORRIENTE, 1.0)
                TOMA = TEMP_TOMACORRIENTE = 1.0
            #-------------------------------------ALERTAS ESPERADAS (ALERTAS GENERADAS EN RESPUESTAS A PETICIONES GENERADAS POR RASPBERRY)------------------------------------
            
            if LEER == bytes('U'.encode()):  # EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
                time.sleep(0.5)
                print(str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                temp = (str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                SERIAL.write(temp.encode())
                RESPUESTA_1 = 0
                while RESPUESTA_1 == 0:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            if LEER == bytes('R'.encode()):
                                print('DIMMER RECIBIO Y SETEO SU CICLO UTIL')
                                RESPUESTA_1 = 1
                                ESPERAR_1 = 0

            if LEER == bytes('I'.encode()):  # EL MICRO YA ENVIO Y RECIBIO LA CONFIRMACION DE LA DESACTIVACION DEL BOMBILLO
                ESPERAR_2 = 0

            if LEER == bytes('G'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                ENVIAR_DATO(COMIDA, 0.0)
                DISP_COMIDA = TEMP_COMIDA = 0.0;
                ESPERAR_3 = 0

            if LEER == bytes('H'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                ENVIAR_DATO(AGUA, 0.0)
                DISP_AGUA = TEMP_AGUA = 0.0;
                ESPERAR_4 = 0

            if LEER == bytes('F'.encode()):
                ESPERAR_5 = 0

            if LEER == bytes('N'.encode()):
                ESPERAR_6 = 0

name = 'main'

if name == 'main':

    #-----------------------------------------------------------------------------RECUPERAR DATOS DESDE UBIDOTS-----------------------------------------------------------------------------
    gpio.output(36, True)
    
    TEMP_CICLO = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
    TEMP_INTERRUPTOR = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
    TEMP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
    TEMP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
    TEMP_CORRIENTE = OBTENER_DATO(DEVICE_LABEL, CORRIENTE)
    TEMP_TOMACORRIENTE = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)

    print('ESTADO INICIAL CICLO UTIL: ' + str(TEMP_CICLO))
    print('ESTADO INICIAL INTERRUPTO: ' + str(TEMP_INTERRUPTOR))
    print('ESTADO INICIAL DIS_COMIDA: ' + str(TEMP_COMIDA))
    print('ESTADO INICIAL DISPE_AGUA: ' + str(TEMP_AGUA))
    print('ESTADO INICIAL  CORRIENTE: ' + str(TEMP_CORRIENTE))
    print('ESTADO INICIAL TOMA_CORRI: ' + str(TEMP_TOMACORRIENTE))

    while (True):

        try:
            #-------------------------------------------------------------------------RECUPERAR DATOS DESDE UBIDOTS--------------------------------------------------------

            DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
            INTERRUP = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
            DISP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
            DISP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
            TOMA = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)
            
            print('DIMMER EN UBIDOTS:   ' + str(DIMMER) +      '  DIMMER EN MEMORIA:   ' + str(TEMP_CICLO))
            print('INTERRUP EN UBIDOTS: ' + str(INTERRUP) +    '  INTERRUP EN MEMORIA: ' + str(TEMP_INTERRUPTOR))
            print('COMIDA EN UBIDOTS:   ' + str(DISP_COMIDA) + '  COMIDA EN MEMORIA:   ' + str(TEMP_COMIDA))
            print('AGUA EN UBIDOTS:     ' + str(DISP_AGUA) +   '  AGUA EN MEMORIA:     ' + str(TEMP_AGUA))
            print('TOMA COR EN UBIDOTS: ' + str(TOMA) +        '  TOMA EN MEMORIA:     ' + str(TEMP_TOMACORRIENTE))

            #----------------------------------------REVISAR SI EL MICROCONTROLADOR TIENE ALGUNA ALERTA PARA RASPBERRY Y/O UBIDOTS----------------------------------------

            LEER_MICRO ()

            #--------------COMPARAR LOS ULTIMOS DATOS LEIDOS EN UBIDOTS CON LOS ALMACENADOS EN LOCAL PARA DETERMINAR SI EL USUARIO QUIERE REALIZAR UNA ACCION--------------

            if INTERRUP != TEMP_INTERRUPTOR:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
                print('El interruptor cambio')
                TEMP_INTERRUPTOR = INTERRUP
                if INTERRUP == 1.0:
                    print('Interruptor Activado')
                    TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
                    LEER_MICRO()
                    temp = 'B'
                    SERIAL.write(temp.encode())
                    ESPERAR_1 = 1
                    while ESPERAR_1 == 1:
                        LEER_MICRO()

                if INTERRUP == 0.0:
                    ENVIAR_DATO(CICLO_UTIL, 0.0)
                    TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
                    print('Interruptor Desactivado')
                    LEER_MICRO()
                    temp = 'I'
                    SERIAL.write(temp.encode())
                    ESPERAR_2 = 1
                    while ESPERAR_2 == 1:
                        LEER_MICRO()
                                
                                
            if DIMMER != TEMP_CICLO and INTERRUP == 1.0:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
                print('El dimmer cambio')
                TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
                LEER_MICRO()
                temp = 'A'
                SERIAL.write(temp.encode())
                ESPERAR_1 = 1
                while ESPERAR_1 == 1:
                    LEER_MICRO()


            if DISP_COMIDA != TEMP_COMIDA:  
                print('La comida cambio')
                TEMP_COMIDA = DISP_COMIDA
                LEER_MICRO()
                temp = 'D'
                SERIAL.write(temp.encode())
                ESPERAR_3 = 1
                while ESPERAR_3 == 1:
                    LEER_MICRO()


            if DISP_AGUA != TEMP_AGUA:  
                print('El agua cambio')
                TEMP_AGUA = DISP_AGUA
                LEER_MICRO()
                temp = 'E'
                SERIAL.write(temp.encode())
                ESPERAR_4 = 1
                while ESPERAR_4 == 1:
                    LEER_MICRO()
                            

            if TOMA != TEMP_TOMACORRIENTE:
                print('La toma cambio')
                TEMP_TOMACORRIENTE = TOMA

                if TOMA == 1:
                    LEER_MICRO()
                    temp = 'F'
                    SERIAL.write(temp.encode())
                    ESPERAR_5 = 1
                    while ESPERAR_5 == 1:
                        LEER_MICRO()
                                

                elif TOMA == 0:
                    LEER_MICRO()
                    temp = 'G'
                    SERIAL.write(temp.encode())
                    ESPERAR_6 = 1
                    while ESPERAR_6 == 1:
                        LEER_MICRO()
                                        
            print('Ejecutando...')

        except Exception as ERROR_M:

            print('ERROR DETECTADO')
            gpio.output(36, False)

            print("EL ERROR DETECTADO FUE = "   + str(ERROR_M))

```

![DIAGRAMA DE FLUJO MAESTRO PYTHON](Imagenes/D_F_PYTHON.png)

## Código Microcontroaldor Maestro

```c
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

//------------------------------------------------------------------------ LIBRERIAS ---------------------------------------------------------------

#include "mbed.h"
#include "nRF24L01P.h"

//------------------------------------------------------------------------ DEFINICIONES ---------------------------------------------------------------

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

//------------------------------------------------------------------------ CREACION DE OBJETOS ---------------------------------------------------------------

nRF24L01P RADIO(PB_5, PB_4, PB_3, PA_15, PA_12);    // MOSI, MISO, SCK, CSN, CE, IRQ
Serial RASPBERRY(PA_9,PA_10);  //TX,RX
Serial PC(PA_2,PA_3);  //TX,RX
DigitalOut ON(PC_13);
Timeout ESPERAR_T_CAMARA;
//------------------------------------------------------------------------ DEFINICION DE FUNCIONES ---------------------------------------------------------------

void CONF_INIC (int FRECUENCIA, int POTENCIA, int VELOCIDAD, unsigned long long DIRECCION, int ANCHO, int TUBERIA);
void ESTADO_I (void);
void PREPARAR (int ANCHO, unsigned long long DIRECCION, int TAM_DIR, int RF);
void RECIBIR (void);
void ENVIAR_CICLO (void);
void LEER_RASPBERRY (void);
void ENVIAR_ALERTAS (void);
void HABILITAR_RECP_CAMARA (void);

//------------------------------------------------------------------------ VARIABLES GLOBALES ---------------------------------------------------------------

char DATA_TX [TAM_TX];
char RX_DATA [TAM_TX];

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
char TOMA_IR [TAM_TX] = {'T','E','I','R'};
char CAMARA_OK [TAM_TX] = {'S','G','Y','E'};

int CENTENAS = 0;
int DECENAS = 0;
int UNIDADES = 0;
int PORCENTAJE = 101;
int PORCENTAJE_T = 0;

int DECIMAL  = 0;
int UNIDAD_1 = 0;
int UNIDAD_2 = 0;
float VALOR_COR = 100.0;
float VALOR_COR_T = 0.0;

char FG_COMIDA = 0;
char FG_AGUA = 0;
char TOMA_ON = 0;
char TOMA_OFF = 0;

char LETRA = ' ';

char E_CICLO = 1;
char E_COR = 1; 
char E_CAMARA = 1;
char E_COR_I = 1;

char PERMISO_CAMARA = 1;

char ERF_CICLO = 1;
char ERF_INTERRUP_OFF = 1;
char ERF_COMIDA = 1;
char ERF_AGUA = 1;
char ERF_TOMA_ON = 1;
char ERF_TOMA_OFF = 1;

char CRF_CAMARA = 1;
char CRF_DIMMER = 1;
char CRF_CORRIENTE = 1;

char ERF_CICLO_C = 0;
char ERF_INTERRUP_OFF_C = 0;
char ERF_COMIDA_C = 0;
char ERF_AGUA_C = 0;
char ERF_TOMA_ON_C = 0;
char ERF_TOMA_OFF_C = 0;

char TOMA_C = 0;

int main (void)
{
    //------------------------------------------------------------------------ CONFIGURACION DE RADIO ---------------------------------------------------------------
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
        //-------------------------------------------------------------- EVALUAR SI EXISTEN ALERTAS POR ATENDER DESDE RASPBERRY ------------------------------------------------------

        LEER_RASPBERRY ();
        
        //---------------------------------------------------------------------- ATENDER ALERTAS ENVIADAS POR RASPBERRY --------------------------------------------------------------

        if(FG_DIMMER == 1)              //OPTIMIZAR
        {
            PC.printf("FUNCION CAMBIAR DIMMER\r\n");
            ENVIAR_CICLO();                                 //LE SOLICITA A LA RASPBERRY EL VALOR DEL DIMMER, Y LO ENVIA AL ESCLAVO CORRESPONDIENTE
            FG_DIMMER = 0;    
        }
        if(FG_INTERRUPTOR_ON == 1)      //OPTIMIZAR
        {
            PC.printf("FUNCION ENCENDER INTERRUPTOR\r\n");
            ENVIAR_CICLO();                                 //LE SOLICITA A LA RASPBERRY EL VALOR DEL DIMMER, Y LO ENVIA AL ESCLAVO CORRESPONDIENTE
            FG_INTERRUPTOR_ON = 0;
        }
        if(FG_INTERRUPTOR_OFF == 1)
        {
            PC.printf("FUNCION APAGAR INTERRUPTOR\r\n");
            RASPBERRY.putc('I');                        //RESPONDER QUE SE RECIBIO LA ORDEN
            PC.printf("RESPONDIO CON UNA I \r\n");
            ERF_INTERRUP_OFF = 0;
            ENVIAR_ALERTAS();
            FG_INTERRUPTOR_OFF = 0;
        }
        if(FG_COMIDA == 1)
        {
            PC.printf("FUNCION DISPENSAR COMIDA\r\n");
            RASPBERRY.putc('G');                        //RESPONDER QUE SE RECIBIO LA ORDEN
            PC.printf("RESPONDIO CON UNA G \r\n");
            ERF_COMIDA = 0;
            ENVIAR_ALERTAS();
            FG_COMIDA = 0;
        }
        if(FG_AGUA == 1)
        {
            PC.printf("FUNCION DISPENSAR AGUA\r\n");
            RASPBERRY.putc('H');                        //RESPONDER QUE SE RECIBIO LA ORDEN
            PC.printf("RESPONDIO CON UNA H \r\n");
            ERF_AGUA = 0;
            ENVIAR_ALERTAS();
            FG_AGUA = 0;
        }
        if(TOMA_ON == 1)
        {
            PC.printf("FUNCION ENCENDER TOMA\r\n");
            RASPBERRY.putc('F');                        //RESPONDER QUE SE RECIBIO LA ORDEN
            PC.printf("RESPONDIO CON UNA F \r\n");
            ERF_TOMA_ON = 0;
            ENVIAR_ALERTAS();
            TOMA_ON = 0;
            TOMA_C = 1;
        }    
        if(TOMA_OFF == 1)
        {
            PC.printf("FUNCION APAGAR TOMA\r\n");
            RASPBERRY.putc('N');                        //RESPONDER QUE SE RECIBIO LA ORDEN
            PC.printf("RESPONDIO CON UNA N \r\n");
            ERF_TOMA_OFF = 0;
            ENVIAR_ALERTAS();
            TOMA_OFF = 0;
            TOMA_C = 0;
        } 

        //---------------------------------------------------------------------- REVISAR SI EL RADIO RECIBIÓ ALERTAS --------------------------------------------------------------

        if(RADIO.readable())
        {           
            PC.printf("ALGO LLEGO\r\n");
            RECIBIR();
            //------------------------------------------------------------ ALERTAS ESPERADAS DESDE CAMARA, DIMMER Y SENSOR DE CORRIENTE ------------------------------------------------
            if(RX_DATA[0] == 'S' && RX_DATA[1] == 'P' && RX_DATA[2] == 'A' && RX_DATA[3] == 'D')
            {
                if(PERMISO_CAMARA == 1)
                {
                    PC.printf("LA PUERTA SE ABRIO\r\n"); 
                    RASPBERRY.putc('P');  
                    CRF_CAMARA = 0;
                    ENVIAR_ALERTAS();
                    PERMISO_CAMARA = 0;
                    ESPERAR_T_CAMARA.attach(&HABILITAR_RECP_CAMARA,60);
                }
                else if(PERMISO_CAMARA == 0)
                {
                    PC.printf("LA PUERTA SE ABRIO\r\n");
                    CRF_CAMARA = 0;
                }
            }
                   
            if(RX_DATA [3] == 'C')
            {
                CRF_DIMMER = 0;
                ENVIAR_ALERTAS();

                CENTENAS = (RX_DATA [0] - 48) * 100;
                DECENAS = (RX_DATA [1] - 48) * 10;
                UNIDADES = (RX_DATA [2] - 48);
                PORCENTAJE_T = CENTENAS + DECENAS + UNIDADES;
               
               if(PORCENTAJE_T != PORCENTAJE && PORCENTAJE_T >= 0 && PORCENTAJE_T <= 100)
               {
                    PORCENTAJE = PORCENTAJE_T;
                    RASPBERRY.putc('C');
                    E_CICLO = 0;
                    while (E_CICLO == 0)
                    {
                        LEER_RASPBERRY();
                    }
                    LEER_RASPBERRY();
               }
               else
               {
                   PC.printf("%d %d %d %d \r\n",CENTENAS,DECENAS,UNIDADES, PORCENTAJE);
               }
                
            }
            if(RX_DATA [3] == 'Z')
            {
                CRF_CORRIENTE = 0;
                ENVIAR_ALERTAS();

                DECIMAL  = (RX_DATA [0] - 48) * 100;
                UNIDAD_1 = (RX_DATA [1] - 48) *10;
                UNIDAD_2 = (RX_DATA [2] - 48);  
                
                VALOR_COR_T = DECIMAL + UNIDAD_1 + UNIDAD_2;
                                                               
                if(VALOR_COR_T != VALOR_COR && VALOR_COR_T >= 0.0 && VALOR_COR_T <= 500.0)
                {
                    VALOR_COR = VALOR_COR_T;
                    RASPBERRY.putc('Z');
                    E_COR = 0;                                                  
                    while (E_COR == 0)
                    {
                        LEER_RASPBERRY();
                    }
                    LEER_RASPBERRY();
                }
                else
                {
                    PC.printf("%f,%f\r\n",VALOR_COR_T,VALOR_COR); 
                }
            }
            if(RX_DATA[0] == 'T' && RX_DATA[1] == 'I' && RX_DATA[2] == 'O' && RX_DATA[3] == 'F')
            {
                if (TOMA_C == 0)
                {
                    RASPBERRY.putc('Q');
                    E_COR_I = 0;
                    TOMA_C = 1;
                }
                else
                {
                    E_COR_I = 0;
                }
            }
            //------------------------------------------------------------ CONFIRMACIONES DE RECEPCION DE ALERTAS ------------------------------------------------

            if(RX_DATA[0] == 'U' && RX_DATA[1] == 'U' && RX_DATA[2] == 'U' && RX_DATA[3] == 'U')
            {
                ERF_CICLO = 1;
                ERF_CICLO_C = 0;
            }

            if(RX_DATA[0] == 'O' && RX_DATA[1] == 'O' && RX_DATA[2] == 'U' && RX_DATA[3] == 'U')
            {
                ERF_INTERRUP_OFF = 1;
                ERF_INTERRUP_OFF_C = 0;
            }    

            if(RX_DATA[0] == 'D' && RX_DATA[1] == 'F' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
            {
                ERF_COMIDA = 1;
                ERF_COMIDA_C = 0;
            }    

            if(RX_DATA[0] == 'D' && RX_DATA[1] == 'A' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
            {
                ERF_AGUA = 1;
                ERF_AGUA_C = 0;
            }    

            if(RX_DATA[0] == 'T' && RX_DATA[1] == 'M' && RX_DATA[2] == 'O' && RX_DATA[3] == 'N')
            {
                ERF_TOMA_ON = 1;
                ERF_TOMA_ON_C = 0;
            }    

            if(RX_DATA[0] == 'T' && RX_DATA[1] == 'M' && RX_DATA[2] == 'O' && RX_DATA[3] == 'F')
            {
                ERF_TOMA_OFF = 1;
                ERF_TOMA_OFF_C = 0;
            }    

        }

        //--------------------------- ENVIAR ALERTAS SI HAY CONFIRMACIONES PENDIENTES O SE RECIBIÓ NUEVAMENTE ALERTAS DESDE CAMARA, SENSOR O DIMMER ---------------------------

        ENVIAR_ALERTAS ();     
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
    char ESPERANDO = 1;     
    char POS = 0;           
    
    RASPBERRY.putc('U');                        //RESPONDER QUE SE ESTA LISTO PARA RECIBIR EL ARREGLO
    PC.printf("RESPONDIO CON UNA U \r\n");
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
    RASPBERRY.putc('R');
    PC.printf("RESPONDIO CON UNA R \r\n");  

    ERF_CICLO = 0;
    ENVIAR_ALERTAS();
}

void LEER_RASPBERRY (void)
{
    char CONFIRMAR_1 = 1;
    char CONFIRMAR_2 = 1;

    if(RASPBERRY.readable() == 1)               //EVALUA SI HAY ALGUNA ACCION POR REALIZAR SEGUN LO QUE LA RASPBERRY ENVIA
    {
        PC.printf("Hay algo para leer\r\n");
        OPCION = RASPBERRY.getc();
        switch (OPCION)                         //DEFINE CUÁL ACCION SE REQUIERE REALIZAR
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
            case 'I':
            {
                FG_INTERRUPTOR_OFF = 1;
                PC.printf("I: DIMMER EN OFF\r\n");
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
            //------------------------------------------------------------------RESPUESTAS A ALERTAS GENERADAS DESDE EL MICRO------------------------------------------------
            case 'C':
            {
                RASPBERRY.printf("%d",PORCENTAJE);
                PC.printf("%d",PORCENTAJE);
                CONFIRMAR_1 = 0;
                while (CONFIRMAR_1 == 0)
                {
                    if(RASPBERRY.readable() == 1)
                    {
                        LETRA = RASPBERRY.getc();
                        if(LETRA == 'c')
                        {
                            CONFIRMAR_1 = 1;
                            E_CICLO = 1;
                        }
                    }
                }
                break;
            }

            case 'Z':
            {
                RASPBERRY.printf("%f",VALOR_COR);
                CONFIRMAR_2 = 0;
                while (CONFIRMAR_2 == 0)
                {
                    if(RASPBERRY.readable() == 1)
                    {
                        LETRA = RASPBERRY.getc();
                        if(LETRA == 'z')
                        {
                            CONFIRMAR_2 = 1;
                            E_COR = 1;
                        }
                    }
                }
                break;
            }

            case 'T':
            {
                E_CAMARA = 1;
                break;
            }
            case 'Q':
            {
                E_COR_I = 1;
                break;
            }
        }    
    }
}

void ENVIAR_ALERTAS (void)
{
    //--------------------------------------------------------------------ENVIAR CICLO ÚTIL AL DIMMER------------------------------------------------------------
    if (ERF_CICLO == 0 && ERF_CICLO_C <= 20)
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
                ERF_CICLO = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_CICLO_C = 0;
            }
        }
        ERF_CICLO_C = ERF_CICLO_C + 1;
        PC.printf("CONTADOR = %i \r\n",ERF_CICLO_C);
    }
    else if(ERF_CICLO == 0 && ERF_CICLO_C > 20)
    {
        ERF_CICLO_C = 0;
        ERF_CICLO = 1;
    }
    if(ERF_INTERRUP_OFF == 0 && ERF_INTERRUP_OFF_C <= 20)
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
                ERF_INTERRUP_OFF = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_INTERRUP_OFF_C = 0;
            }    
        }
        ERF_INTERRUP_OFF_C = ERF_INTERRUP_OFF_C + 1;
        PC.printf("CONTADOR = %i \r\n",ERF_INTERRUP_OFF_C);
    }
    else if(ERF_INTERRUP_OFF == 0 && ERF_INTERRUP_OFF_C > 20)
    {
        ERF_INTERRUP_OFF_C = 0;
        ERF_INTERRUP_OFF = 1;
    }
    if(ERF_COMIDA == 0 && ERF_COMIDA_C <= 20)
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
                ERF_COMIDA = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_COMIDA_C = 0;
            }    
        }
        ERF_COMIDA_C = ERF_COMIDA_C + 1;    
        PC.printf("CONTADOR = %i \r\n",ERF_COMIDA_C);
    }
    else if(ERF_COMIDA == 0 && ERF_COMIDA_C > 20)
    {
        ERF_COMIDA_C = 0;
        ERF_COMIDA = 1;
    }
    if(ERF_AGUA == 0 && ERF_AGUA_C <= 20)
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
                ERF_AGUA = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_AGUA_C = 0;
            }    
        } 
        ERF_AGUA_C = ERF_AGUA_C + 1;   
        PC.printf("CONTADOR = %i \r\n",ERF_COMIDA_C);
    }
    else if(ERF_AGUA == 0 && ERF_AGUA_C > 20)
    {
        ERF_AGUA_C = 0;
        ERF_AGUA = 1;
    }
    if (ERF_TOMA_ON == 0 && ERF_TOMA_ON_C <= 20)
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
                ERF_TOMA_ON = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_TOMA_ON_C = 0;
            }    
        } 
        ERF_TOMA_ON_C = ERF_TOMA_ON_C + 1;
        PC.printf("CONTADOR = %i \r\n",ERF_TOMA_ON_C);
    }
    else if (ERF_TOMA_ON == 0 && ERF_TOMA_ON_C > 20)
    {
        ERF_TOMA_ON_C = 0;
        ERF_TOMA_ON = 1;
    }
    if(ERF_TOMA_OFF == 0 && ERF_TOMA_OFF_C <= 20)
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
                ERF_TOMA_OFF = 1;
                RADIO.setRfFrequency (MI_FREQ);
                RADIO.setReceiveMode();
                ERF_TOMA_OFF_C = 0;
            }    
        }
        ERF_TOMA_OFF_C = ERF_TOMA_OFF_C + 1;
        PC.printf("CONTADOR = %i \r\n",ERF_TOMA_OFF_C);
    }
    else if(ERF_TOMA_OFF == 0 && ERF_TOMA_OFF_C > 20)
    {
        ERF_TOMA_OFF_C = 0;
        ERF_TOMA_OFF = 1;
    }
    if(CRF_CAMARA == 0) //AQUI HAY QUE CONDICIONAR EL TEMA DE QUE SI LA ALAERTA SE RECIBIÓ HACE MENOS DE 1 MINUTO IGNORE LAS ALERTAS QUE LLEGUEN
    {
        RADIO.setTransmitMode();
        PREPARAR(TAM_TX, DIR_PUERTA, TAM_DIRECCIONES, RF_PUERTA);
        RADIO.write(NRF24L01P_PIPE_P0, CAMARA_OK, TAM_TX);
        PC.printf("SE LE RESPONDIO QUE LA ALERTA SE RECIBIO \r\n"); 
        RADIO.setRfFrequency(MI_FREQ);
        RADIO.setReceiveMode();
        wait_ms(250);                                                      
        E_CAMARA = 0;
        /*while (E_CAMARA == 0)
        {
            LEER_RASPBERRY();
        }*/
        CRF_CAMARA = 1;
    }
    if(CRF_DIMMER == 0)
    {
        RADIO.setTransmitMode();
        PREPARAR(TAM_TX, DIR_DIMMER, TAM_DIRECCIONES, RF_DIMMER);
        RADIO.write(NRF24L01P_PIPE_P0, CONF_CIC, TAM_TX);
        PC.printf("SE RESPONDIO \r\n");
        RADIO.setRfFrequency(MI_FREQ);
        RADIO.setReceiveMode();
        wait_ms(250);
        for(int i = 0; i<TAM_TX; i++)
        {
            PC.printf("%c",CONF_CIC[i]);
        }
        PC.printf("\r\n");
        CRF_DIMMER = 1;
    }
    if(CRF_CORRIENTE == 0)
    {
        RADIO.setTransmitMode();
        PREPARAR(TAM_TX, DIR_TOMA, TAM_DIRECCIONES, RF_TOMA);
        RADIO.write(NRF24L01P_PIPE_P0, CONF_COR, TAM_TX);
        PC.printf("SE RESPONDIO \r\n");
        RADIO.setRfFrequency(MI_FREQ);
        RADIO.setReceiveMode();
        wait_ms(250);
        for(int i = 0; i<TAM_TX; i++)
        {
            PC.printf("%c",CONF_COR[i]);
        }
        PC.printf("\r\n");
        CRF_CORRIENTE = 1;
    }
    if (E_COR_I == 0)
    {
        RADIO.setTransmitMode();
        PREPARAR(TAM_TX, DIR_TOMA, TAM_DIRECCIONES, RF_TOMA);
        RADIO.write(NRF24L01P_PIPE_P0, TOMA_IR, TAM_TX);
        PC.printf("SE RESPONDIO \r\n");
        RADIO.setRfFrequency(MI_FREQ);
        RADIO.setReceiveMode();
        wait_ms(250);
        PC.printf("%s \r\n",TOMA_IR);
        E_COR_I = 1;
    }
}

void HABILITAR_RECP_CAMARA (void)
{
    PERMISO_CAMARA = 1;
}
```

![DIAGRAMA DE FLUJO MAESTRO C](Imagenes/D_F_C.png)
