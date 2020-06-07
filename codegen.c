#include "10cc.h"

int num_argregs = 6;
char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

int label_cnt = 0;

void gen_lval(Node *node) {
    if(node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n"); // ベースポインタの値をraxに読み込み
    printf("  sub rax, %d\n", node->offset); // raxをoffsetだけ移動
    printf("  push rax\n"); // local variable のアドレスをスタックにpush
}

// スタックから入力を受け取り、nodeが表す演算の結果をスタックに戻す処理を生成する
void gen(Node *node) {
    int cur_label_cnt;
    switch(node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_FUNC_CALL:
        for(int i = 0; i < node->args->len; i++) {
            gen(node->args->data[i]);
        }
        for(int i = node->args->len - 1; 0 <= i; i--) {
            printf("  pop %s\n", argregs[i]);
        }
        printf("  call %s\n", node->name);
        printf("  push rax\n");
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n"); // local variable のアドレスをraxにpop
        printf("  mov rax, [rax]\n"); // raxの指す先にアクセスして中身をraxにコピー
        printf("  push rax\n"); // コピーしてきた値をスタックにpush
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");        // 右辺値
        printf("  pop rax\n");        // 左辺値
        printf("  mov [rax], rdi\n"); // rdiの値をraxが指すメモリにコピー
        printf("  push rdi\n");       // 代入演算の結果を書き戻す
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
        if(node->els) {
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
    case ND_FOR:
        gen(node->init);
        cur_label_cnt = label_cnt++;
        printf(".Lbegin%03d:\n", cur_label_cnt);
        gen(node->cond);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%03d\n", cur_label_cnt);
        gen(node->then);
        gen(node->upd);
        printf("  jmp .Lbegin%03d\n", cur_label_cnt);
        printf(".Lend%03d:\n", cur_label_cnt);
        return;
    case ND_BLOCK:
        for(int i = 0; i < node->stmts->len; i++) {
            gen(node->stmts->data[i]);
            printf("  pop rax\n");
        }
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch(node->kind) {
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

void gen_func(Node *func) {
    printf("\n%s:\n", func->name);

    // prologue
    // ローカル変数の領域を確保する
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", func->lvars->len * 8);

    // 引数の値を stack に push してローカル変数と同じように扱えるように
    for(int i = 0; i < func->args->len; i++) {
        printf("  mov rax, rbp\n"); // ベースポインタの値をraxに読み込み
        Vector *args = func->args;
        LVar *arg = args->data[i];
        // LVar *arg_by_map = get_elem_from_map(node->lvars, arg->name);
        printf("  sub rax, %d\n", arg->offset); // raxをoffsetだけ移動
        printf("  mov [rax], %s\n", argregs[i]); // 第 i 引数の値をraxが指すメモリにコピー
    }

    // 中身のコードを吐く
    gen(func->impl);
}

void gen_x86() {
    // header
    printf(".intel_syntax noprefix\n");
    printf(".global %s", code[0]->name);
    for(int i = 1; code[i]; i++) {
        printf(", %s", code[i]->name);
    }
    printf("\n");

    // 関数ごとにコードを生成
    for(int i = 0; code[i]; i++) {
        gen_func(code[i]);
    }
}
