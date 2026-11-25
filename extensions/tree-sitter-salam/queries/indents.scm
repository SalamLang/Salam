[
  (function_definition)
  (struct_definition)
  (interface_definition)
  (enum_definition)
  (impl_definition)
  (extern_block)
  (export_block)
  (layout_block)
  (layout_element)
  (component_definition)
  (conditional_block)
  (if_statement)
  (else_clause)
  (until_statement)
  (each_statement)
  (repeat_statement)
  (switch_statement)
  (switch_case)
  (match_expression)
  (match_arm)
  (lambda)
  (block)
  (argument_list)
  (parameter_list)
] @indent.begin

(else_clause "else" @indent.branch)
(else_clause "وگرنه" @indent.branch)

["end" "پایان"] @indent.end @indent.branch

(argument_list ")" @indent.end @indent.branch)
(parameter_list ")" @indent.end @indent.branch)

[
  (line_comment)
  (block_comment)
  (text_block)
] @indent.ignore

(ERROR) @indent.auto
