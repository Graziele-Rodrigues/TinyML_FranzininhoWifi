

# Pulseira para Detecção de Crises Epilépticas

Este projeto é uma prova de conceito que utiliza a Franzininho WiFi, um acelerômetro LIS3DH e o serviço Edge Impulse para detectar padrões de movimento associados a crises epilépticas. Quando uma crise é identificada, uma mensagem de emergência é enviada via WhatsApp para um contato pré-cadastrado.

## Requisitos de Hardware
- 1 Franzininho WiFi
- 1 Acelerômetro LIS3DH
- Jumpers para conexões
- Acesso a uma rede WiFi

## Bibliotecas Utilizadas
- [Adafruit LIS3DH](https://github.com/adafruit/Adafruit_LIS3DH)
- [Edge Impulse Inference](https://github.com/edgeimpulse/example-standalone-inferencing)
- [WiFi](https://www.arduino.cc/en/Reference/WiFi)
- [HTTPClient](https://www.arduino.cc/en/Reference/HTTPClient)

## Funcionalidades
- **Leitura de dados do acelerômetro**: Captura de aceleração nos eixos X, Y, Z e cálculo de ângulos (roll, pitch, yaw).
- **Classificação de movimentos**: O modelo de machine learning, treinado no Edge Impulse, classifica os movimentos capturados em "com crise" ou "sem crise".
- **Envio de alerta**: Se uma crise for detectada, o sistema envia automaticamente uma mensagem de emergência via WhatsApp para o contato cadastrado.

## Configuração do Projeto

1. **Conexão do acelerômetro**:
   - Conecte o acelerômetro LIS3DH aos pinos da Franzininho WiFi conforme a pinagem SPI definida no código:
     - `LIS3DH_CLK` → Pino 36
     - `LIS3DH_MISO` → Pino 37
     - `LIS3DH_MOSI` → Pino 35
     - `LIS3DH_CS` → Pino 34

2. **Credenciais de WiFi**:
   - Atualize as credenciais da sua rede WiFi no código:
     ```cpp
     const char* ssid = "SEU_SSID";
     const char* password = "SUA_SENHA";
     ```

3. **Configuração do contato de emergência**:
   - Insira o número de telefone e a chave API do CallMeBot para o envio de mensagens via WhatsApp:
     ```cpp
     String phoneNumber = "+SEU_NUMERO";
     String apiKey = "SUA_API_KEY";
     ```

## Como Usar

1. **Treinamento do modelo**:
   - Treine um modelo de machine learning no Edge Impulse utilizando dados de movimentos comuns e de simulações de crises epilépticas.
   - Exporte o modelo e inclua-o no código com a biblioteca `epilepsia_inferencing.h`.

2. **Execução**:
   - Carregue o código na Franzininho WiFi.
   - O sistema irá classificar os dados capturados do acelerômetro e, em caso de detecção de crise, enviará um alerta via WhatsApp.

## Licença
Este projeto está licenciado sob a licença MIT.


