/**
 * Tree-sitter grammar for Salam, in both keyword languages.
 *
 * The keyword spellings come from keywords.json, which
 * extensions/tools/gen_syntaxes.salam writes from the compiler's own tables,
 * so a new keyword or a new Persian spelling reaches this grammar by
 * regenerating that file.
 */

const K = require("./keywords.json");

const kw = (name) => {
  const words = K[name] || [];
  if (words.length === 0) {
    throw new Error(`keywords.json has no spellings for ${name}`);
  }
  return words.length === 1 ? words[0] : choice(...words);
};

const PREC = {
  ternary: 1,
  or: 2,
  and: 3,
  not: 4,
  compare: 5,
  bitor: 6,
  bitxor: 7,
  bitand: 8,
  shift: 9,
  add: 10,
  mul: 11,
  power: 12,
  cast: 13,
  unary: 14,
  postfix: 15,
};

const COMMA = choice(",", "\u060c");
const commaSep1 = (rule) =>
  seq(rule, repeat(seq(COMMA, rule)), optional(COMMA));
const commaSep = (rule) => optional(commaSep1(rule));

module.exports = grammar({
  name: "salam",

  externals: ($) => [
    $.name,
    $.member_name,
    $.layout_name,
    $.import_path,
    $._layout_keyword,
    $._component_keyword,
    $._dyn_keyword,
  ],

  extras: ($) => [/\s/, $.line_comment, $.block_comment],

  conflicts: ($) => [
    [$.conditional_block, $.else_clause],
    [$._top_level, $.if_statement],
    [$.conditional_block, $.if_statement],
    [$.parameter, $.dotted_name],
    [$.parameter, $._expression],
    [$.modifier, $.variable_declaration, $.const_declaration],
    [$.modifier, $.variable_declaration],
    [$._expression, $.dotted_name],
    [$.binary_expression, $.call_expression],
    [$.binary_expression, $.ternary_expression, $.try_expression],
    [$.dyn_type, $.variant_type],
    [$.unary_expression, $.ternary_expression, $.try_expression],
    [$.binary_expression, $.unary_expression, $.call_expression],
    [$.meta],
    [$._expression, $.dotted_name],
  ],

  rules: {
    source_file: ($) => repeat($._top_level),

    _top_level: ($) =>
      choice(
        $.conditional_block,
        $.global_declaration,
        $.package_declaration,
        $.import_declaration,
        $.include_declaration,
        $.meta,
        $.extern_block,
        $.export_block,
        $.function_definition,
        $.struct_definition,
        $.interface_definition,
        $.enum_definition,
        $.impl_definition,
        $.type_alias,
        $.layout_block,
        $.component_definition,
        $._statement,
      ),

    conditional_block: ($) =>
      prec.dynamic(
        1,
        seq(
          kw("if"),
          field("condition", $._expression),
          ":",
          repeat($._top_level),
          repeat(
            seq(kw("else"), optional($._expression), ":", repeat($._top_level)),
          ),
          kw("end"),
        ),
      ),

    global_declaration: ($) =>
      prec.right(
        seq(
          optional(kw("pub")),
          field("name", $.name),
          ":",
          field("type", $._type),
          optional(seq("=", field("value", $._expression))),
        ),
      ),

    package_declaration: ($) =>
      seq(kw("package"), field("name", $.dotted_name)),

    import_declaration: ($) =>
      prec.right(
        seq(
          kw("import"),
          field("path", choice($.import_path, $.string)),
          optional(seq(kw("as"), field("alias", $.name))),
        ),
      ),

    include_declaration: ($) =>
      seq(
        kw("include"),
        optional(field("alias", $.name)),
        field("path", $.string),
      ),

    meta: ($) =>
      seq(/@[A-Za-z_]+/, repeat1(choice($.string, $.name, $.integer))),

    extern_block: ($) =>
      seq(
        kw("extern"),
        ":",
        repeat(choice($.function_signature, $.meta)),
        kw("end"),
      ),

    export_block: ($) =>
      seq(
        kw("export"),
        ":",
        repeat(choice($.function_definition, $.meta)),
        kw("end"),
      ),

    modifier: () =>
      choice(
        kw("pub"),
        kw("inline"),
        kw("noinline"),
        kw("pure"),
        kw("noret"),
        kw("deprecated"),
        kw("mut"),
        "static",
        "\u0627\u06cc\u0633\u062a\u0627",
      ),

    function_signature: ($) =>
      prec.right(
        seq(
          repeat($.modifier),
          kw("func"),
          field("name", $.name),
          optional($.type_parameters),
          optional($.parameter_list),
          optional($.return_type),
        ),
      ),

    function_definition: ($) =>
      seq(
        repeat($.modifier),
        kw("func"),
        field("name", choice($.name, $.operator_name)),
        optional($.type_parameters),
        optional($.parameter_list),
        ":",
        optional(seq(field("return_type", $._type), ":")),
        repeat($._statement),
        kw("end"),
      ),

    operator_name: () =>
      choice(
        "+",
        "-",
        "*",
        "/",
        "%",
        "**",
        "^^",
        "==",
        "!=",
        "<",
        ">",
        "<=",
        ">=",
        "[]",
        "[]=",
        kw("not"),
        kw("eq"),
        kw("ne"),
        kw("lt"),
        kw("gt"),
        kw("le"),
        kw("ge"),
      ),

    return_type: ($) => seq(":", field("type", $._type)),

    type_parameters: ($) =>
      seq(
        "<",
        commaSep1(
          seq($.name, optional(seq(":", $._type, repeat(seq("+", $._type))))),
        ),
        ">",
      ),

    parameter_list: ($) => seq("(", commaSep(choice($.parameter, "...")), ")"),

    parameter: ($) =>
      seq(
        optional(kw("mut")),
        field("name", $.name),
        optional(seq(choice(":", "&:"), field("type", $._type))),
        optional(seq("=", field("default", $._expression))),
      ),

    block: ($) => seq(":", repeat($._statement), kw("end")),

    struct_definition: ($) =>
      seq(
        repeat($.modifier),
        kw("struct"),
        field("name", $.name),
        optional($.type_parameters),
        ":",
        repeat(
          choice(
            $.field_declaration,
            $.function_definition,
            $.use_clause,
            $.meta,
            $.const_declaration,
            $.variable_declaration,
          ),
        ),
        kw("end"),
      ),

    field_declaration: ($) =>
      prec.right(
        seq(
          repeat($.modifier),
          field(
            "name",
            choice(
              $.name,
              kw("in"),
              kw("type"),
              kw("to"),
              kw("on"),
              kw("as"),
              kw("input"),
              kw("step"),
            ),
          ),
          ":",
          field("type", $._type),
          optional(seq("=", field("default", $._expression))),
        ),
      ),

    use_clause: ($) =>
      seq(
        optional(kw("pub")),
        choice("use", "\u0634\u0627\u0645\u0644"),
        $._type,
      ),

    interface_definition: ($) =>
      seq(
        repeat($.modifier),
        kw("interface"),
        field("name", $.name),
        optional($.type_parameters),
        ":",
        repeat(
          choice(
            $.function_definition,
            $.function_signature,
            $.use_clause,
            $.meta,
          ),
        ),
        kw("end"),
      ),

    enum_definition: ($) =>
      seq(
        repeat($.modifier),
        kw("enum"),
        field("name", $.name),
        optional($.type_parameters),
        optional(seq(":", field("base", $._type))),
        ":",
        repeat(
          seq(
            choice($.enum_member, $.function_definition, $.meta),
            optional(COMMA),
          ),
        ),
        kw("end"),
      ),

    enum_member: ($) =>
      seq(
        field("name", $.name),
        optional(seq("(", commaSep(choice($.parameter, $._type)), ")")),
        optional(seq("=", field("value", $._expression))),
      ),

    impl_definition: ($) =>
      seq(
        kw("impl"),
        field("interface", $._type),
        kw("on"),
        field("type", $._type),
        ":",
        repeat($.function_definition),
        kw("end"),
      ),

    type_alias: ($) =>
      prec.right(
        seq(
          repeat($.modifier),
          kw("type"),
          field("name", $.name),
          choice("=", ":"),
          field("type", $._type),
          optional(
            seq(repeat1(choice($.function_definition, $.meta)), kw("end")),
          ),
        ),
      ),

    layout_block: ($) =>
      seq(
        alias($._layout_keyword, "page"),
        optional(field("name", $.name)),
        optional($.parameter_list),
        ":",
        repeat($._layout_item),
        kw("end"),
      ),

    component_definition: ($) =>
      seq(
        alias($._component_keyword, "component"),
        field("name", $.name),
        optional($.parameter_list),
        ":",
        repeat($._layout_item),
        kw("end"),
      ),

    _layout_item: ($) => choice($.layout_element, $.layout_attribute, $.meta),

    layout_element: ($) =>
      seq(field("tag", $.layout_name), ":", repeat($._layout_item), kw("end")),

    layout_attribute: ($) =>
      seq(field("name", $.layout_name), "=", field("value", $._expression)),

    _statement: ($) =>
      choice(
        $.variable_declaration,
        $.const_declaration,
        $.assignment,
        $.if_statement,
        $.until_statement,
        $.each_statement,
        $.repeat_statement,
        $.switch_statement,
        $.return_statement,
        $.break_statement,
        $.continue_statement,
        $.defer_statement,
        $.print_statement,
        $.expression_statement,
      ),

    variable_declaration: ($) =>
      prec.right(
        choice(
          seq(
            optional(kw("pub")),
            optional("static"),
            optional(kw("mut")),
            field("name", $.name),
            optional(seq(COMMA, $.name)),
            ":=",
            field("value", choice($._expression, $.try_expression)),
          ),
          seq(
            optional(kw("pub")),
            kw("mut"),
            field("name", $.name),
            ":",
            field("type", $._type),
            optional(seq("=", field("value", $._expression))),
          ),
        ),
      ),

    const_declaration: ($) =>
      prec.right(
        seq(
          optional(kw("pub")),
          optional("static"),
          kw("const"),
          field("name", $.name),
          optional(seq(":", $._type)),
          ":=",
          field("value", $._expression),
        ),
      ),

    assignment: ($) =>
      prec.right(
        seq(
          field("left", $._expression),
          field(
            "operator",
            choice(
              "=",
              "+=",
              "-=",
              "*=",
              "/=",
              "%=",
              "^^=",
              "**=",
              "&=",
              "|=",
              "^=",
              "<<=",
              ">>=",
            ),
          ),
          field("right", choice($._expression, $.try_expression)),
        ),
      ),

    if_statement: ($) =>
      seq(
        kw("if"),
        field("condition", $._expression),
        ":",
        repeat($._statement),
        repeat($.else_clause),
        kw("end"),
      ),

    else_clause: ($) =>
      prec.right(
        seq(
          kw("else"),
          optional(kw("if")),
          optional(field("condition", $._expression)),
          ":",
          repeat($._statement),
        ),
      ),

    until_statement: ($) =>
      seq(
        kw("until"),
        field("condition", $._expression),
        field("body", $.block),
      ),

    each_statement: ($) =>
      seq(
        kw("each"),
        field("item", $.name),
        optional(seq(COMMA, field("value", $.name))),
        kw("in"),
        field("iterable", $._expression),
        field("body", $.block),
      ),

    repeat_statement: ($) =>
      seq(
        kw("repeat"),
        field("count", $._expression),
        optional(seq(kw("to"), field("end", $._expression))),
        optional(seq(kw("step"), field("step", $._expression))),
        optional(seq(kw("in"), field("index", $.name))),
        field("body", $.block),
      ),

    switch_statement: ($) =>
      seq(
        kw("switch"),
        field("value", $._expression),
        ":",
        repeat($.switch_case),
        kw("end"),
      ),

    switch_case: ($) =>
      seq(
        choice(
          commaSep1(
            choice(seq($._expression, kw("to"), $._expression), $._expression),
          ),
          kw("else"),
        ),
        ":",
        repeat($._statement),
        kw("end"),
      ),

    return_statement: ($) =>
      prec.right(
        seq(kw("ret"), optional(choice($._expression, $.try_expression))),
      ),

    break_statement: () => prec.right(seq(kw("break"), optional(/[0-9]+/))),

    continue_statement: () => kw("continue"),

    defer_statement: ($) =>
      seq(
        kw("defer"),
        choice($._expression, $.print_statement, $.assignment, $.block),
      ),

    print_statement: ($) =>
      prec.right(
        seq(
          choice(kw("print"), kw("println"), kw("printerr"), kw("printerrln")),
          commaSep($._expression),
        ),
      ),

    expression_statement: ($) =>
      prec(-1, choice($._expression, $.try_expression)),

    _expression: ($) =>
      choice(
        $.binary_expression,
        $.unary_expression,
        $.ternary_expression,
        $.cast_expression,
        $.call_expression,
        $.member_expression,
        $.index_expression,
        $.slice_expression,
        $.parenthesized_expression,
        $.array_literal,
        $.struct_literal,
        $.lambda,
        $.match_expression,
        $.input_expression,
        $.function_address,
        $.name,
        $.this,
        $.dimension,
        $.integer,
        $.float,
        $.string,
        $.raw_string,
        $.text_block,
        $.char,
        $.boolean,
        $.null,
      ),

    binary_expression: ($) => {
      const table = [
        [PREC.or, choice("||", kw("or"))],
        [PREC.and, choice("&&", kw("and"))],
        [
          PREC.compare,
          choice(
            "==",
            "!=",
            "<",
            ">",
            "<=",
            ">=",
            kw("eq"),
            kw("ne"),
            kw("lt"),
            kw("gt"),
            kw("le"),
            kw("ge"),
            seq(kw("lt"), kw("eq")),
            seq(kw("gt"), kw("eq")),
          ),
        ],
        [PREC.bitor, "|"],
        [PREC.bitxor, "^"],
        [PREC.bitand, "&"],
        [PREC.shift, choice("<<", ">>")],
        [PREC.add, choice("+", "-", "\uff0b", "\uff0d", "\u2212")],
        [PREC.mul, choice("*", "/", "%", "\u00d7", "\u00f7")],
      ];
      return choice(
        ...table.map(([p, op]) =>
          prec.left(
            p,
            seq(
              field("left", $._expression),
              field("operator", op),
              field("right", $._expression),
            ),
          ),
        ),
        prec.right(
          PREC.power,
          seq(
            field("left", $._expression),
            field("operator", choice("^^", "**")),
            field("right", $._expression),
          ),
        ),
      );
    },

    unary_expression: ($) =>
      choice(
        prec(
          PREC.not,
          seq(field("operator", kw("not")), field("operand", $._expression)),
        ),
        prec(
          PREC.unary,
          seq(
            field("operator", choice("-", "!", "~", "+")),
            field("operand", $._expression),
          ),
        ),
        prec(
          PREC.unary,
          seq(
            field("operator", choice("++", "--")),
            field("operand", $._expression),
          ),
        ),
        prec(
          PREC.postfix,
          seq(
            field("operand", $._expression),
            field("operator", choice("++", "--")),
          ),
        ),
      ),

    ternary_expression: ($) =>
      prec.right(
        PREC.ternary,
        seq(
          field("condition", $._expression),
          choice("?", "\u061f"),
          field("then", $._expression),
          ":",
          field("else", $._expression),
        ),
      ),

    cast_expression: ($) =>
      prec.left(
        PREC.cast,
        seq(field("value", $._expression), kw("as"), field("type", $._type)),
      ),

    try_expression: ($) => seq($._expression, choice("?", "\u061f")),

    call_expression: ($) =>
      prec(
        PREC.postfix,
        seq(
          field("function", $._expression),
          optional($.type_arguments),
          field("arguments", $.argument_list),
        ),
      ),

    argument_list: ($) =>
      seq("(", commaSep(choice($._expression, $.named_argument)), ")"),

    named_argument: ($) =>
      seq(field("name", $.name), "=", field("value", $._expression)),

    type_arguments: ($) => prec.dynamic(-1, seq("<", commaSep1($._type), ">")),

    member_expression: ($) =>
      prec(
        PREC.postfix,
        seq(
          field("object", $._expression),
          ".",
          field("member", $.member_name),
        ),
      ),

    index_expression: ($) =>
      prec(
        PREC.postfix,
        seq(
          field("object", $._expression),
          "[",
          field("index", $._expression),
          "]",
        ),
      ),

    slice_expression: ($) =>
      prec(
        PREC.postfix,
        seq(
          field("object", $._expression),
          "[",
          optional($._expression),
          ":",
          optional($._expression),
          "]",
        ),
      ),

    parenthesized_expression: ($) => seq("(", $._expression, ")"),

    array_literal: ($) =>
      seq("[", repeat(seq($._expression, optional(COMMA))), "]"),

    struct_literal: ($) =>
      prec(
        PREC.postfix,
        seq(
          field("type", $.type_name),
          "{",
          repeat(
            seq(choice($.field_initializer, $._expression), optional(COMMA)),
          ),
          "}",
        ),
      ),

    field_initializer: ($) =>
      seq(field("name", $.name), "=", field("value", $._expression)),

    lambda: ($) =>
      prec(PREC.postfix, seq($.parameter_list, field("body", $.block))),

    match_expression: ($) =>
      seq(
        kw("match"),
        field("value", $._expression),
        ":",
        repeat($.match_arm),
        kw("end"),
      ),

    match_arm: ($) =>
      seq(
        field(
          "pattern",
          commaSep1(
            choice(
              seq($.dotted_name, optional(seq("(", commaSep($.name), ")"))),
              $.struct_literal,
              $.integer,
              $.string,
              $.char,
              "_",
              kw("else"),
            ),
          ),
        ),
        optional(seq(kw("if"), field("guard", $._expression))),
        ":",
        repeat($._statement),
        kw("end"),
      ),

    input_expression: ($) =>
      prec.right(seq(kw("input"), optional($._expression))),

    function_address: ($) => prec(PREC.unary, seq("&", $.dotted_name)),

    _type: ($) =>
      choice(
        $.type_name,
        $.pointer_type,
        $.array_type,
        $.slice_type,
        $.function_type,
        $.dyn_type,
        $.variant_type,
      ),

    type_name: ($) =>
      prec.right(
        seq($.dotted_name, optional(seq("<", commaSep1($._type), ">"))),
      ),

    pointer_type: ($) => prec.dynamic(1, prec(20, seq($._type, "*"))),

    array_type: ($) => prec(20, seq($._type, "[", $._expression, "]")),

    slice_type: ($) => prec(20, seq($._type, "[", optional(":"), "]")),

    function_type: ($) =>
      prec.right(
        seq(kw("func"), "(", commaSep($._type), ")", optional($._type)),
      ),

    dyn_type: ($) => seq(alias($._dyn_keyword, "dyn"), $._type),

    variant_type: ($) => prec.left(seq($._type, "|", $._type)),

    dotted_name: ($) =>
      prec.right(seq($.name, repeat(seq(".", $.member_name)))),

    this: () => kw("this"),

    boolean: () => choice(kw("true"), kw("false")),

    null: () => kw("null"),

    integer: () =>
      token(
        choice(
          /0[xX][0-9a-fA-F_]+/,
          /0[bB][01_]+/,
          /0[oO][0-7_]+/,
          /[0-9\u0660-\u0669\u06f0-\u06f9][0-9\u0660-\u0669\u06f0-\u06f9_]*/,
        ),
      ),

    float: () =>
      token(
        choice(
          /[0-9\u0660-\u0669\u06f0-\u06f9][0-9\u0660-\u0669\u06f0-\u06f9_]*[.\u066b][0-9\u0660-\u0669\u06f0-\u06f9_]+([eE][+-]?[0-9]+)?[fF]?/,
          /[0-9][0-9_]*[eE][+-]?[0-9]+[fF]?/,
          /[0-9][0-9_]*[fF]/,
        ),
      ),

    string: ($) =>
      choice(
        seq(
          '"',
          repeat(
            choice($.escape_sequence, token.immediate(prec(1, /[^"\\\n]+/))),
          ),
          '"',
        ),
        seq(
          "«",
          repeat(
            choice($.escape_sequence, token.immediate(prec(1, /[^»\\\n]+/))),
          ),
          "»",
        ),
        token(seq('"""', repeat(choice(/[^"]/, /"[^"]/, /""[^"]/)), '"""')),
      ),

    escape_sequence: () =>
      token.immediate(
        seq(
          "\\",
          choice(
            /[^xuU]/,
            /x[0-9a-fA-F]{2}/,
            /u[0-9a-fA-F]{4}/,
            /U[0-9a-fA-F]{8}/,
          ),
        ),
      ),

    raw_string: () => token(seq("`", /[^`]*/, "`")),

    text_block: () =>
      token(seq("```", repeat(choice(/[^`]/, /`[^`]/, /``[^`]/)), "```")),

    char: () =>
      token(
        seq(
          optional("u"),
          "'",
          choice(
            /[^'\\]/,
            /\\./,
            /\\x[0-9a-fA-F]{2}/,
            /\\u[0-9a-fA-F]{4}/,
            /\\U[0-9a-fA-F]{8}/,
          ),
          "'",
        ),
      ),

    dimension: () =>
      token(
        prec(
          2,
          /[0-9]+(\.[0-9]+)?(px|em|rem|deg|vh|vw|vmin|vmax|ms|s|fr|pt|ch|ex|%)/,
        ),
      ),

    line_comment: () => token(seq("//", /[^\n]*/)),

    block_comment: () => token(seq("/*", /[^*]*\*+([^/*][^*]*\*+)*/, "/")),
  },
});
