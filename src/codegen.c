#include "10cc.h"

char *regs[] = {"r10", "r11", "rbx", "r12", "r13", "r14", "r15"};
char *regs8[] = {"r10b", "r11b", "bl", "r12b", "r13b", "r14b", "r15b"};
char *regs32[] = {"r10d", "r11d", "ebx", "r12d", "r13d", "r14d", "r15d"};

char *argregs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
char *argregs8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
char *argregs32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

int label_cnt = 0;

void gen_gvar(Var *gvar);
void gen_func(Func *fn);
int assign_lvar_offset(Scope *scope, int offset);
void gen(Node *node);
void gen_lval(Node *node);
char *reg(int size);
char *argreg(int r, int size);

void gen_x86_64(Program *prog) {
    printf(".intel_syntax noprefix\n");

    // data segment
    printf(".data\n");
    for (int i = 0; i < prog->gvars->len; i++) {
        gen_gvar(vec_get(prog->gvars->vals, i));
    }

    // text segment
    printf("\n");
    printf(".text\n");
    for (int i = 0; i < prog->funcs->len; i++) {
        gen_func(vec_get(prog->funcs->vals, i));
    }
}

void gen_gvar(Var *gvar) {
    printf("%s:\n", gvar->name);
    printf("  .zero %d\n", gvar->type->size);
}

void gen_func(Func *fn) {
    printf("\n.global %s\n", fn->name);
    printf("%s:\n", fn->name);

    int offset = assign_lvar_offset(fn->scope, 0);
    // prologue
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", offset);  // allocate an area of local variables

    // push arguments to stack to treat them as local variables
    for (int i = 0; i < fn->args->len; i++) {
        Var *arg = vec_get(fn->args, i);
        assert(arg->offset, "The offset of argument missing\n");
        printf("  lea rax, [rbp-%d]\n", arg->offset);
        printf("  mov [rax], %s\n", argreg(i, arg->type->size));
    }

    gen(fn->body);

    // epilogue (used when fuction ends without return stmt)
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
}

int assign_lvar_offset(Scope *scope, int offset) {
    for (int i = 0; i < scope->lvars->len; i++) {
        Var *lvar = vec_get(scope->lvars->vals, i);
        assert(lvar->is_local, "Scope contains global variable: '%s'", lvar->name);
        offset += lvar->type->size;
        lvar->offset = offset;
    }
    // DFS
    for (int i = 0; i < scope->children->len; i++) {
        offset = assign_lvar_offset(vec_get(scope->children, i), offset);
    }
    return offset;
}

// スタックから入力を受け取り、nodeが表す演算の結果をスタックに戻す処理を生成する
void gen(Node *node) {
    int cur_label_cnt;
    switch (node->kind) {
    case ND_NULL:
        return;
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_FUNC_CALL:
        for (int i = 0; i < node->args->len; i++) {
            gen(vec_get(node->args, i));
            printf("  pop %s\n", argreg(i, 8));  // "pop" instruction always moves 8 byte
        }
        printf("  call %s\n", node->func->name);
        printf("  push rax\n");
        return;
    case ND_VARREF:
        gen_lval(node);
        printf("  pop rax\n");                               // set rax to the address of the variable
        printf("  mov %s, [rax]\n", reg(node->type->size));  // raxの指す先にアクセスして中身をraxにコピー
        if (node->type->size == 1) {
            printf("  movsx %s, %s\n", reg(8), reg(1));
        }
        printf("  push %s\n", reg(8));  // コピーしてきた値をスタックにpush
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  pop %s\n", reg(8));                             // rhs
        printf("  pop rax\n");                                    // lhs (address)
        printf("  mov [rax], %s\n", reg(node->lhs->type->size));  // copy rhs value to the memory pointed to by rax
        printf("  push %s\n", reg(8));                            // write the result of assign operation
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
        for (int i = 0; i < node->stmts->len; i++) {
            gen(vec_get(node->stmts, i));
        }
        return;
    case ND_ADDR:
        gen_lval(node->lhs);
        return;
    case ND_DEREF:       // 右辺値参照
        gen(node->lhs);  // アドレスがスタックに積まれる
        printf("  pop rax\n");
        printf("  mov %s, [rax]\n", reg(node->type->size));
        if (node->type->size == 1) {
            printf("  movsx %s, %s\n", reg(8), reg(1));
        }
        printf("  push %s\n", reg(8));
        return;
    default:
        break;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");  // rhs
    printf("  pop rax\n");  // lhs

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
    default:
        error("Unknown node kind: %d", node->kind);
    }
    printf("  push rax\n");
}

// Push the address of the local/global variable represented by node
void gen_lval(Node *node) {
    switch (node->kind) {
    case ND_VARREF:
        if (node->var->is_local) {
            printf("  lea rax, [rbp-%d]\n", node->var->offset);
        } else {
            printf("  lea rax, %s\n", node->var->name);
        }
        printf("  push rax\n");
        break;
    case ND_DEREF:
        gen(node->lhs);
        break;
    default:
        error("Referable node expected");
    }
}

char *reg(int size) {
    int r = 0;  // return 0th register tentatively
    switch (size) {
    case 1:
        return regs8[r];
    case 4:
        return regs32[r];
    case 8:
        return regs[r];
    default:
        error("Invalid register size: %d", size);
    }
}

char *argreg(int r, int size) {
    switch (size) {
    case 1:
        return argregs8[r];
    case 4:
        return argregs32[r];
    case 8:
        return argregs[r];
    default:
        error("Invalid arg register size: %d", size);
    }
}
