#include "10cc.h"

char *regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
char *regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
char *regs32[] = {"r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"};

int num_regs = sizeof(regs) / sizeof(*regs);

char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *argregs8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *argregs32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

int label_cnt = 0;

// 一時的に常に0番目のレジスタを返す
char *node_to_reg(Node *node) {
    switch (node->ty->ty) {
    case PTR:
    case ARRAY:
        return regs[0];
    case INT:
        return regs32[0];
    case CHAR:
        return regs8[0];
    }
}

char *node_to_argreg(Node *node, int i) {
    switch (node->ty->ty) {
    case PTR:
    case ARRAY:
        return argregs[i];
    case INT:
        return argregs32[i];
    case CHAR:
        return argregs8[i];
    }
}

// node が指す local variable の "スタック上のアドレス" をスタックに push する
void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("ローカル変数ではありません: %d", node->kind);
    }
    printf("  lea rax, [rbp-%d]\n", node->offset);
    printf("  push rax\n"); // local variable のアドレスをスタックにpush
}

// node が指す global variable の "メモリ上のアドレス" をスタックに push する
void gen_gval(Node *node) {
    if (node->kind != ND_GVAR) {
        error("グローバル変数ではありません: %d", node->kind);
    }
    printf("  lea rax, %s\n", node->name);
    printf("  push rax\n"); // global variable のアドレスをスタックにpush
}

// スタックから入力を受け取り、nodeが表す演算の結果をスタックに戻す処理を生成する
void gen(Node *node) {
    int cur_label_cnt;
    switch (node->kind) {
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_FUNC_CALL:
        for (int i = 0; i < node->args->len; i++) {
            gen(node->args->data[i]);
        }
        for (int i = node->args->len - 1; 0 <= i; i--) {
            printf("  pop %s\n", argregs[i]);
        }
        printf("  call %s\n", node->name);
        printf("  push rax\n");
        return;
    case ND_GVAR:
        gen_gval(node);
        printf("  pop rax\n");        // local variable のアドレスをraxにpop
        printf("  mov rax, [rax]\n"); // raxの指す先にアクセスして中身をraxにコピー
        printf("  push rax\n");       // コピーしてきた値をスタックにpush
        return;
    case ND_LVAR: // node にアクセスして中身をスタックに push
        gen_lval(node);
        printf("  pop rax\n");                          // local variable のアドレスをraxにpop
        printf("  mov %s, [rax]\n", node_to_reg(node)); // raxの指す先にアクセスして中身をraxにコピー
        printf("  push %s\n", regs[0]);                 // コピーしてきた値をスタックにpush
        return;
    case ND_ASSIGN:
        if (node->lhs->kind == ND_DEREF) {
            gen(node->lhs->lhs); // node->lhs->lhs: ポインタ型 local variable
        } else if (node->lhs->kind == ND_LVAR) {
            gen_lval(node->lhs);
        } else if (node->lhs->kind == ND_GVAR) {
            gen_gval(node->lhs);
        } else {
            error("有効な左辺値ではありません．");
        }
        gen(node->rhs);
        printf("  pop %s\n", regs[0]);                       // 右辺値
        printf("  pop rax\n");                               // 左辺値(アドレス)
        printf("  mov [rax], %s\n", node_to_reg(node->rhs)); // rdiの値をraxが指すメモリにコピー
        printf("  push %s\n", regs[0]);                      // 代入演算の結果を書き戻す
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
        printf("  pop %s\n", regs[0]);
        printf("  cmp %s, 0\n", regs[0]);
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
        printf("  pop %s\n", regs[0]);
        printf("  cmp %s, 0\n", regs[0]);
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
        printf("  pop %s\n", regs[0]);
        printf("  cmp %s, 0\n", regs[0]);
        printf("  je .Lend%03d\n", cur_label_cnt);
        gen(node->then);
        gen(node->upd);
        printf("  jmp .Lbegin%03d\n", cur_label_cnt);
        printf(".Lend%03d:\n", cur_label_cnt);
        return;
    case ND_BLOCK:
        for (int i = 0; i < node->stmts->len; i++) {
            gen(node->stmts->data[i]);
            printf("  pop %s\n", regs[0]);
        }
        return;
    case ND_ADDR:
        if (node->lhs->kind == ND_LVAR) {
            gen_lval(node->lhs);
        } else if (node->lhs->kind == ND_GVAR) {
            gen_gval(node->lhs);
        } else {
            error("アドレスを参照できません");
        }
        return;
    case ND_DEREF:      // 右辺値参照
        gen(node->lhs); // アドレスがスタックに積まれる
        printf("  pop %s\n", "rax");
        printf("  mov %s, [%s]\n", node_to_reg(node->lhs), "rax");
        printf("  push %s\n", regs[0]);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n"); // rhs
    printf("  pop rax\n"); // lhs

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

void gen_gvar(Node *g) {
    printf("%s:\n", g->name);
    printf("  .zero %d\n", g->ty->size);
}

void gen_func(Func *f) {
    printf(".global %s\n", f->name);
    printf("\n%s:\n", f->name);

    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", get_offset(f->lvars));  // ローカル変数の領域を確保する

    // 引数の値を stack に push してローカル変数と同じように扱えるように
    for (int i = 0; i < f->args->len; i++) {
        Node *arg = f->args->data[i];  // ND_LVAR
        printf("  lea rax, [rbp-%d]\n", arg->offset);
        char *argreg = node_to_argreg(arg, i);
        printf("  mov [rax], %s\n", argreg); // 第 i 引数の値をraxが指すメモリにコピー
    }

    // 中身のコードを吐く
    for (int i = 0; i < f->body->len; i++) {
        gen(f->body->data[i]);
    }
}

void gen_x86(Program *prog) {
    printf(".intel_syntax noprefix\n");

    printf(".data\n");
    for (int i = 0; i < prog->gvars->len; i++) {
        gen_gvar(prog->gvars->vals->data[i]);
    }
    printf("\n");
    printf(".text\n");
    for (int i = 0; i < prog->fns->len; i++) {
        gen_func(prog->fns->vals->data[i]);
    }
}
