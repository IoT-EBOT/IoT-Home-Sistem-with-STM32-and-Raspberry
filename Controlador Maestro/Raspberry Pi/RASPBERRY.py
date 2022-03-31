# -------------------------------------------------LIBRERIAS USADAS------------------------------------------

import requests
import serial
import time
import os

# -------------------------------------------------ConfiguraciOn Puerto Serie------------------------------------------

SERIAL = serial.Serial('/dev/ttyAMA0', 9600, timeout=3.0, write_timeout=3.0)  # TTL se_port

# ---------------------------------------------Parametros  y variables en Ubidots-------------------------------------------

TOKEN = "BBFF-1JPdBOfYF2swcxLWhBIwkKDZLmeBti"   # TOKEN de ubidots 
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
                        print('LA CORRIENTE RECIBIDA ES : ' + str(corriente) + '   ' + str(type(corriente)))
                        COR = 1
                        temp = 'z'
                        SERIAL.write(temp.encode())
                        COR_ENV = float(corriente)
                        COR_ENV = COR_ENV / 100
                        print('LA CORRIENTE EN MEMORIA ES: ' + str(COR_ENV) + '   ' + str(type(COR_ENV)))
                        ENVIAR_DATO(CORRIENTE, COR_ENV)
            
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

        #-------------------------------------------------------------------------RECUPERAR DATOS DESDE UBIDOTS--------------------------------------------------------

        DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
        INTERRUP = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
        DISP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
        DISP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
        TOMA = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)

        #----------------------------------------REVISAR SI EL MICROCONTROLADOR TIENE ALGUNA ALERTA PARA RASPBERRY Y/O UBIDOTS----------------------------------------

        LEER_MICRO ()

        #--------------COMPARAR LOS ULTIMOS DATOS LEIDOS EN UBIDOTS CON LOS ALMACENADOS EN LOCAL PARA DETERMINAR SI EL USUARIO QUIERE REALIZAR UNA ACCION--------------

        if INTERRUP != TEMP_INTERRUPTOR:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print('El interruptor cambio')
            TEMP_INTERRUPTOR = INTERRUP
            if INTERRUP == 1.0:
                print('Interruptor Activado')
                TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
                temp = 'B'
                SERIAL.write(temp.encode())
                ESPERAR_1 = 1
                while ESPERAR_1 == 1:
                    LEER_MICRO()

            if INTERRUP == 0.0:
                ENVIAR_DATO(CICLO_UTIL, 0)
                print('Interruptor Desactivado')
                temp = 'I'
                SERIAL.write(temp.encode())
                ESPERAR_2 = 1
                while ESPERAR_2 == 1:
                    LEER_MICRO()
                            
                            
        if DIMMER != TEMP_CICLO and INTERRUP == 1.0:  # Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print('El dimmer cambio')
            TEMP_CICLO = DIMMER = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
            temp = 'A'
            SERIAL.write(temp.encode())
            ESPERAR_1 = 1
            while ESPERAR_1 == 1:
                LEER_MICRO()


        if DISP_COMIDA != TEMP_COMIDA:  
            print('La comida cambio')
            TEMP_COMIDA = DISP_COMIDA
            temp = 'D'
            SERIAL.write(temp.encode())
            ESPERAR_3 = 1
            while ESPERAR_3 == 1:
                LEER_MICRO()


        if DISP_AGUA != TEMP_AGUA:  
            print('El agua cambio')
            TEMP_AGUA = DISP_AGUA
            temp = 'E'
            SERIAL.write(temp.encode())
            ESPERAR_4 = 1
            while ESPERAR_4 == 1:
                LEER_MICRO()
                        

        if TOMA != TEMP_TOMACORRIENTE:
            print('La toma cambio cambio')
            TEMP_TOMACORRIENTE = TOMA

            if TOMA == 1:
                temp = 'F'
                SERIAL.write(temp.encode())
                ESPERAR_5 = 1
                while ESPERAR_5 == 1:
                    LEER_MICRO()
                            

            elif TOMA == 0:
                temp = 'G'
                SERIAL.write(temp.encode())
                ESPERAR_6 = 1
                while ESPERAR_6 == 1:
                    LEER_MICRO()
                                    
        print('El bucle está operando')

        #ANTIBLOQUEO: FUNCION QUE INICIE UN CONTADOR, CUANDO EL CONTADOR EXCEDA EL SEGUNDO REVISE SI ES QUE TIENE ALGO POR LEER, ATIENDA EL LLAMADO Y AHÍ SI VUELVA A INSISTIR CON LO QUE QUIER ENVIAR