# exit.s    
    .section    __TEXT,__text
    .globl  _main
_main:
    movq    $2, %rax
    cmpq    $10, %rax
    jl lbl1
    movq    $1, %rax
lbl1:
    ret

