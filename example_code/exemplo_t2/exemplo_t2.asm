.data
primos: .word 1, 3, 5, 7, 11, 13, 17, 19
size: 	.word 8
msg: 	.asciz "Os oito primeiros numeros primos sao : "
space: 	.ascii " "
.text
	la t0, primos # carrega endereço inicial do array
	la t1, size # carrega endereço de size
	lw t1, 0(t1) # carrega size em t1
	li a7, 4 # imprime mensagem inicial
	la a0, msg
	ecall
loop: 
	beq t1, zero, exit # se processou todo o array, encerra
	li a7, 1 # serviço de impressão de inteiros
	lw a0, 0(t0) # inteiro a ser exibido
	ecall
	li a7, 4 # imprime separador
	la a0, space
	ecall
	addi t0, t0, 4 # incrementa indice array
	addi t1, t1, -1 # decrementa contador
	j loop # novo loop
exit: 
	li a7, 10
	ecall
