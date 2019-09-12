/* BNF:
   program    = deffunc*
   deffunc    = "int" ident "(" ((defvar ",")* defvar)? ")" "{" stmt* "}"
   stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | return expr ";"
              | defvar ";"
   expr       = assign
   defvar     = "int" ident
   assign     = equality ("=" assign)?
   equality   = relational ("==" relational | "!=" relational)*
   relational = add ("<" add | "<=" add | ">" add | ">=" add)*
   add        = mul ("+" mul | "-" mul)*
   mul        = unary ("*" unary | "/" unary)*
   unary      = ("+" | "-")? term
              | ("*" | "&") unary
   term       = num
              | ident ("(" ((expr ",")* expr)? ")")?
              | "(" expr ")"
   ident      = ("a" ～ "z") ("a" ～ "z" | "A" ～ "Z" | "0" ～ "9" | "_")*
   num        = int ("0" | int)*
   int        = "1" ～ "9"
 */
#include "9cc.h"

/* ローカル関数 */
static Node *expr(Node *pblock);

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

/* NULL ノードを作成する */
static Node *new_node_null(void) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NULL;
    return node;
}

/* 1項演算子のノードを作成する */
static Node *new_node_op1(NodeKind kind, Node *expr) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->v.op1.expr = expr;
    return node;
}

/* 2項演算子のノードを作成する */
static Node *new_node_op2(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = kind;
    node->v.op2.lhs = lhs;
    node->v.op2.rhs = rhs;
    return node;
}

/* if 文のノードを作成する */
static Node *new_node_if(Node *test, Node *tbody, Node *ebody) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_IF;
    node->v.cif.test = test;
    node->v.cif.tbody = tbody;
    node->v.cif.ebody = ebody;
    return node;
}

/* while 文のノードを作成する */
static Node *new_node_while(Node *test, Node *body) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_WHILE;
    node->v.cwhile.test = test;
    node->v.cwhile.body = body;
    return node;
}

/* for 文のノードを作成する */
static Node *new_node_for(Node *init, Node *test, Node *update, Node *body) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_FOR;
    node->v.cfor.init = init;
    node->v.cfor.test = test;
    node->v.cfor.update = update;
    node->v.cfor.body = body;
    return node;
}

/* 数値ノードを作成する */
static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_NUM;
    node->v.num.val = val;
    return node;
}

/* ブロックノードを作成する */
static Node *new_node_block(Node *pblock) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_BLOCK;
    node->v.block.pblock = pblock;
    return node;
}

/* ブロックノードにノードを追加する */
static void block_add_node(Node *block, Node *node) {
    if (block->kind != ND_BLOCK) {
        error("ブロックノードではありません。");
    }

    if (block->v.block.max_code == block->v.block.num_code) {
        block->v.block.max_code += MAX_CODE;
        block->v.block.code = (Node **)realloc(
            block->v.block.code, block->v.block.max_code * sizeof(Node));
        if (block->v.block.code == NULL) {
            error("ブロックノードを %d に拡張できません。",
                  block->v.block.max_code);
        }
    }
    block->v.block.code[block->v.block.num_code] = node;
    block->v.block.num_code++;
}

/* ブロックノードに変数を登録する */
static void block_add_local(Node *block, LVar *var) {
    var->next = block->v.block.locals;
    block->v.block.locals = var;
    block->v.block.num_local++;

    // ブロック内の変数の数をインクリメント
    // 最上位の親ブロックまでさかのぼってインクリメントすることにより、
    // 下位ブロックも含めた変数の個数が分かる
    while (block != NULL) {
        block->v.block.total_local++;
        block = block->v.block.pblock;
    }
}

/* tok に一致するローカル変数があれば、その情報を返す。
   なければ、親ブロックにさかのぼって探して返す。
   グローバルブロックまで探して見つからなければ NULL を返す。
 */
static LVar *block_find_local(Node *block, const Token *tok) {
    while (block != NULL) {
        LVar *var = block->v.block.locals;
        while (var != NULL) {
            if ((tok->len == var->len)
                && (memcmp(tok->str, var->name, var->len) == 0)) {
                return var;
            }
            var = var->next;
        }
        block = block->v.block.pblock;
    }
    return NULL;
}

/* ローカル変数の rbp からのオフセットの最大値を返す */
static int block_total_local(Node *block) {
    while (block->v.block.pblock != NULL) {
        block = block->v.block.pblock;
    }
    return block->v.block.total_local;
}

/* func ノードを作成する */
static Node *new_node_func(const Token *tok) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_FUNC;
    node->v.func.name = (char *)tok->str;
    node->v.func.len = tok->len;
    return node;
}

