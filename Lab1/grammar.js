// grammar.js — комбинированная версия (структура + лексика)
module.exports = grammar({
  name: 'v2lang_test',

  // Игнорируем пробелы и комментарии
  extras: $ => [/\s+/, $.comment],

  rules: {
    // ---- Входной файл ----
    source_file: $ => repeat($.sourceItem),

    // ---- Комментарии ----
    comment: _ => token(choice(
      seq('//', /[^\n]*/),
      seq('{', /[^}]*/, '}')
    )),

    // ---- Элементы программы ----
    sourceItem: $ => choice($.funcDef),

    // ---- Типы ----
    typeRef: $ => choice(
      // builtin
      choice('bool', 'byte', 'int', 'uint', 'long', 'ulong', 'char', 'string'),
      // custom
      $.identifier,
      // array: array '[' (',')* ']' 'of' typeRef
      seq('array', '[', repeat(','), ']', 'of', $.typeRef)
    ),

    // funcSignature: identifier '(' list<argDef> ')' (':' typeRef)?
    funcSignature: $ => seq(
      $.identifier,
      '(', optional(seq(optional($.argList))), ')',
      optional(seq(':', $.typeRef))
    ),

    argList: $ => seq($.argDef, repeat(seq(',', $.argDef))),

    argDef: $ => seq($.identifier, optional(seq(':', $.typeRef))),

    // method foo(...) body|;
    funcDef: $ => seq('method', $.funcSignature, choice($.body, ';')),

    body: $ => seq(
      repeat(seq('var', repeat($.varDecl))),
      $.block
    ),

    varDecl: $ => seq($.idList, optional(seq(':', $.typeRef)), ';'),

    idList: $ => prec.left(3, seq($.identifier, repeat(seq(',', $.identifier)))),

    // ---- Statements ----
    statement: $ => choice(
      $.if_statement,
      $.block,
      $.while_statement,
      $.do_statement,
      $.break_statement,
      $.assignment,
      $.expr_stmt
    ),

    if_statement: $ => prec.right(seq('if', $.expr, 'then', $.statement, optional(seq('else', $.statement)))),

    block: $ => seq('begin', repeat($.statement), 'end', ';'),

    while_statement: $ => seq('while', $.expr, 'do', $.statement),

    do_statement: $ => seq('repeat', $.statement, choice('while', 'until'), $.expr, ';'),

    break_statement: $ => seq('break', ';'),

    expr_stmt: $ => seq($.expr, ';'),

    // ---- Expressions with precedence ----
    expr: $ => $._expr,

    _expr: $ => choice(
      $.logical_or
    ),

    logical_or: $ => prec.left(seq($.logical_and, repeat(seq(choice('||', 'or'), $.logical_and)))),
    logical_and: $ => prec.left(seq($.bitwise_or, repeat(seq(choice('&&', 'and'), $.bitwise_or)))),

    bitwise_or: $ => prec.left(seq($.bitwise_xor, repeat(seq('|', $.bitwise_xor)))),
    bitwise_xor: $ => prec.left(seq($.bitwise_and, repeat(seq('^', $.bitwise_and)))),
    bitwise_and: $ => prec.left(seq($.equality, repeat(seq('&', $.equality)))),

    equality: $ => prec.left(seq($.relational, repeat(seq(choice('=', '!='), $.relational)))),
    relational: $ => prec.left(seq($.shift, repeat(seq(choice('<', '>', '<=', '>='), $.shift)))),

    shift: $ => prec.left(seq($.add, repeat(seq(choice('<<', '>>'), $.add)))),

    add: $ => prec.left(seq($.mul, repeat(seq(choice('+', '-'), $.mul)))),
    mul: $ => prec.left(seq($.unary, repeat(seq(choice('*', '/', '%'), $.unary)))),

    unary: $ => choice(seq(choice('-', '!', 'not', '~'), $.unary), $.postfix),

    // assignment is a statement (not an expression): postfix ':=' expr ';'
    assignment: $ => prec.right(seq($.postfix, ':=', $.expr, ';')),

    // primary expressions: literals, identifiers, parenthesis
    primary: $ => choice($.literal, $.identifier, seq('(', $.expr, ')')),

    // postfix: primary with any number of calls/indexers
    postfix: $ => choice(
      $.primary,
      seq($.postfix, '(', optional($.exprList), ')'),
      seq($.postfix, '[', optional($.exprList), ']')
    ),

    exprList: $ => seq($.expr, repeat(seq(',', $.expr))),

    // ---- Лексемы ----
    identifier: _ => /[a-zA-Z_][a-zA-Z_0-9]*/,             // идентификатор
    str:        _ => /"[^"\\]*(?:\\.[^"\\]*)*"/,            // строка в кавычках
    char:       _ => /'[^'\\]'/,                              // одиночный символ
    hex:        _ => /0[xX][0-9A-Fa-f]+/,                   // 0x...
    bits:       _ => /0[bB][01]+/,                          // 0b...
    dec:        _ => /[0-9]+/,                              // число
    bool:       _ => choice('true', 'false'),               // булево

    // объединяем все литералы
    literal: $ => choice($.bool, $.str, $.char, $.hex, $.bits, $.dec),
  }
});
