# Trabalho 2 - Simulador RV32I - OAC
Código e artefatos escritos por Luiz Carlos Schonarth Junior, matrícula 19/0055171, Departamento de Ciência da Computação, UnB.

## Informações do Sistema
- SO: Manjaro Linux x86_63
- Kernel: 5.10.59-1-MANJARO
- g++/gcc versão 11.1.0

## Compilando o simulador

Dentro de um terminal, mude para o diretório raiz deste projeto e execute o comando:

```
g++ ./src/runtime.cpp ./src/riscv.cpp ./src/main.cpp -o simulator
```

Verifique que sua máquina tenha o compilador g++ instalado.

## Executando o simulador

Após a compilação execute:

```
./simulador /path/to/code.bin /path/to/data.bin
```

Sendo `code.bin` e `data.bin` os arquivos binários gerados pelo RARS.

### Exemplo de uso

Para usar o código testador do simulador, execute:

```
./simulador ./example_code/testador/code.bin ./example_code/testador/data.bin
```
