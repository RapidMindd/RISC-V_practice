.global _start
.option norelax

_start:
	li a7, 63
	li a0, 0
	la a1, name
	li a2, 100
	ecall
	mv t0, a0
	addi t0, t0, -1


	li a7, 63
	li a0, 0
	la a1, group_number
	li a2, 100
	ecall
	mv t1, a0

	li a7, 64
	li a0, 1
	la a1, hello_
	li a2, 12
	ecall
	
	li a7, 64
	li a0, 1
	la a1, name
	mv a2, t0
	ecall

	li a7, 64
	li a0, 1
	la a1, from_group_
	li a2, 12
	ecall

	li a7, 64
	li a0, 1
	la a1, group_number
	mv a2, t1
	ecall

	li a7, 64
	li a0, 1
	la a1, first_string_
	li a2, 2
	ecall

	li a7, 64
	li a0, 1
	la a1, second_string_
	li a2, 3
	ecall

	li a7, 64
	li a0, 1
	la a1, third_string_
	li a2, 4
	ecall

	li a7, 93
	li a0, 0
	ecall

.data
name: .space 100
group_number: .space 100
hello_: .ascii "Hello, I am "
from_group_: .ascii " from group "
first_string_: .ascii "#\n"
second_string_: .ascii "##\n"
third_string_: .ascii "###\n"
