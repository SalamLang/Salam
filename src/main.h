#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdio.h>  // for printf

// ---------- utility
// file
#include <utility/file/file_exists/file_exists.h>
#include <utility/file/file_reads/file_reads.h>
#include <utility/file/file_write/file_write.h>

// memory
#include <utility/memory/memory_destroy/memory_destroy.h>

// log
#include <utility/log/log_fatal/log_fatal.h>

// io
#include <utility/io/io_directory/io_directory.h>

// ----------  stage
// scanner
#include <stage/scanner/scanner_create/scanner_create.h>
#include <stage/scanner/scanner_destroy/scanner_destroy.h>
#include <stage/scanner/scanner_json/scanner_json.h>
#include <stage/scanner/scanner_log/scanner_log.h>
#include <stage/scanner/scanner_scan/scanner_scan.h>
#include <stage/scanner/scanner_set/scanner_set.h>
#include <stage/scanner/type.h>

// parser
#include <stage/parser/parser_create/parser_create.h>
#include <stage/parser/parser_destroy/parser_destroy.h>
#include <stage/parser/parser_log/parser_log.h>
#include <stage/parser/parser_parse/parser_parse_all/parser_parse_all.h>
#include <stage/parser/type.h>

// ast
#include <stage/ast/ast_create/ast_create.h>
#include <stage/ast/ast_destroy/ast_destroy.h>
#include <stage/ast/ast_json/ast_json.h>
#include <stage/ast/ast_log/ast_log.h>
#include <stage/ast/type.h>

// generator

int main(int argc, char** argv);

#endif  // _MAIN_H_
