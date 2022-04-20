# IoT-Home-Sistem-with-STM32-and-Raspberry

Este repositorio consigna toda la información relacionada con el desarrollo de un sistema de IoT para controlar y/o monitorear 4 eventos concretos en un hogar: 

1. Ingreso a la vivienda.
2. Monitoreo de consumo eléctrico en tomacorrientes.
3. Encendido, apagado y dimerización de luces.
4. Dispensado de bebida y alimento para mascotas.

## Esquema General de la Solución

![Esquema General del Sistema](Imagenes/ESQUEMA_GENERAL.png)

El sistema es capaz de detectar las órdenes fijadas por el usuario en la nube y direccionar cada orden al módulo encargado de la tarea a realizar por medio del controlador maestro. Además, cada módulo que requiere enviar información hacia la nube (como es el caso del sensor de corriente) envía los datos al controlador maestro, y este se encarga de actualizar la información en la nube mediante la SBC.

## Esquema General del Controlador Maestro y Módulos de Control

![Esquema General del Controlador Maestro y Modulos de Control](Imagenes/ESQUEMA_MAESTRO_MODULOS.png)

## Generalidades del Sistema

### DASHBOARD:
El dashboard mediante el cual el usuario puede indicar las tareas a realizar en cada módulo de control es [Ubidots](https://ubidots.com/), una plataforma IoT que permite conectar hardware con la nube mediante diferentes protocolos de comunicación como HTTP, MQTT, TCP, entre otros. Esta plataforma nos permite crear diferentes tableros que incorporan botones, gráficos, alertas y demás elementos que permiten visualizar, modificar y/o monitorear los valores de las variables ligadas a cada uno de los elementos que componen el dashboard. 

![DASHBOARD_FINAL](Imagenes/DASHBOARD.png)

### CONTROLADOR MAESTRO:

Para poder establecer una comunicación entre en dashboard en la nube y el controlador maestro es necesario contar con un dispositivo con capacidad de conexión a internet. A su vez para conectar el controlador maestro con los módulos de control se requiere que la SBC tenga puertos de entrada, salida e interfaces de comunicación. 

Teniendo en cuenta lo anterior, el controlador maestro implementado a partir de una SBC es básicamente un puente entre el dashboard en la nube y los módulos periféricos que son los que realizan las mediciones y/o acciones.
## Esquema de enlace entre Dashboard y módulos de control mediante el controlador maestro
![CONTROLADOR_MAESTRO](Imagenes/CONTROLADOR_MAESTRO.png)

- La SBC que enlaza las ordenes enviadas por el usuario en la nube con el sistema es la Raspberry Pi 4 de 2GB de RAM, a la cual se le instaló el sistema operativo [Raspbian](https://www.raspberrypi.com/software/).


- La tarjeta de desarrollo implementada en esta solución es la Blue Pill que incorpora un STM32F103C8, una CPU Cortex-M3 de 32-bits. La programación de dicha tarjeta se realizó mediante [Mbed Online Compiler](https://os.mbed.com/).


- El intercambio de información entre los diferentes módulos de control y un único controlador maestro se realizó implementando comunicación RF en la banda ISM mediante los radios de comunicación nRF24L01 con antena integrada. La librería usada en Mbed para integrar el radio con la Blue Pill fue [nRF24L01](https://os.mbed.com/components/nRF24L01/)


# Componentes principales de la solución

| Componente | Aspecto Físico | Aspecto Físico |
| :---         |          ---: |          ---: |
| Raspberry Pi 4                | **Procesador:** ARM corex-A72 64bits 4 núcleos 1.5 GHz.<br /> **GPU:** Video Core VI.<br /> **Memoria RAM:** 2GB.<br /> **Puertos:** Pines GPIO (40), micro HDMI (2), USB 2.0 (2), USB 3.0 (2).<br /> **Conectividad:** WI-FI, Bluetooth 5.0, Gigabit Ethernet |![](Imagenes/RASPBERRY.png) |
| Blue Pill                     |  |![](Imagenes/BLUEPILL.png) |
| Radio nRF24L01                |  |![](Imagenes/RADIO.png) |
| Cámara IP                     |  |![](Imagenes/CAMARA.png) |
| Motor DC                      |  |![](Imagenes/MOTOR.png) |
| Motobomba DC                  |  |![](Imagenes/MOTOBOMBA.png) |
| Módulo Puente H               |  |![](Imagenes/PUENTEH.png) |
| Conversor AC/DC               |  |![](Imagenes/FUENTE.png) |
| Regulador de Voltaje          |  |![](Imagenes/78L33.png) |
| Amplificador Operacional      |  |![](Imagenes/358.png) |
| Amplificador Operacional      |  |![](Imagenes/TL084.png) |
| Optoacoplador                 |  |![](Imagenes/3021.png) |
| Tiristor                      |  |![](Imagenes/BT138.png) |
| Transformador de Corriente    |  |![](Imagenes/CURRENT_T.png) |
| Contacto Magnético            |  |![](Imagenes/SENSOR_PUERTA.png) |

# Solución Final

## Dashboard en Ubidots

![DASHBOARD_FINAL](Imagenes/UBIDOTS.png)

## Controlador Maestro

### Tarjeta de circuito impreso módulo controlador maestro
![DASHBOARD_FINAL](Imagenes/FOTO_CONTROLADOR.png)
### Conexión módulo maestro y Raspberry Pi
![DASHBOARD_FINAL](Imagenes/FOTO_CONTROLADOR_MAESTRO.png)

## Módulo de Monitoreo de Puerta
### Tarjeta de circuito impreso módulo monitoreo de puerta
![DASHBOARD_FINAL](Imagenes/FOTO_MODULO_PUERTA.png)
### Foto cámara implementada
![DASHBOARD_FINAL](Imagenes/FOTO_CAMARA_IMPLEMENTADA.png)
### Foto contacto magnético en puerta
![DASHBOARD_FINAL](Imagenes/FOTO_SENSOR_MAGNETICO.png)
### Correo electrónico con grabación
![DASHBOARD_FINAL](Imagenes/CORREO_ENVIADO.png)

## Módulo Sensor de Corriente
### Tarjeta de circuito impreso módulo sensor de corriente
![DASHBOARD_FINAL](Imagenes/FOTO_SENSOR_CORRIENTE.png)
### Tablero de pruebas módulo sensor de corriente
![DASHBOARD_FINAL](Imagenes/FOTO_TABLERO_SENSOR.png)
### Diagrama tablero de pruebas
![DASHBOARD_FINAL](Imagenes/ESQUEMA_TABLERO_SENSOR.png)
### Medición realizada por el sensor-vs-corriente leida por Multuimetro Unit UT39C
![DASHBOARD_FINAL](Imagenes/MEDIDA_SENSOR_VS_MULTIMETRO.png)

## Módulo Dimmer
### Tarjeta de circuito impreso módulo dimmer
![DASHBOARD_FINAL](Imagenes/FOTO_DIMMER.png)
### Carga al 30%
![DASHBOARD_FINAL](Imagenes/DIMMER_30.png)
### Carga al 100%
![DASHBOARD_FINAL](Imagenes/DIMMER_100.png)

## Módulo Dispensador de Comida y Bebida para Mascotas
### Tarjeta de circuito impreso módulo dispensador
![DASHBOARD_FINAL](Imagenes/FOTO_MODULO_DISPENSADOR.png)
### Conexiones módulo dispensador con Puente H
![DASHBOARD_FINAL](Imagenes/FOTO_DISPENSADOR_PUENTEH.png)
### Dispensador de comida (motor y tornillo sin fin)
![DASHBOARD_FINAL](Imagenes/FOTO_DISPENSADOR_COMIDA.png)
### Dispensador de agua (motobomba)
![DASHBOARD_FINAL](Imagenes/FOTO_DISPENSADOR_AGUA.png)
### Prototipo de dispensador de comida y bebida para mascotas
![DASHBOARD_FINAL](Imagenes/FOTO_DISPENSADOR.png)