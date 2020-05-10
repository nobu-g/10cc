#include "9cc.h"


int label_cnt = 0;

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");  // ベースポインタの値をraxに読み込み
    printf("  sub rax, %d\n", node->offset);  // raxをoffsetだけ移動
    printf("  push rax\n");  // local variable のアドレスをスタックにpush
}

void gen(Node *node) {
    int cur_label_cnt;
    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");  // local variable のアドレスをraxにpop
        printf("  mov rax, [rax]\n");  // raxの指す先にアクセスして中身をraxにコピー
        printf("  push rax\n");  // コピーしてきた値をスタックにpush
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");  // 右辺値
        printf("  pop rax\n");  // 左辺値
        printf("  mov [rax], rdi\n");  // rdiの値をraxが指すメモリにコピー
        printf("  push rdi\n");  // 代入演算の結果を書き戻す
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_IF:
        cur_label_cnt = label_cnt++;
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        if (node->els) {
            printf("  je .Lelse%03d\n", cur_label_cnt);
            gen(node->then);
            printf("  jmp .Lend%03d\n", cur_label_cnt);
            printf(".Lelse%03d:\n", cur_label_cnt);
            gen(node->els);
        } else {
            printf("  je .Lend%03d\n", cur_label_cnt);
            gen(node->then);
        }
        printf(".Lend%03d:\n", cur_label_cnt);
        return;
    case ND_WHILE:
        cur_label_cnt = label_cnt++;
        printf(".Lbegin%03d:\n", cur_label_cnt);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", cur_label_cnt);
        gen(node->then);
        printf("  jmp .Lbegin%03d\n", cur_label_cnt);
        printf(".Lend%03d:\n", cur_label_cnt);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }

    printf("  push rax\n");
}
