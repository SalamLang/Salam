#ifndef _STAGE_AST_AST_DESTROY_AST_DESTROY_H_
#define _STAGE_AST_AST_DESTROY_AST_DESTROY_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// ast
#include <stage/ast/type.h>

#include <stage/ast/asts/ast_variable_decl/ast_variable_decl_direct_destroy/ast_variable_decl_direct_destroy.h>
#include <stage/ast/asts/ast_function_decl/ast_function_decl_direct_destroy/ast_function_decl_direct_destroy.h>
#include <stage/ast/asts/ast_block/ast_block_direct_destroy/ast_block_direct_destroy.h>
#include <stage/ast/asts/ast_program/ast_program_direct_destroy/ast_program_direct_destroy.h>
#include <stage/ast/asts/ast_parameter/ast_parameter_item/ast_parameter_item_direct_destroy/ast_parameter_item_direct_destroy.h>
#include <stage/ast/asts/ast_parameter/ast_parameters/ast_parameters_direct_destroy/ast_parameters_direct_destroy.h>
#include <stage/ast/asts/ast_attribute/ast_attribute_item/ast_attribute_item_direct_destroy/ast_attribute_item_direct_destroy.h>
#include <stage/ast/asts/ast_attribute/ast_attributes/ast_attributes_direct_destroy/ast_attributes_direct_destroy.h>
#include <stage/ast/asts/ast_argument/ast_argument_item/ast_argument_item_direct_destroy/ast_argument_item_direct_destroy.h>
#include <stage/ast/asts/ast_argument/ast_arguments/ast_arguments_direct_destroy/ast_arguments_direct_destroy.h>

#include <stage/ast/asts/ast_package/ast_package_direct_destroy/ast_package_direct_destroy.h>
#include <stage/ast/asts/ast_import/ast_import_direct_destroy/ast_import_direct_destroy.h>

#include <stage/ast/asts/ast_kind/ast_kind_decl/ast_kind_decl_direct_destroy/ast_kind_decl_direct_destroy.h>
#include <stage/ast/asts/ast_kind/ast_kind_struct/ast_kind_struct_direct_destroy/ast_kind_struct_direct_destroy.h>
#include <stage/ast/asts/ast_kind/ast_kind_enum/ast_kind_enum_direct_destroy/ast_kind_enum_direct_destroy.h>
#include <stage/ast/asts/ast_kind/ast_kind_union/ast_kind_union_direct_destroy/ast_kind_union_direct_destroy.h>

#include <stage/ast/asts/ast_expression/ast_expressions/ast_expressions_direct_destroy/ast_expressions_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_item/ast_expression_item_direct_destroy/ast_expression_item_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_literal/ast_expression_literal_direct_destroy/ast_expression_literal_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_identifier/ast_expression_identifier_direct_destroy/ast_expression_identifier_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_binary/ast_expression_binary_direct_destroy/ast_expression_binary_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_unary/ast_expression_unary_direct_destroy/ast_expression_unary_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_index/ast_expression_index_direct_destroy/ast_expression_index_direct_destroy.h>
#include <stage/ast/asts/ast_expression/ast_expression_call/ast_expression_call_direct_destroy/ast_expression_call_direct_destroy.h>

#include <stage/ast/asts/ast_statement/ast_statement_if/ast_statement_if_direct_destroy/ast_statement_if_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_for/ast_statement_for_direct_destroy/ast_statement_for_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_foreach/ast_statement_foreach_direct_destroy/ast_statement_foreach_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_return/ast_statement_return_direct_destroy/ast_statement_return_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_print/ast_statement_print_direct_destroy/ast_statement_print_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_variable_decl/ast_statement_variable_decl_direct_destroy/ast_statement_variable_decl_direct_destroy.h>
#include <stage/ast/asts/ast_statement/ast_statement_expression/ast_statement_expression_direct_destroy/ast_statement_expression_direct_destroy.h>

void ast_destroy(ast_t* ast);

#endif // _STAGE_AST_AST_DESTROY_AST_DESTROY_H_
