/* BNF:
   program    = stmt*
   stmt       = expr ";" | return expr ";"
   expr       = assign
   assign     = equality ("=" assign)?
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("*" unary | "/" unary)*
   unary      = ("+" | "-")? term
   term       = num | var | "(" expr ")"
   var        = ("a" ～ "z") ("a" ～ "z" | "A" ～ "Z" | "0" ～ "9" | "_")*
   num        = int ("0" | int)*
   int        = "1" ～ "9"
 */
#include "9cc.h"

/* コード */
Node *code[MAX_CODE];

/* 現在のトークン */
static Token *token = NULL;

/* ローカル変数のリスト */
LVar *locals = NULL;

/* トークンの種類を文字列に変換する */
static const char *tokenKind_to_str(TokenKind kind) {
    switch (kind) {
    case TK_RESERVED:
        return "TK_RESERVED";
    case TK_NUM:
        return "TK_NUM";
    case TK_EOF:
        return "TK_EOF";
    default:
        return "Undefiend";
    }
}

/* トークンを表示する */
static void print_token(const Token *tok) {
    printf("Token: 0x%08lx\n"
           " .kind : %s\n"
           " .str  : %s\n"
           " .len  : %d\n"
           " .num  : %d\n"
           " .next : 0x%08lx\n",
           (uintptr_t)tok,
           tokenKind_to_str(tok->kind),
           tok->str,
           tok->len,
           tok->num,
           (uintptr_t)tok->next);
}

/* トークンリストを表示する */
static void print_token_list(const Token *head) {
    Token *tok = (Token *)head;
    while (tok != NULL) {
        print_token(tok);
        tok = tok->next;
    }
}

/* 文字が /[A-Za-z9-0_]/ なら非0 を、違っていたら 0 を返す */
static int is_alnumubar(int c) {
    int ret = isalnum(c);
    if (ret == 0) {
        ret = c == '_';
    }
    return ret;
}

/* 文字列が空白なら、その文字数を返す。
   空白でなければ 0 を返す。
 */
static int is_exp_space(const char *exp) {
    int i = 0;
    while (isspace(exp[i]) != 0) {
        i++;
    }
    return i;
}

/* 文字列が記号なら、その文字数を返す。
   記号でなければ 0 を返す。
 */
static int is_exp_reserved(const char *exp) {
    const char *marks[] = {"<=", ">=", "==", "!="};

    // 2文字
    for (int i = 0; i < sizeof(marks) / sizeof(char *); i++) {
        if (memcmp(marks[i], exp, 2) == 0) {
            return 2;
        }
    }

    // 1文字
    switch (exp[0]) {
    case '+':  // fall down
    case '-':  // fall down
    case '*':  // fall down
    case '/':  // fall down
    case '(':  // fall down
    case ')':  // fall down
    case '>':  // fall down
    case '<':  // fall down
    case '=':  // fall down
    case ';':  // fall down
        return 1;
    }
    return 0;
}

/* 文字列が "return" なら、その文字数を返す。
   違っていれば 0 を返す。
 */
static int is_exp_reserved_return(const char *exp) {
    if ((memcmp(exp, "return", 6) == 0) && (is_alnumubar(exp[6]) == 0)) {
        return 6;
    }
    return 0;
}

/* 文字列が変数なら、その文字数を返す。
   変数でなければ 0 を返す。
 */
static int is_exp_variable(const char *exp) {
    int i = 0;

    if (isalpha(exp[i]) != 0) {
        i++;
        while (isalnum(exp[i]) != 0) {
            i++;
        }
    }
    return i;
}

/* 文字列から新しいトークンを作成し、cur リストに追加する */
static Token *new_token(TokenKind kind, const char *exp, int len, Token *cur) {
    Token *tok = calloc(1, sizeof(Token));

    tok->kind = kind;
    tok->str = exp;
    tok->len = len;
    cur->next = tok;
    return tok;
}

/* トークナイズする */
void tokenize(char *exp) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    int len;

    while (1) {
        // EOF
        if (exp[0] == '\0') {
            cur = new_token(TK_EOF, exp, 0, cur);
            break;
        }
        // 空白をスキップ
        else if ((len = is_exp_space(exp)) > 0) {
            exp += len;
        }
        // 予約後
        else if ((len = is_exp_reserved(exp)) > 0) {
            cur = new_token(TK_RESERVED, exp, len, cur);
            exp += len;
        }
        // return
        else if ((len = is_exp_reserved_return(exp)) > 0) {
            cur = new_token(TK_RETURN, exp, len, cur);
            exp += len;
        }
        // 変数
        else if ((len = is_exp_variable(exp)) > 0) {
            cur = new_token(TK_VAR, exp, len, cur);
            exp += len;
        }
        // 数値
        else if (isdigit(exp[0])) {
            cur = new_token(TK_NUM, exp, -1, cur);
            cur->num = strtol(exp, &exp, 10);
            cur->len = exp - cur->str;
        }
        // 未知のトークン
        else {
            error_at(exp, "トークナイズできません。");
        }
    }
    token = head.next;
}

