/*
 * Por: Paulo Cesar de Jesus Di Lauro
 *
 * Este código é uma adaptação do código original do
 * professor Wilton Lacerda Silva
 * para a utilização da matriz de LEDs WS2812 do BitDogLab.
 *
 * A seleção de LEDs acesos é feita por meio de um buffer de LEDs, onde
 * cada posição do buffer representa um LED da matriz 5x5.
 * Original em:
 * https://github.com/wiltonlacerda/EmbarcaTechU4C4/tree/72b3f5e897974c0957e3d281d8b548a674a1948d/06_ws2812_Escolha
 * 
 *                    Funcionamento do programa 
 *O programa mostra números de 0 a 9 na matriz de led 5x5, sendo o usuário capaz
 * de incrementar um número ao apertar o botão A e decrementar um numéro ao apertar
 *  o botão B da BitDogLab, como o led vermelho RGB (gpio 13) piscando 5 vezes por segundo.
 *                    Tratamento de deboucing com interrupção 
 * A ativação dos botões A e B são feitas através de uma rotina de interrupção, sendo
 * implementada condição para tratar o efeito boucing na rotina.
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
#define tempo 200 // para led vermelho piscar 5 vezes por segundo
#define Frames 10
#define LED_PIN_R 13
#define Botao_A 5 // gpio do botão A na BitDogLab
#define Botao_B 6 // gpio do botão B na BitDogLab

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 255;  // Intensidade do vermelho
uint8_t led_g = 0; // Intensidade do verde
uint8_t led_b = 0; // Intensidade do azul

//variáveis globais 
static volatile int aux = 5; // posição do numero impresso na matriz, inicialmente imprime numero 5
static volatile uint32_t last_time_A = 0; // Armazena o tempo do último evento para Bot A(em microssegundos)
static volatile uint32_t last_time_B = 0; // Armazena o tempo do último evento para Bot B(em microssegundos)

// Buffer para armazenar quais LEDs estão ligados matriz 5x5
bool led_buffer[NUM_PIXELS] = {
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0};

// Frames que formam os números de 0 a 9
bool buffer_Numeros[Frames][NUM_PIXELS] =
    {
      //{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24} referência para posição na BitDogLab matriz 5x5
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o número zero
        {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0}, // para o número 1
        {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0}, // para o número 2
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0}, // para o número 3
        {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0}, // para o número 4
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0}, // para o número 5
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0}, // para o número 6
        {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o número 7
        {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, // para o número 8
        {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}  // para o número 9
};

void atualiza_buffer(bool buffer[], bool b[][NUM_PIXELS], int c); ///protótipo função que atualiza buffer


static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b);//liga os LEDs escolhidos 

void gpio_irq_handler(uint gpio, uint32_t events);// protótipo função interrupção para os botões A e B com condição para deboucing

int main()
{
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    // configuração led RGB vermelho
    gpio_init(LED_PIN_R);              // inicializa pino do led vermelho
    gpio_set_dir(LED_PIN_R, GPIO_OUT); // configrua como saída
    gpio_put(LED_PIN_R, 0);            // led inicia apagado

    // configuração botão A
    gpio_init(Botao_A);             // inicializa pino do botão A
    gpio_set_dir(Botao_A, GPIO_IN); // configura como entrada
    gpio_pull_up(Botao_A);          // Habilita o pull-up interno

    // configuração botão B
    gpio_init(Botao_B);             // inicializa pino do botão B
    gpio_set_dir(Botao_B, GPIO_IN); // configura como entrada
    gpio_pull_up(Botao_B);          // Habilita o pull-up interno

    atualiza_buffer(led_buffer, buffer_Numeros, aux); // atualiza buffer para numero 5
    set_one_led(led_r, led_g, led_b);               // forma numero 5 primeira vez

    // configurando a interrupção com botão na descida para o botão A
    gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // configurando a interrupção com botão na descida para o botão B
    gpio_set_irq_enabled_with_callback(Botao_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (1)
    {

        gpio_put(LED_PIN_R, 1); // liga led
        sleep_ms(tempo / 2);    // mantém ligado por 100   ms
        gpio_put(LED_PIN_R, 0); // desliga o led
        sleep_ms(tempo / 2);    // mantém desligado por 100   ms, totalizamdo 200 ms de espera até ligar novamente (pisca 5 vezes por segundo)

    }

    return 0;
}

// função que atualiza o buffer de acordo o número de 0 a 9
void atualiza_buffer(bool buffer[],bool b[][NUM_PIXELS], int c)
{
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        buffer[i] = b[c][i];
    }
}

void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}

// função interrupção para os botões A e B com condição para deboucing
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());//// Obtém o tempo atual em microssegundos
    if (gpio_get(Botao_A) == 0 && aux < Frames - 1 && (current_time - last_time_A) > 200000)//200ms de boucing adiconado como condição 
    { // se botão A for pressionado e aux<9 incrementa aux em 1(próximo número) 
        last_time_A = current_time; // Atualiza o tempo do último evento
        aux++;
        atualiza_buffer(led_buffer, buffer_Numeros, aux); /// atualiza buffer
        set_one_led(led_r, led_g, led_b);               // forma numero na matriz
    }
    if (gpio_get(Botao_B) == 0 && aux > 0 && (current_time - last_time_B) > 200000)//200ms de boucing adiconado como condição 
    { // se botão B for pressionado e aux>0 decrementa aux em 1(número anterior)
        last_time_B = current_time; // Atualiza o tempo do último evento
        aux--;
        atualiza_buffer(led_buffer, buffer_Numeros, aux); // atualiza buffer
        set_one_led(led_r, led_g, led_b);               // forma número na matriz
    }
}