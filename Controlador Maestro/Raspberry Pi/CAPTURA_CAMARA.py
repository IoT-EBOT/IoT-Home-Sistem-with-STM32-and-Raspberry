import cv2
import numpy as np
import time
import smtplib

from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email.encoders import encode_base64

EMPIEZA_CONTEO = time.time()   # conteo para finalizar grabacion
TEMPORIZADOR_GRABACION = 20    # tiempo de ejecucion del programa

#--------------DATOS GRABACION--------------
URL = 'rtsp://192.168.0.100/live/ch00_1'
CAPTURA = cv2.VideoCapture(URL)

FPS = CAPTURA.get(cv2.CAP_PROP_FPS)
ANCHO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_WIDTH))
ALTO = int(CAPTURA.get(cv2.CAP_PROP_FRAME_HEIGHT))
FORMATO = cv2.VideoWriter_fourcc('X','2','6','4')
VIDEO_SALIDA = cv2.VideoWriter('GRABACION.avi', FORMATO, FPS, (ANCHO,ALTO))

#-----------DATOS ENVIO CORREO------------------
CORREO_DESTINO = 'dgomezbernal24@gmail.com,cristiancobos2002@gmail.com'
CORREO_MAESTRO = 'iot.e.bot21@gmail.com'
PASSWORD = 'E-BOT2021' 
smtp_server = 'smtp.gmail.com:587' #HOST,PUERTO(PARA GMAIL)
msg = MIMEMultipart()

def ENVIO_CORREO():

    msg['To'] = CORREO_DESTINO
    msg['From'] = CORREO_MAESTRO
    msg['Subject'] = 'ALERTA PUERTA'
    msg.attach(MIMEText('GRABACION DE ALERTA DETECTADA EN LA PUERTA '))
    GRABACION = open('/home/pi/Desktop/MAESTRO/GRABACION.avi', 'rb')  # RUTA DEL VIDEO A MANDAR
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
    
def CAP_VIDEO():
   
    DETENER = False
    EMPIEZA_CONTEO = 0.0
    EMPIEZA_CONTEO = time.time() 
    
    while True:
              
        ret, FRAME = CAPTURA.read()
        
        if (ret == 1):         
            print("GRABANDO")       
            print("FPS = "   + str(FPS))
            print("ANCHO = " + str(ANCHO))
            print("ALTO = "  + str(ALTO))            
        
            VIDEO_SALIDA.write(FRAME)
            cv2.imshow('VIDEO', FRAME)               
            TRASCURRIDO = time.time() - EMPIEZA_CONTEO
            
            if TRASCURRIDO > TEMPORIZADOR_GRABACION:
                DETENER = True
                print("PASARON " + str(TRASCURRIDO) + " SEGUNDOS")
                
        else:
            break
        
        if (cv2.waitKey(1) & DETENER == True):
            
            print("CAPTURA DE VIDEO FINALIZADO ")
            DETENER = False
            EMPIEZA_CONTEO = 0.0
            EMPIEZA_CONTEO = time.time()
            ret = True
            TRASCURRIDO = 0.0                  
            break
    
    CAPTURA.release()
    VIDEO_SALIDA.release()
    cv2.destroyAllWindows()    

if __name__ == '__main__':
    
    print("Se ejecuto la rutina principal ")  
    CAP_VIDEO()
    ENVIO_CORREO()
    print("SE CERRO PROGRAMA DE GRABACION") 
    exit(1)
    
