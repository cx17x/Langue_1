// grammar.js — комбинированная версия (структура + лексика)
module.exports = grammar({
  name: 'v2lang_test',

  // Игнорируем пробелы и комментарии
  extras: $ => [/\s/, $.comment],

  rules: {
    // ---- Входной файл ----
    source_file: $ => repeat($.sourceItem),

    // ---- Комментарии ----
    comment: _ => token(choice(
      seq('//', /[^\n]*/),
      seq('{', /[^}]*/, '}')
    )),

    // ---- Элементы программы ----
    sourceItem: $ => $.funcDef,

    // method foo() begin end;
    funcDef: $ => seq(
      'method',
      $.identifier,
      '(', ')',
      $.block
    ),

    // begin ... end;
    block: $ => seq('begin', repeat($.statement), 'end', ';'),

    // Простые операторы (пока для теста)
    statement: $ => choice(
      $.assignment,
      $.expr_stmt,
      $.block
    ),

    // x := 5;
    assignment: $ => seq($.identifier, ':=', $.expr, ';'),

    // expr ;
    expr_stmt: $ => seq($.expr, ';'),

    // ---- Выражения ----
    expr: $ => choice($.literal, $.identifier),

    // ---- Лексемы ----
    identifier: _ => /[a-zA-Z_][a-zA-Z_0-9]*/,             // идентификатор
    str:        _ => /"[^"\\]*(?:\\.[^"\\]*)*"/,            // строка в кавычках
    char:       _ => /'[^']'/,                              // одиночный символ
    hex:        _ => /0[xX][0-9A-Fa-f]+/,                   // 0x...
    bits:       _ => /0[bB][01]+/,                          // 0b...
    dec:        _ => /[0-9]+/,                              // число
    bool:       _ => choice('true', 'false'),               // булево

    // объединяем все литералы
    literal: $ => choice($.bool, $.str, $.char, $.hex, $.bits, $.dec),
  }
});
