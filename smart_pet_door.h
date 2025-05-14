#ifndef SMART_PET_DOOR_H
#define SMART_PET_DOOR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>              // Biblioteca manipular strings

#include "pico/stdlib.h"
#include "lib/lwipopts.h"
#include "lib/hc_sr04.h"
#include "lib/servomotor.h"

#include "pico/cyw43_arch.h"     // Biblioteca para arquitetura Wi-Fi da Pico com CYW43  
#include "lwip/pbuf.h"           // Lightweight IP stack - manipulação de buffers de pacotes de rede
#include "lwip/tcp.h"            // Lightweight IP stack - fornece funções e estruturas para trabalhar com o protocolo TCP
#include "lwip/netif.h"          // Lightweight IP stack - fornece funções e estruturas para trabalhar com interfaces de rede (netif)


/* Credenciais WIFI - ADICIONE SUAS CREDENCIAIS - Tome cuidado se publicar no github! */
#define WIFI_SSID "SEU_SSID"        // Nome da rede Wi-Fi
#define WIFI_PASSWORD "SUA_SENHA"  // Senha da rede Wi-Fi

volatile bool door_locked = false;  // Door status (open or closed)
volatile char tags[3][24];

volatile bool tag_permission[3] = {false, false, false}; // Autorization status for each pet
volatile bool pet_inside[3] = {true, true, true};


#endif // SMART_PET_DOOR_H
