# Sistema IoT de Monitoramento da Qualidade da Água

## Descrição
Este projeto implementa um sistema IoT de baixo custo para monitoramento da qualidade da água em tempo real, utilizando ESP32, sensores de pH e turbidez, com transmissão de dados via protocolo MQTT.

## Objetivos
- Monitorar parâmetros de qualidade da água (pH e turbidez)
- Transmitir dados em tempo real via MQTT
- Acionar uma válvula solenoide quando os parâmetros estiverem fora dos limites aceitáveis
- Contribuir para o ODS 6 (Água Potável e Saneamento) da ONU

## Componentes Necessários
- ESP32 DevKit V1
- Sensor de pH E-201 (DFRobot)
- Sensor de Turbidez SEN0189
- Módulo Relé
- Válvula Solenoide 12V
- Cabos e protoboard

## Instruções de Montagem
1. Conecte o sensor de pH ao pino GPIO36 do ESP32
2. Conecte o sensor de turbidez ao pino GPIO39 do ESP32
3. Conecte o módulo relé ao pino GPIO23 do ESP32
4. Conecte a válvula solenoide ao módulo relé
5. Alimente o ESP32 e a válvula com as fontes apropriadas

## Instruções de Uso
1. Clone este repositório
2. Instale as bibliotecas necessárias descritas no arquivo `sistema_monitoramento_agua.ino`
3. Configure suas credenciais Wi-Fi no arquivo `sistema_monitoramento_agua.ino`
4. Carregue o código para o ESP32
5. Calibre os sensores 
6. Utilize um cliente MQTT para visualizar os dados

## Comunicação MQTT
- Broker: HiveMQ (broker.hivemq.com)
- Tópicos:
  - agua/ph: Valor atual do pH
  - agua/turbidez: Valor atual da turbidez (NTU)
  - agua/valvula: Estado atual da válvula (ABERTA/FECHADA)

## Resultados
Os resultados dos testes e medições estão disponíveis no artigo do projeto

## Vídeo Demonstrativo
https://youtu.be/v0RIwP7N0Bg

## Autor
Danilo Diniz Silva

## Licença
Este projeto está licenciado sob a licença MIT - veja o arquivo LICENSE para detalhes.
```



