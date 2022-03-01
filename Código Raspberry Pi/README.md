# Diseño Controlador Maestro

Si bien la SBC tiene la capacidad de enviar y recibir información desde Ubidots, es necesario implementar un radio NRF24L01 a este dispositivo, pues dicho radio es el medio de comunicación de la presente solución. La Raspberry pi cuenta con una interfaz de comunicación SPI, por lo que la conexión con el radio podría ser directa, sin embargo, para facilitar el desarrollo del controlador maestro se decide habilitar el puerto serie de la Raspberry pi para comunicación Serial comandada por un programa Python (revisar [Habilitar Puerto Serie Raspberryt Pi](https://pages.github.com/)), y así tener un microcontrolador encargado de enviar y recibir la información del sistema mediante el radio, y la SBC opera como puente entre el sistema e Internet.

![ESQUEMÁTICO](Imagenes/ESQUEMATICO.png)

Por lo tanto, el módulo del maestro transceptor cuenta con un conector GPIO macho de 40 pines (J1) que permite la comunicación serial entre la Raspberry pi y el microcontrolador, y, además, proporciona la alimentación necesaria para los componentes de la PCB (microcontrolador y radio). Se adicionó un led que permite reconocer si el programa se está ejecutando en la Raspberry Pi.

## Código Python Controlador Mestro (Raspberry Pi 4)
```python
# -------------------------------------------------LIBRERIAS USADAS------------------------------------------

import requests
import serial
import time

import cv2
import numpy as np
import smtplib

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.encoders import encode_base64

# -------------------------------------------------ConfiguraciOn Puerto Serie------------------------------------------

SERIAL = serial.Serial('/dev/ttyAMA0', 9600, timeout=3.0, write_timeout=3.0)  # TTL se_port

# ---------------------------------------------Parametros  y variables en Ubidots-------------------------------------------

TOKEN = "BBFF-1JPdBOfYF2swcxLWhBIwkKDZLmeBti"  # TOKEN de ubidots 
DEVICE_LABEL = "raspberry"  # Nombre de dispositivo en Ubidots

CICLO_UTIL = "dimmer"  # VARIABLE: Ciclo util para disparo de dimmer
INTERRUPTOR = "interruptor"  # VARIABLE: Activar o desactivar bombillo

COMIDA = "dispensador_com"  # VARIABLE: Control para activación de dispensador de comida
AGUA = "dispensador_agua"  # VARIABLE: Control para activación de dispensador de agua

CORRIENTE = "sen_cor"  # VARIABLE: Valor de corriente sensada
TOMA_CORRIENTE = "toma_cor"  # VARIABLE: On/off para toma corriente

DELAY = 1  # Delay en segundos

# -------------Variables locales usadas para almacenar lo valores rescatados y a enviar a Ubidots----------

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

# ---Parámetros y variables para control de cámara____Datos correo electrónico de órigen del video y los correos que lo reciben---

# EMPIEZA_CONTEO = time.time()   # conteo para finalizar grabacion
TEMPORIZADOR_GRABACION = 20  # PIERDE TRES SEGUNDOS DE GRABACION
DETENER = False  # PERMISO PARA DETENER GRABACION AUTOMATICAMENTE
CAMARA = False  # PERMISO PARA INICIAR GRABACION LO MANDA MAESTRO CON UNA 'P'

# --------------DATOS GRABACION--------------

URL = 'rtsp://192.168.0.100/live/ch00_1'        # IP estática de la cámara, obtenido desde ONVIF
CAPTURA = cv2.VideoCapture(URL)
FPS = 10
ANCHO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_WIDTH))
ALTO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_HEIGHT))
FORMATO = cv2.VideoWriter_fourcc('X','2','6','4')
VIDEO_SALIDA = cv2.VideoWriter('GRABACION.avi', FORMATO, FPS, (ANCHO,ALTO))

# -----------DATOS ENVIO CORREO------------------

CORREO_DESTINO = 'dgomezbernal24@gmail.com,cristiancobos2002@gmail.com'
CORREO_MAESTRO = 'iot.e.bot21@gmail.com'
PASSWORD = 'E-BOT2021'
smtp_server = 'smtp.gmail.com:587' #HOST,PUERTO(PARA GMAIL)
msg = MIMEMultipart()


# -----------FUNCION PARA EL ENMVIO DE DATOS A UBIDOTS-----------

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


#

# -----------FUNCION PARA EL ENVIO DE CORREO ELECTRONICO CON EL VIDEO CAPTURADO-----------

def ENVIO_CORREO():
#
    msg['To'] = CORREO_DESTINO
    msg['From'] = CORREO_MAESTRO
    msg['Subject'] = 'ALERTA '
    msg.attach(MIMEText('GRABACION DE ALERTA '))
    GRABACION = open('/home/pi/Documents/Camara/GRABACION.avi', 'rb')  # RUTA DEL VIDEO A MANDAR
    adjunto = MIMEBase('multipart', 'encrypted')

    adjunto.set_payload(GRABACION.read())
    GRABACION.close()
    encode_base64(adjunto)
    adjunto.add_header('Content-Disposition', 'attachment', filename='VIDEO GRABADO.mp4')
    msg.attach(adjunto)

    server = smtplib.SMTP(smtp_server)
    server.starttls()
    server.login(CORREO_MAESTRO, PASSWORD)
    server.sendmail(CORREO_MAESTRO, CORREO_DESTINO, msg.as_string())
    print("GRABACION ENVIADA")
    server.quit()
#

# -----------FUNCION PARA LA CAPTURA DEL VIDEO POR 20 SEGUNDOS-----------

def CAP_VIDEO():
#  
    DETENER = False

    while True:   
        ret, FRAME = CAPTURA.read()
        if ret:
            VIDEO_SALIDA.write(FRAME)
            cv2.imshow('VIDEO', FRAME)               
            TRASCURRIDO = time.time() - EMPIEZA_CONTEO 

            if TRASCURRIDO > TEMPORIZADOR_GRABACION:
                DETENER = True
                print("PASARON 10 SEGUNDOS ",TRASCURRIDO)

        else:
            break

        if cv2.waitKey(1) & DETENER == True:
            print("VIDEO FINALIZADO ENVIO DE VIDEO A CORREO")        
            break

    CAPTURA.release()
    VIDEO_SALIDA.release()
    cv2.destroyAllWindows()
#
# ----

'''def RECUPERAR_DATOS ():
#
    TEMP_CICLO = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
    TEMP_INTERRUPTOR = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
    TEMP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
    TEMP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
    TEMP_CORRIENTE = OBTENER_DATO(DEVICE_LABEL, CORRIENTE)
    TEMP_TOMACORRIENTE = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)
#'''

name = 'main'

if name == 'main':

    # RECUPERAR_DATOS()

    # RECUPERAR DATOS DESDE UBIDOTS

    TEMP_CICLO = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
    TEMP_INTERRUPTOR = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
    TEMP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
    TEMP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
    TEMP_CORRIENTE = OBTENER_DATO(DEVICE_LABEL, CORRIENTE)
    TEMP_TOMACORRIENTE = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)

    print("ESTADO INICIAL CICLO UTIL: " + str(TEMP_CICLO))
    print("ESTADO INICIAL INTERRUPTO: " + str(TEMP_INTERRUPTOR))
    print("ESTADO INICIAL DIS_COMIDA: " + str(TEMP_COMIDA))
    print("ESTADO INICIAL DISPE_AGUA: " + str(TEMP_AGUA))
    print("ESTADO INICIAL  CORRIENTE: " + str(TEMP_CORRIENTE))
    print("ESTADO INICIAL TOMA_CORRI: " + str(TEMP_TOMACORRIENTE))

    while (True):

        # RECUPERAR DATOS DESDE UBIDOTS

        DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
        INTERRUP = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
        DISP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
        DISP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
        CURRENT = OBTENER_DATO(DEVICE_LABEL, CORRIENTE)
        TOMA = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)

        # COMPARAR LOS ULTIMOS DATOS LEIDOS EN UBIDOTS CON LOS ALAMCENADOS EN LOCAL PARA DETERMINAR SI EL USUARIO QUIERE REALIZAR UNA ACCION

        if INTERRUP != TEMP_INTERRUPTOR:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("El interruptor cambio")
            TEMP_INTERRUPTOR = INTERRUP
            if INTERRUP == 1.0:
                print("Interruptor Activado")
                TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
                temp = 'B'
                SERIAL.write(temp.encode())
                ESPERAR = 1
                while ESPERAR == 1:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            # .decode   PROBAR
                            if LEER == bytes('U'.encode()):  # EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
                                time.sleep(0.5)
                                print(str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                                temp = (str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                                SERIAL.write(temp.encode())
                                RESPUESTA = 0
                                while RESPUESTA == 0:
                                    if SERIAL.readable() == True:
                                        LEER = SERIAL.read()
                                        if LEER != bytes(''.encode()):
                                            print(LEER)
                                            if LEER == bytes('R'.encode()):
                                                print("DIMMER RECIBIO Y SETEO SU CICLO UTIL")
                                                RESPUESTA = 1
                                                ESPERAR = 0
            if INTERRUP == 0.0:
                ENVIAR_DATO(CICLO_UTIL, 0)
                print("Interruptor Desactivado")
                temp = 'C'
                SERIAL.write(temp.encode())
                ESPERAR = 1
                while ESPERAR == 1:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            if LEER == bytes(
                                    'I'.encode()):  # EL MICRO YA ENVIO Y RECIBIO LA CONFIRMACION DE LA DESACTIVACION DEL BOMBILLO
                                ESPERAR = 0
                            
                            
        if DIMMER != TEMP_CICLO and INTERRUP == 1.0:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("El dimmer cambio")
            TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
            temp = 'A'
            SERIAL.write(temp.encode())
            ESPERAR = 1
            while ESPERAR == 1:
                if SERIAL.readable() == True:
                    LEER = SERIAL.read()
                    if LEER != bytes(''.encode()):
                        print(LEER)
                        if LEER == bytes('U'.encode()):  # EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
                            time.sleep(0.5)
                            print(str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                            temp = (str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                            SERIAL.write(temp.encode())
                            RESPUESTA = 0
                            while RESPUESTA == 0:
                                if SERIAL.readable() == True:
                                    LEER = SERIAL.read()
                                    if LEER != bytes(''.encode()):
                                        print(LEER)
                                        if LEER == bytes('R'.encode()):
                                            print("DIMMER RECIBIO Y SETEO SU CICLO UTIL")
                                            RESPUESTA = 1
                                            ESPERAR = 0


        if DISP_COMIDA != TEMP_COMIDA:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("La comida cambio")
            TEMP_COMIDA = DISP_COMIDA
            temp = 'D'
            SERIAL.write(temp.encode())
            ESPERAR = 1
            while ESPERAR == 1:
                if SERIAL.readable() == True:
                    LEER = SERIAL.read()
                    if LEER != bytes(''.encode()):
                        print(LEER)
                        if LEER == bytes('G'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                            ENVIAR_DATO(COMIDA, 0.0)
                            DISP_COMIDA = TEMP_COMIDA = 0.0;
                            ESPERAR = 0


        if DISP_AGUA != TEMP_AGUA:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("El agua cambio")
            TEMP_AGUA = DISP_AGUA
            temp = 'E'
            SERIAL.write(temp.encode())
            ESPERAR = 1
            while ESPERAR == 1:
                if SERIAL.readable() == True:
                    LEER = SERIAL.read()
                    if LEER != bytes(''.encode()):
                        print(LEER)
                        if LEER == bytes('H'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                            ENVIAR_DATO(AGUA, 0.0)
                            DISP_AGUA = TEMP_AGUA = 0.0;
                            ESPERAR = 0


        if CURRENT != TEMP_CORRIENTE:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("La corriente cambio cambio")
            TEMP_CORRIENTE = CURRENT


        if TOMA != TEMP_TOMACORRIENTE:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("La toma cambio cambio")
            TEMP_TOMACORRIENTE = TOMA

            if TOMA == 1:
                temp = 'F'
                SERIAL.write(temp.encode())
                ESPERAR_2 = 1
                while ESPERAR_2 == 1:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            if LEER == bytes('F'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                                ESPERAR_2 = 0

            elif TOMA == 0:
                temp = 'G'
                SERIAL.write(temp.encode())
                ESPERAR_3 = 1
                while ESPERAR_3 == 1:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            if LEER == bytes('N'.encode()):  # EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                                ESPERAR_3 = 0

        
        print("el bucle está operando")
        

        
        if SERIAL.readable() == True:
            LEER = SERIAL.read()
            if LEER != bytes(''.encode()):
                print(LEER)
                # print(str(LEER) + str(type(LEER)))
                
                if LEER == bytes('U'.encode()):  # EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
                    print(str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                    temp = (str("{:03d}".format(int(TEMP_CICLO))) + str('C'))
                    SERIAL.write(temp.encode())
                if LEER == bytes('R'.encode()):
                    print("DIMMER RECIBIO Y SETEO SU CICLO UTIL")
                if LEER == bytes('C'.encode()):
                    print("EL MAESTRO TIENE UN CILO PARA ENVIARME")
                    temp = 'C'
                    SERIAL.write(temp.encode())
                    CICLO = 0
                    while CICLO == 0:
                        if SERIAL.readable() == True:
                            SERIAL.flush()
                            DUTY = SERIAL.readline()
                            print("El ciclo recibido es: " + str(DUTY) + "   " + str(type(DUTY)))
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
        
                if LEER == bytes('Z'.encode()):
                    print("EL MAESTRO TIENE UNA CORRIENTE PARA ENVIAR")
                    temp = 'Z'
                    SERIAL.write(temp.encode())
                    COR = 0
                    while COR == 0:
                        if SERIAL.readable() == True:
                            SERIAL.flush()
                            corriente = SERIAL.readline()
                            print("LA CORRIENTE RECIBIDA ES : " + str(corriente) + "   " + str(type(corriente)))
                            COR = 1
                            temp = 'z'
                            SERIAL.write(temp.encode())
                            COR_ENV = float(corriente)
                            COR_ENV = COR_ENV / 100
                            print("LA CORRIENTE EN MEMORIA ES: " + str(COR_ENV) + "   " + str(type(COR_ENV)))
                            ENVIAR_DATO(CORRIENTE, COR_ENV)
            
                # ------camara--------
                if LEER == bytes('P'.encode()):
                    print("PUERTA SE ABRIO")
                    # EMPIEZA_CONTEO = time.time()  # conteo para finalizar grabacion                   
                    # CAP_VIDEO()                   # Llama funciones de captura de video y envio al correo 
                    # ENVIO_CORREO()                  
                    temp = 'T'
                    SERIAL.write(temp.encode())
                    print("GRABACION Y ENVIO COMPLETADO ")

```

# FALTA ADJUNTAR DIAGRAMA DE FLUJO PYTHON

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
```

# FALTA ADJUNTAR DIAGRAMA DE FLUJO C
