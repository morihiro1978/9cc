/* BNF:
   program    = stmt*
   stmt       = expr ";"
   expr       = assign
   assign     = equality ("=" assign)?
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("*" unary | "/" unary)*
   unary      = ("+" | "-")? term
   term       = num | var | "(" expr ")"
   var        = "a" ～ "z"
   num        = int ("0" | int)*
   int        = "1" ～ "9"
 */
#include "9cc.h"

/* コード */
Node *code[MAX_CODE];

/* 現在のトークン */
static Token *token = NULL;

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
           " .kind     : %s\n"
           " .str      : %s\n"
           " .len      : %d\n"
           " .num      : %d\n"
           " .list.next: 0x%08lx\n",
           (uintptr_t)tok,
           tokenKind_to_str(tok->kind),
           tok->str,
           tok->len,
           tok->num,
           (uintptr_t)tok->list.next);
}

/* トークンリストを表示する */
static void print_token_list(const Token *head) {
    Token *tok = (Token *)head;
    while (tok != NULL) {
        print_token(tok);
        tok = tok->list.next;
    }
}

/* 文字が1文字の記号か？ */
static bool is_exp_reserved1(const char c) {
    switch (c) {
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
        return true;
    }
    return false;
}

/* 文字が2文字の記号か？ */
static bool is_exp_reserved2(const char *exp) {
    const char *marks[] = {"<=", ">=", "==", "!="};

    for (int i = 0; i < sizeof(marks) / sizeof(char *); i++) {
        if (memcmp(marks[i], exp, 2) == 0) {
            return true;
        }
    }
    return false;
}

/* 文字が変数か */
static bool is_exp_variable(const char c) {
    if (('a' <= c) && (c <= 'z')) {
        return true;
    }
    return false;
}

/* 文字列から新しいトークンを作成し、cur リストに追加する */
static Token *new_token(TokenKind kind, const char *exp, int len, Token *cur) {
    Token *tok = calloc(1, sizeof(Token));

    tok->kind = kind;
    tok->str = exp;
    tok->len = len;
    cur->list.next = tok;
    return tok;
}

/* トークナイズする */
void tokenize(char *exp) {
    Token head;
    head.list.next = NULL;
    Token *cur = &head;

    while (1) {
        // EOF
        if (exp[0] == '\0') {
            cur = new_token(TK_EOF, exp, 0, cur);
            break;
        }
        // 空白をスキップ
        else if (isspace(exp[0]) != 0) {
            exp += 1;
        }
        // 予約後(2文字)
        else if (is_exp_reserved2(exp) == true) {
            cur = new_token(TK_RESERVED, exp, 2, cur);
            exp += 2;
        }
        // 予約後(1文字)
        else if (is_exp_reserved1(exp[0]) == true) {
            cur = new_token(TK_RESERVED, exp, 1, cur);
            exp += 1;
        }
        // 変数
        else if (is_exp_variable(exp[0]) == true) {
            cur = new_token(TK_VAR, exp, 1, cur);
            exp += 1;
        }
        // 数値
        else if (isdigit(exp[0])) {
            cur = new_token(TK_NUM, exp, 0, cur);
            cur->num = strtol(exp, &exp, 10);
            cur->len = exp - cur->str;
        }
        // 未知のトークン
        else {
            error_at(exp, "トークナイズできません。");
        }
    }
    token = head.list.next;
}

/* トークンが指定の記号なら true を返し、トークンを進める。
   別の記号なら false を返す。
 */
static bool consume(const char *op) {
    if ((token->kind != TK_RESERVED) || (strlen(op) != token->len)
        || (memcmp(op, token->str, token->len) != 0)) {
        return false;
    }
    token = token->list.next;
    return true;
}

/* トークンが変数ならそのトークンを返し、トークンを進める。
   違っていたら NULL を返す。
*/
static Token *consume_variable(void) {
    if (token->kind != TK_VAR) {
        return NULL;
    }
    Token *cur = token;
    token = token->list.next;
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
    token = token->list.next;
    return num;
}

/* EOFか？ */
static bool eof(void) {
    if (token->kind != TK_EOF) {
        return false;
    }
    return true;
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
static Node *var(char var) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_LVAR;
    node->num = (var - 'a' + 1) * 8U;
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
        Token *tok = consume_variable();
        if (tok != NULL) {
            return var(tok->str[0]);
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
    Node *node = expr();
    expect(";");
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