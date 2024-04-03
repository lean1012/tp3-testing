# Trabajo Práctico Número 3

## Uso del repositorio

Este repositorio testea el módulo mesh_routing.c. Este módulo es usado dentro de otro proyecto en el cual se implementa una red mesh basada en BLE (Bluetooth Low Energy). Este módulo se encarga de resolver el ruteo dentro de la red.

Consta de los siguientes módulos:

mesh_conn: se encarga de asignar la id del nodo de la red mesh con la id de conexión de ble. Ademas envía y recive msg BLE.

mesh_routing: se encarga de rutear los mensajes, en caso que correspondan a él lo pasará a la siguiente capa, si es para otro nodo le asignara un next_hop y volvera a la capa inferior para ser transmitido por BLE.

mesh_app: cada nodo de la red puede subscribirse a recibir determinada información, cada información está asociada a un OPCODE. Esta capa se encarga de pasar a la aplicación la información de ese msg
