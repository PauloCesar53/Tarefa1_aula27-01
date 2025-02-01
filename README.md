# Números de 0 a 9 na matriz de LED da BitDogLab 

## Descrição
Este projeto utiliza os botões A e B da BitDogLab para controlar os números de 0 a 9 na matriz de LED, com o led vermelho piscando 5 vezes por segundo 

---

## Funcionalidades
1. O LED vermelho do LED RGB deve piscar continuamente 5 vezes por segundo.
2. O botão A deve incrementar o número exibido na matriz de LEDs cada vez que for pressionado.
3. O botão B deve decrementar o número exibido na matriz de LEDs cada vez que for pressionado.
4. Os LEDs WS2812 são usados para criar efeitos visuais representando números de 0 a 9.


---

## Requisitos

Requisitos do Projeto
1. Uso de interrupções: Todas as funcionalidades relacionadas aos botões foram implementadas
utilizando rotinas de interrupção (IRQ).
2. Debouncing: Foi feito o tratamento do bouncing dos botões via software.
3. Controle de LEDs: O projeto inclui o uso de LEDs comuns (LED vermelho RGB) e LEDs WS2812 (matriz de LEDs)

## Configuração

### Mapeamento dos Botões

| Botão       | Função                               |
|-------------|--------------------------------------|
| **Botão A** | incrementa o número na matriz de LEDs|
| **Botão B** | decrementa o número na matriz de LEDs|

## Compilação e Execução

1. Certifique-se de que o SDK do Raspberry Pi Pico está configurado no seu ambiente.
2. Compile o programa utilizando a extensão **Raspberry Pi Pico Project** no VS Code:
   - Abra o projeto no VS Code.
   - Vá até a extensão do **Raspberry pi pico project** e clique em **Compile Project**.
3. Coloque a placa em modo BOOTSEL e copie o arquivo `ws2812.uf2`, que está na pasta build, para a BitDogLab conectado via USB.

---

## Demonstração no YouTube

Confira a demonstração completa deste projeto no YouTube: (https://youtube.com/shorts/jfCe6ezIlYM?si=kFjhQT5BCX9zyJDC)

---

## Emulação com Wokwi

Para testar o programa sem hardware físico, você pode utilizar o **Wokwi** para emulação no VS Code:

1. Instale a extensão **Wokwi for VS Code**.
2. Inicie a emulação:
   - Clique no arquivo `diagram.json` e inicie a emulação.
4. Teste o funcionamento do programa diretamente no ambiente emulado.

---

## Colaboradores
- [PauloCesar53 - Paulo César de Jesus Di Lauro ] (https://github.com/PauloCesar53)
