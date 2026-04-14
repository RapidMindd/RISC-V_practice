.section .data
    prompt_num:   .asciz "Enter list number: "
    prompt_grp:   .asciz "Enter last digit of group: "
    hello_msg:    .asciz "Hello, I am Student from group XX\n"
    newline:      .asciz "\n"

    # Символы
    sym_at:       .ascii "@"
    sym_hash:     .ascii "#"
    sym_dollar:   .ascii "$"
    sym_percent:  .ascii "%"

.section .bss
    buffer:       .space 8

.section .text
.globl _start

_start:
    # При сборке с -nostdlib регистр gp нужно инициализировать вручную,
    # иначе обращения к данным через la могут указывать в неверный адрес.
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # 1. Приветствие
    li a0, 1
    la a1, hello_msg
    li a2, 34
    li a7, 64
    ecall

    # 2. Номер в списке
    li a0, 1
    la a1, prompt_num
    li a2, 19
    li a7, 64
    ecall

    li a0, 0
    la a1, buffer
    li a2, 2
    li a7, 63
    ecall
    lb t0, 0(a1)
    andi t0, t0, 1      # t0 = 0 (чет), 1 (нечет)

    # 3. Цифра группы
    li a0, 1
    la a1, prompt_grp
    li a2, 27
    li a7, 64
    ecall

    li a0, 0
    la a1, buffer
    li a2, 2
    li a7, 63
    ecall
    lb t1, 0(a1)
    addi t1, t1, -48    # ASCII -> число
    andi t1, t1, 3      # t1 % 4

    # Выбор символа
    la s1, sym_at
    li t2, 1
    beq t1, t2, set_hash
    li t2, 2
    beq t1, t2, set_dollar
    li t2, 3
    beq t1, t2, set_percent
    j draw_choice

set_hash:    la s1, sym_hash;    j draw_choice
set_dollar:  la s1, sym_dollar;  j draw_choice
set_percent: la s1, sym_percent; j draw_choice

draw_choice:
    bnez t0, draw_triangle

# --- Квадрат 3x3 ---
draw_square:
    li s3, 3            # 3 строки
sq_row:
    li s4, 3            # 3 символа в строке
sq_char:
    li a0, 1
    mv a1, s1
    li a2, 1
    li a7, 64
    ecall
    addi s4, s4, -1
    bnez s4, sq_char

    # Новая строка
    li a0, 1
    la a1, newline
    li a2, 1
    li a7, 64
    ecall
    addi s3, s3, -1
    bnez s3, sq_row
    j exit

# --- Треугольник (прямоугольный) ---
draw_triangle:
    li s3, 1            # Начинаем с 1 символа
tr_row:
    mv s4, s3           # Печатаем s3 символов
tr_char:
    li a0, 1
    mv a1, s1
    li a2, 1
    li a7, 64
    ecall
    addi s4, s4, -1
    bnez s4, tr_char

    li a0, 1
    la a1, newline
    li a2, 1
    li a7, 64
    ecall
    addi s3, s3, 1
    li t2, 4
    blt s3, t2, tr_row

exit:
    li a0, 0
    li a7, 93
    ecall
