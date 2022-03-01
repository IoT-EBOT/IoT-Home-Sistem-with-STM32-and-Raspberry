# Implementación Google Assistant Y Ubidots

Con la implementación de Google assistant y Ubidots se podrá controlar el estado del interruptor (encendido y apagado), el nivel de intensidad lumínica del dimmer, solicitar el accionamiento del dispensador tanto para agua como comida usando comandos de voz. Esto se logra usando IFTTT, el cual es un servicio web que permite conectar aplicaciones y automatizar tareas.En este caso al recibir un comando de voz mediante Google Voice assistant, realiza una solicitud web con webhook a Ubidots para modificar las variables requeridas. En otras palabras, IFTTT es un intermediario que envía la solicitud realizada por Google Assistant a Ubidots directamente.


Para configurar una acción en IFTTT se deben seguir una serie de pasos descritos a continuación:


1. Crear una cuenta en IFTTT, esta cuenta al ser gratis nos permite crear 5 applets. Además, se debe otorgar permisos a la cuenta de Google para que se pueda usar los servicios de IFTTT.
2. Creación de un applet:

    ![CREAR](imagenes/CREAR.png)

* 2.1 If this: Permite activar un evento para esto se selecciona el servicio de Google assistant el cual recibirá el parámetro que se desea cambiar por medio de un comando de voz, en este apartado se debe indicar la frase para tomar alguna acción determinada.

    ![IF_THIS](imagenes/IF_THIS.png)

* 2.2 Then that: Permite realizar una acción para esto se selecciona el servicio Webhooks el cual realizara la solicitud web al servicio de Ubidots para cambiar el estado de la variable que se le indique, esta acción se ejecutara luego de recibir el comando de voz, en dicho apartado se debe suministrar el URL del servicio al que va a realizar la solicitud en este caso Ubidots seguido del token, Seguido del método(post) y tipo de contenido (aplicaction/json) y por último la variable que se desea modificar junto con el valor que va a tomar {"api_label ":valor}.

    ![THAT](imagenes/THAT.png)

3. Una vez configurado el activador, la acción ya se puede verificar por medio de la aplicación de Google Voice assistant. 

    ![COMANDOS](imagenes/COMANDOS.png)

    ![PRUEBA](imagenes/PRUEBA.png)

