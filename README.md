# Alerta de Radares

Código para um dispositivo dedicado que te indica a existência de um radar na rodovia, te alertando caso sua velocidade exceda a velocidade permitida.

Utiliza as informações de radar disponíveis em: [maparadar.com](maparadar.com)

#### Itens necessários:
- ESP8266
- Módulo GPS NEO6M
- Display OLED SSD1306
- Buzzer
- ✨Bruxaria ✨

#### Guia

- Primeiro precisamos obter os dados atualizados sobre os radares brasileiros. Acesse [maparadar.com](maparadar.com) e exporte os dados para **IGO8\Amigo**, marcando os tipos de alerta desejados e a checkbox de direção.

- Agora que temos o arquivo **maparadar.txt**, vamos pré processar esses dados antes de enviar para o ESP8266 utilizando o scipt Python. Coloque o arquivo no mesmo diretório que o script e o execute. Assim será gerado um novo arquivo **sorted_maparadar.txt**, ordenado por longitude.

- Salvaremos esse arquivo na memória SPIFFS do ESP8266. Siga [este guia](https://www.instructables.com/Using-ESP8266-SPIFFS/).

- Agora podemos conectar os módulos e fazer o upload do sketch **radar.ino** para a placa.


## Setting Up the Environment

After cloning the project, follow these steps to configure your environment:

1. **Create a `.env` file**:  
   In the root of the project, create a `.env` file (if it doesn't exist).

2. **Add the environment variables**:  
   Inside the `.env` file, add your radar service username (and any other required credentials or tokens).  
   Example:

   ```env
   RADAR_USER=your_username_here