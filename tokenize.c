#include "9cc.h"

/* 現在のトークン */
static Token *token = NULL;

/* 文字が /[A-Za-z_]/ なら非0 を、違っていたら 0 を返す */
static int is_alpha(int c) {
    int ret = isalpha(c);
    if (ret == 0) {
        ret = c == '_';
    }
    return ret;
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
    case '&':  // fall down
    case '/':  // fall down
    case '(':  // fall down
    case ')':  // fall down
    case '>':  // fall down
    case '<':  // fall down
    case '=':  // fall down
    case ';':  // fall down
    case '{':  // fall down
    case '}':  // fall down
    case ',':  // fall down
        return 1;
    }
    return 0;
}

/* 文字列が予約後 ident なら、その文字数を返す。
   違っていれば 0 を返す。
 */
static int is_exp_reserved_as(const char *exp, const char *ident) {
    int len = strlen(ident);
    if ((memcmp(exp, ident, len) == 0) && (is_alnumubar(exp[len]) == 0)) {
        return len;
    }
    return 0;
}

/* 文字列が変数なら、その文字数を返す。
   変数でなければ 0 を返す。
 */
static int is_exp_variable(const char *exp) {
    int i = 0;

    if (is_alpha(exp[i]) != 0) {
        i++;
        while (is_alnumubar(exp[i]) != 0) {
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
        // 型
        else if ((len = is_exp_reserved_as(exp, "int")) > 0) {
            cur = new_token(TK_TYPE, exp, len, cur);
            cur->type = TY_INT;
            exp += len;
        }
        // 予約語
        else if ((len = is_exp_reserved(exp)) > 0) {
            cur = new_token(TK_RESERVED, exp, len, cur);
            exp += len;
        }
        // return
        else if ((len = is_exp_reserved_as(exp, "return")) > 0) {
            cur = new_token(TK_RETURN, exp, len, cur);
            exp += len;
        }
        // if
        else if ((len = is_exp_reserved_as(exp, "if")) > 0) {
            cur = new_token(TK_IF, exp, len, cur);
            exp += len;
        }
        // else
        else if ((len = is_exp_reserved_as(exp, "else")) > 0) {
            cur = new_token(TK_ELSE, exp, len, cur);
            exp += len;
        }
        // while
        else if ((len = is_exp_reserved_as(exp, "while")) > 0) {
            cur = new_token(TK_WHILE, exp, len, cur);
            exp += len;
        }
        // for
        else if ((len = is_exp_reserved_as(exp, "for")) > 0) {
            cur = new_token(TK_FOR, exp, len, cur);
            exp += len;
        }
        // 変数
        else if ((len = is_exp_variable(exp)) > 0) {
            cur = new_token(TK_IDENT, exp, len, cur);
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
bool consume(const char *op) {
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
Token *consume_with_kind(TokenKind kind) {
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
void expect(const char *op) {
    if (consume(op) == false) {
        error_at(token->str, "予期せぬトークンです");
    }
}

/* トークンが kind ならそれを返し、トークンを進める。
   違っていたらパニックする。
 */
Token *expect_with_kind(TokenKind kind) {
    if (token->kind != kind) {
        const char *str[] = { "記号", "return", "if", "else", "while", "for", "変数", "整数", "型", "EOF" };
        error_at(token->str, "%sではありません", str[kind]);
    }
    Token *tok = token;
    token = token->next;
    return tok;
}

/* EOFか？ */
bool eof(void) {
    if (token->kind != TK_EOF) {
        return false;
    }
    return true;
}
