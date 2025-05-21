#include "smart_pet_door.h"

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
void user_request(char **request);


// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (!p){
        tcp_close(tpcb);
        tcp_recv(tpcb, NULL);
        return ERR_OK;
    }

    // Alocação do request na memória dinámica
    char *request = (char *)malloc(p->len + 1);
    memcpy(request, p->payload, p->len);
    request[p->len] = '\0';

    printf("Request: %s\n", request);

    // Call request handler
    user_request(&request);

    // Cria a resposta HTML
    char html[1080]; // Buffer para armazenar a resposta HTML

    // Instruções html do webserver
    snprintf(html, sizeof(html), // Formatar uma string e armazená-la em um buffer de caracteres
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "\r\n"
             "<!DOCTYPE html>\n"
             "<html>\n"
             "<head>\n"
             "<title>Smart PetDoor</title>\n"
             "<style>\n"
             "body { background-color: #4B6382; font-family: Monaco, monospace; text-align: center; margin-top: 50px; }\n"
             "h1 { font-size: 48px; margin-bottom: 30px; }\n"
             "button { background-color: #CDD5DB ;width: 200px; padding: 10px 20px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px}\n"
             "p {background-color: #a4b5c4; width: 350px; padding: 10px 20px; margin: 10px; border: none; border-radius: 2px; cursor: pointer; font-size: 16px; display: absolute; margin: 0 auto}\n"           
             "</style>\n"
             "</head>\n"
             "<body>\n"
             "<h1>Smart PetDoor</h1>\n"
             "<form action=\"./control_door\"><button>Trancar/destrancar porta</button></form>\n"
             "<form action=\"./tag1\"><button>Tag 1</button></form>\n"
             "<form action=\"./tag2\"><button>Tag 2</button></form>\n"
             "<form action=\"./tag3\"><button>Tag 3</button></form>\n"
             "<div style=\"text-align: center;\">\n"
             "<p>Status da porta: %s</p>\n"
             "<p>Tag 1: %s</p>\n"
             "<p>Tag 2: %s</p>\n"
             "<p>Tag 3: %s</p>\n"
              "</div>\n"
             "</body>\n"
             "</html>\n",
             door_locked ? "Trancada" : "Destrancada", tag_permission[0] ? "Sim" : "-", tag_permission[1] ? "Sim" : "-", tag_permission[2] ? "Sim" : "-");

    // Escreve dados para envio (mas não os envia imediatamente).
    tcp_write(tpcb, html, strlen(html), TCP_WRITE_FLAG_COPY);

    // Envia a mensagem
    tcp_output(tpcb);

    //libera memória alocada dinamicamente
    free(request);
    
    //libera um buffer de pacote (pbuf) que foi alocado anteriormente
    pbuf_free(p);

    return ERR_OK;
}

// Função de callback ao aceitar conexões TCP
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

// Tratamento do request do usuário - digite aqui
void user_request(char **request) {

    if (strstr(*request, "GET /control_door") != NULL) {
        door_locked = !door_locked; // Trancar a porta
    
    }else if (strstr(*request, "GET /tag1") != NULL) {
        tag_permission[0] = !tag_permission[0]; // Autorizar/desautorizar pet 1

    }else if (strstr(*request, "GET /tag2") != NULL) {
       tag_permission[1] = !tag_permission[1]; // Autorizar/desautorizar pet 2
       
    }else if (strstr(*request, "GET /tag3") != NULL) {
       tag_permission[2] = !tag_permission[2]; // Autorizar/desautorizar pet 3
    }
};



// Função principal
int main() {
    //Inicializa todos os tipos de bibliotecas stdio padrão presentes que estão ligados ao binário.
    stdio_init_all();

    //Inicializa a arquitetura do cyw43
    while (cyw43_arch_init()) {
        printf("Falha ao inicializar Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }


    // Ativa o Wi-Fi no modo Station, de modo a que possam ser feitas ligações a outros pontos de acesso Wi-Fi.
    cyw43_arch_enable_sta_mode();

    // Conectar à rede WiFI - fazer um loop até que esteja conectado
    printf("Conectando ao Wi-Fi...\n");
    while (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 20000)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        sleep_ms(100);
        return -1;
    }
    printf("Conectado ao Wi-Fi\n");

    // Caso seja a interface de rede padrão - imprimir o IP do dispositivo.
    if (netif_default) {
        printf("IP do dispositivo: %s\n", ipaddr_ntoa(&netif_default->ip_addr));
    }

    // Configura o servidor TCP - cria novos PCBs TCP. É o primeiro passo para estabelecer uma conexão TCP.
    struct tcp_pcb *server = tcp_new();
    if (!server) {
        printf("Falha ao criar servidor TCP\n");
        return -1;
    }

    //vincula um PCB (Protocol Control Block) TCP a um endereço IP e porta específicos.
    if (tcp_bind(server, IP_ADDR_ANY, 80) != ERR_OK) {
        printf("Falha ao associar servidor TCP à porta 80\n");
        return -1;
    }

    // Coloca um PCB (Protocol Control Block) TCP em modo de escuta, permitindo que ele aceite conexões de entrada.
    server = tcp_listen(server);

    // Define uma função de callback para aceitar conexões TCP de entrada. É um passo importante na configuração de servidores TCP.
    tcp_accept(server, tcp_server_accept);
    printf("Servidor ouvindo na porta 80\n");

    
    float distance = 0.0;
    int64_t echo_duration = 0;
    uint slice;
    bool open_door = false;

    hcsr04_init(); // Inicializa o sensor ultrassônico
    slice = servomotor_setup(); // Inicializa o servomotor
    servomotor_set_position(slice, 65536/2);

    gpio_init(11); // Inicializa o LED
    gpio_set_dir(11, GPIO_OUT); // Configura o LED como saída
    gpio_put(11, 0); // Liga o LED
    
    
    while(1){
        cyw43_arch_poll();

        echo_duration = hcsr04_get_echo_duration();

        if (echo_duration > 0) {
            distance = hcsr04_calculate_distance(echo_duration);
        }

        if (distance < 20.0 && !door_locked && stdio_usb_connected()) {
            // Leitura da coleira (simulado)
            printf("Select tag (1-3): ");
            int tag = getchar_timeout_us(2000000);
            printf("----\n");
            
            if (tag != PICO_ERROR_TIMEOUT) {
                uint8_t tag_index = tag - '0' - 1; // Converte o caractere para índice
                open_door |= (tag_index < 3 && tag_permission[tag_index]);
            }
        }else if(distance > 20.0) {
            open_door = false;
        }

        if (open_door) {
            servomotor_set_position(slice, 6000);
            gpio_put(11, 1);
        } else {
            servomotor_set_position(slice, 1250);
            gpio_put(11, 0);
        }
        
        // Aguarda 100ms antes de verificar novamente
        sleep_ms(100);
    }
    
    // Se o agendador do FreeRTOS falhar, o código abaixo será executado.
    cyw43_arch_deinit(); // Desliga a arquitetura CYW43
    panic_unsupported();

    return 0;
}
