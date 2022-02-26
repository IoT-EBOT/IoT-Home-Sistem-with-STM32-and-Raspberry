#-------------------------------------------------LIBRERIAS USADAS------------------------------------------

import requests
import serial
import time
'''import cv2
import numpy as np
import smtplib

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.encoders import encode_base64'''

#-------------------------------------------------ConfiguraciOn Puerto Serie------------------------------------------

SERIAL = serial.Serial('/dev/ttyAMA0', 9600, timeout=3.0, write_timeout = 3.0) #TTL se_port

#---------------------------------------------Parametros  y variables en Ubidots-------------------------------------------

TOKEN = "BBFF-1JPdBOfYF2swcxLWhBIwkKDZLmeBti"   # TOKEN de ubidots 
DEVICE_LABEL = "raspberry"                      # Nombre de dispositivo en Ubidots

CICLO_UTIL = "dimmer"                           # VARIABLE: Ciclo util para disparo de dimmer
INTERRUPTOR = "interruptor"                     # VARIABLE: Activar o desactivar bombillo

COMIDA = "dispensador_com"                      # VARIABLE: Control para activación de dispensador de comida
AGUA = "dispensador_agua"                       # VARIABLE: Control para activación de dispensador de agua

CORRIENTE = "sen_cor"                           # VARIABLE: Valor de corriente sensada
TOMA_CORRIENTE = "toma_cor"                     # VARIABLE: On/off para toma corriente

DELAY = 1  # Delay en segundos

#-------------Variables locales usadas para almacenar lo valores rescatados y a enviar a Ubidots----------

DIMMER = 0
INTERRUP = 0
DISP_COMIDA = 0
DISP_AGUA = 0
CURRENT = 0.0
TOMA = 0

#--------------------------VARIABLES TEMPORALES PARA ALMACENAR DATOS Y COMPARARLOS--------------------------

TEMP_CICLO = 0.0
TEMP_INTERRUPTOR = 0.0
TEMP_COMIDA = 0.0
TEMP_AGUA = 0.0
TEMP_CORRIENTE = 0.0
TEMP_TOMACORRIENTE = 0.0

#---Parámetros y variables para control de cámara____Datos correo electrónico de órigen del video y los correos que lo reciben---

#EMPIEZA_CONTEO = time.time()   # conteo para finalizar grabacion
TEMPORIZADOR_GRABACION = 20    # PIERDE TRES SEGUNDOS DE GRABACION
DETENER = False                # PERMISO PARA DETENER GRABACION AUTOMATICAMENTE
CAMARA  = False                # PERMISO PARA INICIAR GRABACION LO MANDA MAESTRO CON UNA 'P'

#--------------DATOS GRABACION--------------

'''URL = 'rtsp://192.168.0.100/live/ch00_1'        # IP estática de la cámara, obtenido desde ONVIF
CAPTURA = cv2.VideoCapture(URL)
FPS = 10
ANCHO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_WIDTH))
ALTO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_HEIGHT))
FORMATO = cv2.VideoWriter_fourcc('X','2','6','4')
VIDEO_SALIDA = cv2.VideoWriter('GRABACION.avi', FORMATO, FPS, (ANCHO,ALTO))'''

#-----------DATOS ENVIO CORREO------------------

'''CORREO_DESTINO = 'dgomezbernal24@gmail.com,cristiancobos2002@gmail.com'
CORREO_MAESTRO = 'iot.e.bot21@gmail.com'
PASSWORD = 'E-BOT2021'
smtp_server = 'smtp.gmail.com:587' #HOST,PUERTO(PARA GMAIL)
msg = MIMEMultipart()'''

#-----------FUNCION PARA EL ENMVIO DE DATOS A UBIDOTS-----------

def ENVIAR_DATO(VARIABLE, VALOR):
#
    #CONTRUIR DICCIONARIO TEMPORAL PARA EL ENVíO DE ENFORMACIÓN
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

#-----------FUNCION PARA EL RESCATE DE DATOS DESDE UBIDOTS-----------

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

#-----------FUNCION PARA EL ENVIO DE CORREO ELECTRONICO CON EL VIDEO CAPTURADO-----------

'''def ENVIO_CORREO():
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
#'''

#-----------FUNCION PARA LA CAPTURA DEL VIDEO POR 20 SEGUNDOS-----------

'''def CAP_VIDEO():
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
#'''
#----

'''def RECUPERAR_DATOS ():
#
    TEMP_CICLO = OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL)
    TEMP_INTERRUPTOR = OBTENER_DATO(DEVICE_LABEL, INTERRUPTOR)
    TEMP_COMIDA = OBTENER_DATO(DEVICE_LABEL, COMIDA)
    TEMP_AGUA = OBTENER_DATO(DEVICE_LABEL, AGUA)
    TEMP_CORRIENTE = OBTENER_DATO(DEVICE_LABEL, CORRIENTE)
    TEMP_TOMACORRIENTE = OBTENER_DATO(DEVICE_LABEL, TOMA_CORRIENTE)
#'''

