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
| :---:         |          :--- |          :---: |
| Raspberry Pi 4                | **Procesador:** ARM corex-A72 64bits 4 núcleos 1.5 GHz<br /> **GPU:** Video Core VI<br /> **Memoria RAM:** 2GB<br /> **Puertos:** Pines GPIO (40), micro HDMI (2), USB 2.0 (2), USB 3.0 (2)<br /> **Conectividad:** WI-FI, Bluetooth 5.0, Gigabit Ethernet |![](Imagenes/RASPBERRY.png) |
| Blue Pill                     | **Voltaje de operación:** 3.3V<br /> **Pines analógicos:** 10<br /> **Pines Digitales:** 37<br /> **Protocolos de comunicación:** SPI (2), UART (3), I2C (2)<br /> **Memoria flash:** 64/128 KB<br /> **Memoria SRAM:** 20KB<br />  |![](Imagenes/BLUEPILL.png) |
| Radio nRF24L01                | **Modulo transceptor**<br />**Voltaje de operación:** 3.3V<br />**Protocolo de comunicación:** SPI<br />**Frecuencia:** 2.4 GHz – 2.5GHz<br />**Canales RF:** 126<br />**Alcance:** 20m – 30m |![](Imagenes/RADIO.png) |
| Cámara IP                     | **Modelo:** V380<br />**Protocolo:** ONVIF<br />**Visión nocturna**<br />**Conectividad:** WIFI, Red RJ45<br />**Resolución:** 1080p |![](Imagenes/CAMARA.png) |
| Motor DC                      | **Modelo JGY370**<br />**Motor con motorreductor de engranajes**<br />**Voltaje de operación:** 12V DC<br />**Corriente de operación:** 0.5A<br />**Velocidad:** 40RPM<br />**Torque:** 3 Kg/cm |![](Imagenes/MOTOR.png) |
| Motobomba DC                  | **Modelo:** R385<br />**Voltaje de operación:** 12V DC<br />**Corriente de operación:** 0.5-0.7A<br />**Dimensiones:** 90mm x 40 mm x 35mm<br />**Flujo volumétrico:** 1.5-2 L/min<br />**Diámetro de salida:** 6mm |![](Imagenes/MOTOBOMBA.png) |
| Módulo Puente H               | **Circuito Integrado:** L298<br />**Voltaje de operación:** 5V DC<br />**Corriente de operación:** 36mA<br />**Número de canales:** 2<br />**Voltaje por canal:** 5V DC - 35V DC<br />**Corriente MAX por canal:** 2A |![](Imagenes/PUENTEH.png) |
| Conversor AC/DC               | **Modelo:** Hi-link (HLK-PM01)<br />**Voltaje de entrada:** 100-240V AC<br />**Voltaje de salida:** 5V DC<br />**Corriente de salida:** 0.6A DC<br /> |![](Imagenes/FUENTE.png) |
| Regulador de Voltaje          | **Modelo:** L78L33<br />**Voltaje Max entrada:** 30V DC<br />**Voltaje de salida:** 3.3V DC<br />**Corriente de salida:** 100mA |![](Imagenes/78L33.png) |
| Amplificador Operacional      | **Modelo:** LM358<br />**Numero de amplificadores:** 2<br />**Voltaje de operación:** 3V – 32V<br />**Voltaje offset entradas:** 2mV<br />**Ganancia de voltaje diferencial:** 100 dB |![](Imagenes/358.png) |
| Amplificador Operacional      | **Modelo:** TL084<br />**Entradas transistores JFET**<br />**Numero de amplificadores:** 4<br />**Voltaje de operación:** ± 18V<br />**Voltaje offset entradas:** 3mV |![](Imagenes/TL084.png) |
| Optoacoplador                 | **Modelo:** MOC 3021<br />**Salida:** Fototriac<br />**Voltaje directo Max LED:** 1.5V<br />**Corriente Max LED:** 60mA<br />**Corriente de activación LED:** 15-30mA<br />**Voltaje Max salida:** 400Vp |![](Imagenes/3021.png) |
| Tiristor                      | **Modelo:** TRIAC BT138<br />**Valor instantáneo máximo de pulsos de voltaje (VDRM):** 600V<br />**Valor de corriente RMS ánodo-cátodo en directo (IT):** 12A<br />**Valor pico máximo de corriente encendido:** 95A<br />**Voltaje pico de gate (VGM):** 5V<br />**Corriente máxima Trigger Gate:** 70mA |![](Imagenes/BT138.png) |
| Transformador de Corriente    | **Modelo:** ZMCT103C<br />**Rango corriente de entrada:** 0 - 10A (50Ω)<br />**Rango corriente de salida:** 5mA<br />**Proporción:** 1000:1<br />**Linealidad:** ≤0.2%<br />**Voltaje de aislamiento:** 4500V |![](Imagenes/CURRENT_T.png) |
| Contacto Magnético            | **Tipo:** Interruptor Reed Switch<br />**Voltaje máximo:** 100V DC<br />**Corriente máxima:** 0.5A<br />**Potencia:** 10W<br />**Distancia de actuación:** 20-25mm |![](Imagenes/SENSOR_PUERTA.png) |

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