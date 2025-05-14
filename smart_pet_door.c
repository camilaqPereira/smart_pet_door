#include "smart_pet_door.h"

static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
void user_request(char **request);


// Função de callback para processar requisições HTTP
static err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
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
    char html[1050]; // Buffer para armazenar a resposta HTML

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
             "<form action=\"./unlock_door\"><button>Destrancar porta</button></form>\n"
             "<form action=\"./lock_door\"><button>Trancar porta</button></form>\n"
             "<form action=\"./add_pet\"><button>Cadastrar pet</button></form>\n"
             "<form action=\"./get_status\"><button>Atualizar status</button></form>\n"
             "<div style=\"text-align: center;\">\n"
             "<p>Status da porta: %s</p>\n"
             "<p>Tag 1: %s</p>\n"
             "<p>Tag 2: %s</p>\n"
             "<p>Tag 3: %s</p>\n"
              "</div>\n"
             "</body>\n"
             "</html>\n",
             door_locked ? "Trancada" : "Destrancada", pet_inside[0] ? "Dentro" : "Fora", pet_inside[1] ? "Dentro" : "Fora", pet_inside[2] ? "Dentro" : "Fora");

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
void user_request(char **request){

    if (strstr(*request, "GET /lock_door") != NULL) {
        door_locked = true; // Trancar a porta
    
    }else if (strstr(*request, "GET /unlock_door") != NULL) {
        door_locked = false; // Destrancar a porta
    
    }else if (strstr(*request, "GET /get_status") != NULL) {
       // Força atualização da pagina
       
    }else if (strstr(*request, "GET /add_pet") != NULL) {
        printf("Adicionando pet...\n");
        //fflush(stdin);
        //scanf("%s", tags[0]); // BUG
        printf("Tag cadastrada: %s\n", tags[0]);
        tag_permission[0] = true; // Autorizar o pet
    }
};


// Função principal
int main()
{
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


    while (true){
        /* 
        * Efetuar o processamento exigido pelo cyw43_driver ou pela stack TCP/IP.
        * Este método deve ser chamado periodicamente a partir do ciclo principal 
        * quando se utiliza um estilo de sondagem pico_cyw43_arch 
        */
        cyw43_arch_poll(); // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);      // Reduz o uso da CPU
    }

    //Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}