/* func ノードにパラメータを追加する */
static void func_add_param(Node *func, Node *param) {
    if (func->kind != ND_FUNC) {
        error("funcノードではありません。");
    }

    if (func->v.func.max_param == func->v.func.num_param) {
        func->v.func.max_param += MAX_PARAM;
        func->v.func.params = (Node **)realloc(
            func->v.func.params, func->v.func.max_param * sizeof(Node*));
        if (func->v.func.params == NULL) {
            error("funcノードを %d に拡張できません。", func->v.func.max_param);
        }
    }
    func->v.func.params[func->v.func.num_param] = param;
    func->v.func.num_param++;
}

/* deffunc ノードを作成する */
static Node *new_node_deffunc(const Token *tok) {
    Node *node = calloc(1, sizeof(Node));

    node->kind = ND_DEFFUNC;
    node->v.deffunc.name = (char *)tok->str;
    node->v.deffunc.len = tok->len;
    return node;
}

/* deffunc ノードにパラメータを追加する */
static void deffunc_add_param(Node *deffunc, LVar *var) {
    if (deffunc->kind != ND_DEFFUNC) {
        error("deffuncノードではありません。");
    }

    if (deffunc->v.deffunc.max_param == deffunc->v.deffunc.num_param) {
        deffunc->v.deffunc.max_param += MAX_PARAM;
        deffunc->v.deffunc.params
            = (LVar **)realloc(deffunc->v.deffunc.params,
                               deffunc->v.deffunc.max_param * sizeof(LVar*));
        if (deffunc->v.deffunc.params == NULL) {
            error("deffuncノードを %d に拡張できません。",
                  deffunc->v.deffunc.max_param);
        }
    }
    deffunc->v.deffunc.params[deffunc->v.deffunc.num_param] = var;
    deffunc->v.deffunc.num_param++;
}

/* パーサ: num */
static Node *num(void) {
    Token *tok = expect_with_kind(TK_NUM);
    return new_node_num(tok->num);
}

/* パーサ: var */
static Node *var(Node *pblock, const Token *tok) {
    Node *node = calloc(1, sizeof(Node));

    LVar *var = block_find_local(pblock, tok);
    if (var == NULL) {
        error_at(
            tok->str, "変数 %.*s は宣言されていません", tok->len, tok->str);
    }

    node->kind = ND_LVAR;
    node->v.lvar.name = var->name;
    node->v.lvar.len = var->len;
    node->v.lvar.offset = var->offset;
    return node;
}

/* パーサ: term */
static Node *term(Node *pblock) {
    if (consume("(") == true) {
        Node *node = expr(pblock);
        expect(")");
        return node;
    } else {
        Token *tok = consume_with_kind(TK_IDENT);
        if (tok != NULL) {
            // 関数呼び出し
            if (consume("(") == true) {
                Node *func = new_node_func(tok);
                if (consume(")") == false) {
                    while (1) {
                        func_add_param(func, expr(pblock));
                        if (consume(")") == true) {
                            break;
                        } else {
                            expect(",");
                        }
                    }
                }
                if (func->v.func.max_param > MAX_PARAM) {
                    error("パラメータが %d 個以上設定されています。",
                          MAX_PARAM);
                }
                return func;
            }
            // 変数
            else {
                return var(pblock, tok);
            }
        } else {
            return num();
        }
    }
}

/* パーサ: unary */
static Node *unary(Node *pblock) {
    if (consume("+") == true) {
        return term(pblock);
    } else if (consume("-") == true) {
        return new_node_op2(ND_SUB, new_node_num(0), term(pblock));
    } else if (consume("&") == true) {
        return new_node_op1(ND_ADDR, unary(pblock));
    } else if (consume("*") == true) {
        return new_node_op1(ND_DEREF, unary(pblock));
    } else {
        return term(pblock);
    }
}

