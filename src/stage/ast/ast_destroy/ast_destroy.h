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
#include <stage/ast/asts/ast_parameter/ast_parameter_direct_destroy/ast_parameter_direct_destroy.h>
#include <stage/ast/asts/ast_parameters/ast_parameters_direct_destroy/ast_parameters_direct_destroy.h>
#include <stage/ast/asts/ast_attribute/ast_attribute_direct_destroy/ast_attribute_direct_destroy.h>
#include <stage/ast/asts/ast_attributes/ast_attributes_direct_destroy/ast_attributes_direct_destroy.h>
#include <stage/ast/asts/ast_argument/ast_argument_direct_destroy/ast_argument_direct_destroy.h>
#include <stage/ast/asts/ast_arguments/ast_arguments_direct_destroy/ast_arguments_direct_destroy.h>

// #include <stage/ast/asts/ast_variable_decl/ast_variable_decl_destroy/ast_variable_decl_destroy.h>
// #include <stage/ast/asts/ast_function_decl/ast_function_decl_destroy/ast_function_decl_destroy.h>
// #include <stage/ast/asts/ast_block/ast_block_destroy/ast_block_destroy.h>
// #include <stage/ast/asts/ast_program/ast_program_destroy/ast_program_destroy.h>
// #include <stage/ast/asts/ast_parameter/ast_parameter_destroy/ast_parameter_destroy.h>
// #include <stage/ast/asts/ast_parameters/ast_parameters_destroy/ast_parameters_destroy.h>
// #include <stage/ast/asts/ast_attribute/ast_attribute_destroy/ast_attribute_destroy.h>
// #include <stage/ast/asts/ast_attributes/ast_attributes_destroy/ast_attributes_destroy.h>
// #include <stage/ast/asts/ast_argument/ast_argument_destroy/ast_argument_destroy.h>
// #include <stage/ast/asts/ast_arguments/ast_arguments_destroy/ast_arguments_destroy.h>

void ast_destroy(ast_t* ast);

#endif // _STAGE_AST_AST_DESTROY_AST_DESTROY_H_
