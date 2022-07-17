# Network-Sockets-cpp
## Membros
- Caio Brandolim Rovetta     | N USP:11232156
- Calvin Suzuki de Camargo   | N USP:11232420
- Guilherme Soares Silvestre | N USP:11299832
## Descrição
A aplicação foi projetada e testada em máquinas operando com o Ubuntu 20.04 e Manjaro. Para a compilação foram utilizadas as configurações no MakeFile, o qual usa o G++.
## Como compilar?
Configuramos o ``Makefile`` para facilitar a compilação dos arquivos, que no momento, envolvem somente dois documentos ``*.cpp`` : ``server.cpp`` e ``client.cpp``.

Para compilar e executar o Servidor: 

```
make sv
```

Para compilar e executar o Cliente: 

```
make client
```
## Funcionamento do programa

Precisamos executar o servidor ANTES do cliente para a conexão acontecer.

 **1.** O *server* inicialmente espera uma primeira mensagem do *client* : 
```
=> Enter '$' at the end of the message
Waiting client message...
```
 **2.** *Client* faz o envio da mensagem e espera uma resposta do *server*:
```
Client: oi $
Waiting server message...
```
 **3.** No terminal do *server*, a mensagem do *client* é exibida e recomeça o ciclo de troca de mensagens:
```
=> Enter '$' at the end of the message
Waiting client message...
Client: oi $ 
Server: ola $      
Waiting client message...
```

## Considerações 

Podemos observar que no momento atual, o trabalho deu ênfase na comunicação básica via *socket*, infelizmente, não demos a profundidade de criar a fluidez de um *chat* normal, i.e., uma recepção e envio de mensagens livre entre as máquinas. Sempre que um fala, o próximo precisa responder.