/* パーサ: mul */
static Node *mul(Node *pblock) {
    Node *node = unary(pblock);

    while (1) {
        if (consume("*") == true) {
            node = new_node_op2(ND_MUL, node, unary(pblock));
        } else if (consume("/") == true) {
            node = new_node_op2(ND_DIV, node, unary(pblock));
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: add */
static Node *add(Node *pblock) {
    Node *node = mul(pblock);

    while (1) {
        if (consume("+") == true) {
            node = new_node_op2(ND_ADD, node, mul(pblock));
        } else if (consume("-") == true) {
            node = new_node_op2(ND_SUB, node, mul(pblock));
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: relational */
static Node *relational(Node *pblock) {
    Node *node = add(pblock);

    while (1) {
        if (consume("<") == true) {
            node = new_node_op2(ND_LT, node, add(pblock));
        } else if (consume("<=") == true) {
            node = new_node_op2(ND_LE, node, add(pblock));
        } else if (consume(">") == true) {
            node = new_node_op2(ND_LT, add(pblock), node);
        } else if (consume(">=") == true) {
            node = new_node_op2(ND_LE, add(pblock), node);
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: equality */
static Node *equality(Node *pblock) {
    Node *node = relational(pblock);

    while (1) {
        if (consume("==") == true) {
            node = new_node_op2(ND_EQ, node, relational(pblock));
        } else if (consume("!=") == true) {
            node = new_node_op2(ND_NE, node, relational(pblock));
        } else {
            break;
        }
    }
    return node;
}

/* パーサ: assign */
static Node *assign(Node *pblock) {
    Node *node = equality(pblock);

    if (consume("=") == true) {
        node = new_node_op2(ND_ASSIGN, node, assign(pblock));
    }
    return node;
}

/* パーサ: expr */
static Node *expr(Node *pblock) {
    return assign(pblock);
}

/* ローカル変数を宣言する */
static LVar *defvar_lvar(Node *pblock, Type type) {
    LVar *var = NULL;
    Token *tok = consume_with_kind(TK_IDENT);
    if (tok != NULL) {
        var = block_find_local(pblock, tok);
        if (var == NULL) {
            var = calloc(1, sizeof(LVar));
            var->name = (char *)tok->str;
            var->len = tok->len;
            var->type = type;
            var->offset = (block_total_local(pblock) + 1) * 8;
            block_add_local(pblock, var);
        } else {
            error_at(tok->str,
                     "変数 %.*s が多重定義されました。",
                     tok->len,
                     tok->str);
        }
    } else {
        error("型の後に変数が定義されていません。");
    }
    return var;
}

/* パーサ: defvar */
static LVar *defvar(Node *pblock) {
    Token *tok = expect_with_kind(TK_TYPE);
    return defvar_lvar(pblock, tok->type);
}

/* パーサ: stmt */
static Node *stmt(Node *pblock) {
    Token *tok = NULL;
    Node *node = NULL;
    if (consume_with_kind(TK_RETURN) != NULL) {
        node = new_node_op1(ND_RETURN, expr(pblock));
        expect(";");
    } else if (consume_with_kind(TK_IF) != NULL) {
        expect("(");
        Node *test = expr(pblock);
        expect(")");
        Node *tbody = stmt(pblock);
        Node *ebody = NULL;
        if (consume_with_kind(TK_ELSE) != NULL) {
            ebody = stmt(pblock);
        }
        node = new_node_if(test, tbody, ebody);
    } else if (consume_with_kind(TK_WHILE) != NULL) {
        expect("(");
        Node *test = expr(pblock);
        expect(")");
        node = new_node_while(test, stmt(pblock));
    } else if (consume_with_kind(TK_FOR) != NULL) {
        Node *init = NULL;
        Node *test = NULL;
        Node *update = NULL;
        expect("(");
        if (consume(";") == false) {
            init = expr(pblock);
            expect(";");
        }
        if (consume(";") == false) {
            test = expr(pblock);
            expect(";");
        }
        if (consume(")") == false) {
            update = expr(pblock);
            expect(")");
        }
        node = new_node_for(init, test, update, stmt(pblock));
    } else if (consume("{") == true) {
        Node *block = new_node_block(pblock);
        while (consume("}") == false) {
            block_add_node(block, stmt(block));
        }
        return block;
    } else if (peek()->kind == TK_TYPE) {
        (void)defvar(pblock);
        node = new_node_null();
        expect(";");
    } else {
        node = expr(pblock);
        expect(";");
    }
    return node;
}

/* パーサ: deffunc */
static Node *deffunc(void) {
    Token *type = expect_with_kind(TK_TYPE);
    Token *tok = expect_with_kind(TK_IDENT);
    Node *deffunc = new_node_deffunc(tok);
    deffunc->v.deffunc.rettype = type->type;

    Node *block = new_node_block(NULL);
    deffunc->v.deffunc.block = block;

    // parameter
    expect("(");
    if (consume(")") == false) {
        while (1) {
            deffunc_add_param(deffunc, defvar(block));
            if (consume(")") == true) {
                break;
            } else {
                expect(",");
            }
        }
    }
    if (deffunc->v.deffunc.max_param > MAX_PARAM) {
        error("パラメータが %d 個以上設定されています。", MAX_PARAM);
    }

    // block
    expect("{");
    while (consume("}") == false) {
        block_add_node(block, stmt(block));
    }
    return deffunc;
}

/* パーサ: program */
static Node *program(void) {
    Node *global_block = new_node_block(NULL);

    while (eof() == false) {
        block_add_node(global_block, deffunc());
    }
    return global_block;
}

/* パース */
Node *parse(void) {
    return program();
}