/* トークンが指定の記号なら true を返し、トークンを進める。
   別の記号なら false を返す。
 */
static bool consume(const char *op) {
    if ((token->kind != TK_RESERVED) || (strlen(op) != token->len)
        || (memcmp(op, token->str, token->len) != 0)) {
        return false;
    }
    token = token->next;
    return true;
}

/* トークンが指定の種類ならそのトークンを返し、トークンを進める。
   違っていたら NULL を返す。
*/
static Token *consume_with_kind(TokenKind kind) {
    if (token->kind != kind) {
        return NULL;
    }
    Token *cur = token;
    token = token->next;
    return cur;
}

/* トークンが指定の記号なら、トークンを進める。
   違っていたらパニックする。
*/
static void expect(const char *op) {
    if (consume(op) == false) {
        error_at(token->str, "予期せぬトークンです");
    }
}

/* トークンが数値ならそれを返し、トークンを進める。
   違っていたらパニックする。
 */
static int expect_number(void) {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数値ではありません");
    }
    int num = token->num;
    token = token->next;
    return num;
}

/* EOFか？ */
static bool eof(void) {
    if (token->kind != TK_EOF) {
        return false;
    }
    return true;
}

/* tok に一致するローカル変数があれば、その情報を返す。
   なければ NULL を返す。
 */
static LVar *find_local(const Token *tok) {
    LVar *var = locals;
    while (var != NULL) {
        if ((tok->len == var->len)
            && (memcmp(tok->str, var->name, var->len) == 0)) {
            break;
        }
        var = var->next;
    }
    return var;
}

/* 2項のノードを作成する */
struct Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

/* 数値ノードを作成する */
struct Node *new_node_num(int num) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->num = num;
    return node;
}

/* パーサ: num */
static Node *num(void) {
    return new_node_num(expect_number());
}

/* パーサ: var */
static Node *var(const Token *tok) {
    Node *node = calloc(1, sizeof(Node));

    LVar *var = find_local(tok);
    if (var == NULL) {
        var = calloc(1, sizeof(LVar));
        var->name = (char *)tok->str;
        var->len = tok->len;
        var->offset = locals == NULL ? 8 : locals->offset + 8;
        var->next = locals;
        locals = var;
    }

    node->kind = ND_LVAR;
    node->offset = var->offset;
    return node;
}

/* パーサ: term */
static Node *expr(void);
static Node *term(void) {
    if (consume("(") == true) {
        Node *node = expr();
        expect(")");
        return node;
    } else {
        Token *tok = consume_with_kind(TK_VAR);
        if (tok != NULL) {
            return var(tok);
        } else {
            return num();
        }
    }
}

/* パーサ: unary */
static Node *unary(void) {
    if (consume("+") == true) {
        return term();
    } else if (consume("-") == true) {
        return new_node(ND_SUB, new_node_num(0), term());
    } else {
        return term();
    }
}

/* パーサ: mul */
static Node *mul(void) {
    Node *node = unary();

    while (1) {
        if (consume("*") == true) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/") == true) {
            node = new_node(ND_DIV, node, unary());
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: add */
static Node *add(void) {
    Node *node = mul();

    while (1) {
        if (consume("+") == true) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-") == true) {
            node = new_node(ND_SUB, node, mul());
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: relational */
static Node *relational(void) {
    Node *node = add();

    while (1) {
        if (consume("<") == true) {
            node = new_node(ND_LT, node, add());
        } else if (consume("<=") == true) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">") == true) {
            node = new_node(ND_LT, add(), node);
        } else if (consume(">=") == true) {
            node = new_node(ND_LE, add(), node);
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: equality */
static Node *equality(void) {
    Node *node = relational();

    while (1) {
        if (consume("==") == true) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=") == true) {
            node = new_node(ND_NE, node, relational());
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: assign */
static Node *assign(void) {
    Node *node = equality();

    if (consume("=") == true) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

/* パーサ: expr */
static Node *expr(void) {
    return assign();
}

/* パーサ: stmt */
static Node *stmt(void) {
    Node *node = NULL;
    if (consume_with_kind(TK_RETURN) != NULL) {
        node = new_node(ND_RETURN, NULL, expr());
        expect(";");
    } else {
        node = expr();
        expect(";");
    }
    return node;
}

/* パーサ: program */
static void program(void) {
    int i = 0;

    while (eof() == false) {
        code[i] = stmt();
        i++;
    }
    code[i] = NULL;
}

/* パース */
void parse(void) {
    program();
}