if name == 'main':
     
    #RECUPERAR_DATOS()
    
    #RECUPERAR DATOS DESDE UBIDOTS

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

        #COMPARAR LOS ULTIMOS DATOS LEIDOS EN UBIDOTS CON LOS ALAMCENADOS EN LOCAL PARA DETERMINAR SI EL USUARIO QUIERE REALIZAR UNA ACCION
                if INTERRUP != TEMP_INTERRUPTOR:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
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
                            #.decode   PROBAR
                            if LEER == bytes('U'.encode()): #EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
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
                ENVIAR_DATO(CICLO_UTIL,0)
                print("Interruptor Desactivado")
                temp = 'C'
                SERIAL.write(temp.encode())
                ESPERAR = 1
                while ESPERAR == 1:
                    if SERIAL.readable() == True:
                        LEER = SERIAL.read()
                        if LEER != bytes(''.encode()):
                            print(LEER)
                            if LEER == bytes('I'.encode()): #EL MICRO YA ENVIO Y RECIBIO LA CONFIRMACION DE LA DESACTIVACION DEL BOMBILLO
                                ESPERAR = 0
        # or (INTERRUP != TEMP_INTERRUPTOR and INTERRUP == 1.0)
        if DIMMER != TEMP_CICLO and INTERRUP == 1.0:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
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
                        if LEER == bytes('U'.encode()): #EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
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
        if DISP_COMIDA != TEMP_COMIDA:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
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
                            if LEER == bytes('G'.encode()): #EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                                ENVIAR_DATO(COMIDA,0.0)
                                DISP_COMIDA = TEMP_COMIDA = 0.0;
                                ESPERAR = 0
        if DISP_AGUA != TEMP_AGUA:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
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
                            if LEER == bytes('H'.encode()): #EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                                ENVIAR_DATO(AGUA,0.0)
                                DISP_AGUA = TEMP_AGUA = 0.0;
                                ESPERAR = 0
        if CURRENT != TEMP_CORRIENTE:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
            print("La corriente cambio cambio")
            TEMP_CORRIENTE = CURRENT
        if TOMA != TEMP_TOMACORRIENTE:      #Aqui falta dar un tiempo por si el usuario se pone de CHISTOSO a jugar con el slider
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
                                if LEER == bytes('F'.encode()): #EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
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
                                if LEER == bytes('N'.encode()): #EL MICRO YA ENVIO Y EL DISPENSADOR RECIBIO LA ORDEN
                                    ESPERAR_3 = 0
            
        print("el bucle está operando")      
            
                if SERIAL.readable() == True:
            LEER = SERIAL.read()
            if LEER != bytes(''.encode()):
                print(LEER)
                #print(str(LEER) + str(type(LEER)))
                if LEER == bytes('U'.encode()): #EL MICRO ESTA LISTO PARA RECIBIR EL CICLO UTIL
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
                            ENVIAR_DATO(CICLO_UTIL,DUTY)
                            if DUTY != 0:
                                ENVIAR_DATO(INTERRUPTOR,1.0)
                                INTERRUP = TEMP_INTERRUPTOR = 1.0
                                DIMMER = TEMP_CICLO = float(DUTY)
                            elif DUTY == 0:
                                ENVIAR_DATO(INTERRUPTOR,0.0)
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
                            COR_ENV = COR_ENV/100
                            print("LA CORRIENTE EN MEMORIA ES: " + str(COR_ENV) + "   " + str(type(COR_ENV)))
                            ENVIAR_DATO(CORRIENTE,COR_ENV)

                #------camara--------
                if LEER == bytes('P'.encode()):
                    print("PUERTA SE ABRIO")
                    #EMPIEZA_CONTEO = time.time()  # conteo para finalizar grabacion                   
                    #CAP_VIDEO()                   # Llama funciones de captura de video y envio al correo 
                    #ENVIO_CORREO()                  
                    temp = 'T'
                    SERIAL.write(temp.encode())
                    print("GRABACION Y ENVIO COMPLETADO ")

                    
        #for i in range(0,101,10):
        #    ENVIAR_DATO(CICLO_UTIL,i)

        
        #DIMMER = int(OBTENER_DATO(DEVICE_LABEL, CICLO_UTIL))
        #print("VALOR DE DIMER:     " + str(DIMMER) + "   " + str(type(DIMMER)))
        
        #INTERRUP = int(OBTENER_DATO(DEVICE_LABEL,INTERRUPTOR))
        #print("ESTADO INTERRUPTOR: " + str(INTERRUP) + "   " + str(type(INTERRUP)))
        
        #DISPENSADOR = int(OBTENER_DATO(DEVICE_LABEL, COMIDA))
        #print("ESTADO DISPENSADOR: " + str(DISPENSADOR) + "   " + str(type(DISPENSADOR)))