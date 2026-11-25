; Text objects for editors that follow the nvim-treesitter-textobjects names.

; A Salam block has no body node: its statements are children of the construct,
; so an inner object is the run of statements between the colon and end.

;; functions, methods and lambdas

(function_definition
  [
    (variable_declaration)
    (const_declaration)
    (assignment)
    (if_statement)
    (until_statement)
    (each_statement)
    (repeat_statement)
    (switch_statement)
    (return_statement)
    (break_statement)
    (continue_statement)
    (defer_statement)
    (print_statement)
    (expression_statement)
  ]+ @function.inner) @function.outer

(function_definition) @function.outer

(lambda body: (block (_)* @function.inner)) @function.outer

;; types and components

(struct_definition) @class.outer

(interface_definition) @class.outer

(enum_definition) @class.outer

(impl_definition) @class.outer

(struct_definition [(field_declaration) (function_definition) (use_clause) (const_declaration) (variable_declaration)]+ @class.inner)

(interface_definition [(function_signature) (function_definition) (meta)]+ @class.inner)

(enum_definition [(enum_member) (function_definition) (meta)]+ @class.inner)

(impl_definition (function_definition)+ @class.inner)

(component_definition) @class.outer

(component_definition [(layout_variable) (layout_element) (layout_attribute) (meta)]+ @class.inner)

;; page blocks and elements: page, box, button and the rest

(layout_block) @block.outer

(layout_block [(layout_variable) (layout_element) (layout_attribute) (meta)]+ @block.inner)

(layout_element) @block.outer

(layout_element [(layout_element) (layout_attribute) (meta)]+ @block.inner)

(layout_attribute) @attribute.outer

(layout_attribute value: (_) @attribute.inner)

;; conditionals, loops and blocks

(if_statement) @conditional.outer

(conditional_block) @conditional.outer

(switch_statement) @conditional.outer

(match_expression) @conditional.outer

(if_statement condition: (_) @conditional.inner)

(else_clause condition: (_) @conditional.inner)

(switch_case) @conditional.inner

(match_arm) @conditional.inner

(until_statement body: (block (_)* @loop.inner)) @loop.outer

(each_statement body: (block (_)* @loop.inner)) @loop.outer

(repeat_statement body: (block (_)* @loop.inner)) @loop.outer

(block) @block.outer

(block (_)* @block.inner)

;; parameters, arguments and calls

(parameter_list (parameter) @parameter.inner @parameter.outer)

(argument_list (_) @parameter.inner @parameter.outer)

(call_expression arguments: (argument_list) @call.inner) @call.outer

;; statements

(return_statement (_) @return.inner) @return.outer

(return_statement) @return.outer

(variable_declaration name: (name) @assignment.lhs value: (_) @assignment.rhs) @assignment.outer

(assignment left: (_) @assignment.lhs right: (_) @assignment.rhs) @assignment.outer

[(variable_declaration) (const_declaration) (assignment) (return_statement) (break_statement) (continue_statement) (defer_statement) (print_statement) (expression_statement)] @statement.outer

;; comments and numbers

[(line_comment) (block_comment)] @comment.outer

[(integer) (float) (dimension)] @number.inner
