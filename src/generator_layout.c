#include "generator_layout.h"

/**
 *
 * @function generator_code_layout_block_item
 * @brief Generate the HTML code for the layout block item
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_node_t*} node - Node
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_block_item(generator_t *generator,
                                           ast_layout_node_t *node) {
    string_t *layout_block_str = string_create(1024);
    string_t *node_attrs_str =
        generator_code_layout_attributes(generator, node->block);
    char *node_name = generator_code_layout_node_type(node->type);

    hashmap_t *attributes = node->block->attributes;

    size_t repeat_value_sizet = 1;
    ast_layout_attribute_t *repeat = hashmap_get(attributes, "repeat");
    ast_value_t *repeat_value = NULL;

    if (repeat != NULL && repeat->values->length > 1) {
        error_generator(1, "The 'repeat' attribute must have only one value");
    } else if (repeat != NULL && repeat->values->length == 1) {
        repeat_value = array_get(repeat->values, 0);

        if (repeat_value->type->kind == AST_TYPE_KIND_STRING) {
            if (string_is_integer(repeat_value->data.string_value) == false) {
                error_generator(
                    1, "The 'repeat' attribute must be an integer value");
            } else {
                repeat_value_sizet = atoi(repeat_value->data.string_value);
            }
        } else if (repeat_value->type->kind == AST_TYPE_KIND_INT) {
            repeat_value_sizet = repeat_value->data.int_value;
        } else {
            error_generator(1,
                            "The 'repeat' attribute must be an integer value");
        }
    }

    if (repeat_value_sizet < 1) {
        error_generator(1,
                        "The 'repeat' attribute value must be greater than 0");
    } else if (repeat_value_sizet > 1000) {
        error_generator(
            1, "The 'repeat' attribute value must be less or equal to 1000");
    }

    if (node->type == AST_LAYOUT_TYPE_INCLUDE) {
        ast_layout_attribute_t *src = hashmap_get(attributes, "src");
        ast_value_t *src_value = NULL;

        if (src == NULL) {
            error_generator(1, "Include node must have a 'src' attribute");
        } else if (src->values->length > 1) {
            error_generator(
                1, "Include node 'src' attribute must have only one value");
        }

        src_value = array_get(src->values, 0);
        if (src_value->type->kind != AST_TYPE_KIND_STRING) {
            error_generator(1, "Include node 'src' attribute must be a string");
        }

        char *path = src_value->data.string_value;

        if (!file_exists(path)) {
            error_generator(1, "Include file '%s' does not exist", path);
        }

        size_t size = 0;
        char *content = file_reads_binary(path, &size);
        lexer_t *lexer = lexer_create(path, content);
        lexer_lex(lexer);

        // lexer_save(lexer, "include-tokens.txt");

        ast_t *ast = parser_parse(lexer);

        if (ast->layout == NULL) {
            error_generator(1, "Include file '%s' does not have a layout block",
                            path);
        }
        // else if (ast->layout->block->children->length == 0)
        // {
        // 	error_generator(1, "Include file '%s' layout block does not have
        // any children", path);
        // }
        else {
            for (size_t i = 1; i <= repeat_value_sizet; i++) {
                if (ast->layout->block->text_content != NULL) {
                    string_append_str(layout_block_str,
                                      ast->layout->block->text_content);
                }

                string_t *block_code = generator_code_layout_block(
                    generator, ast->layout->block->children);

                string_append(layout_block_str, block_code);

                string_destroy(block_code);
            }
        }

        ast_destroy(ast);
        lexer_destroy(lexer);
        memory_destroy(content);
    } else {
        for (size_t i = 1; i <= repeat_value_sizet; i++) {
            string_append_char(layout_block_str, '<');
            string_append_str(layout_block_str, node_name);

            if (node->type == AST_LAYOUT_TYPE_INPUT &&
                node->block->text_content != NULL) {
                string_append_str(node_attrs_str, " value=\"");
                string_append_str(
                    node_attrs_str,
                    node->block
                        ->text_content);  // TODO: we need to bypass inner \"
                string_append_str(node_attrs_str, "\"");
            }

            if (node_attrs_str->length > 0) {
                string_append_char(layout_block_str, ' ');
                string_append(layout_block_str, node_attrs_str);
            }

            string_append_str(layout_block_str, ">");

            if (node->block->children->length > 0 ||
                (node->block->text_content != NULL &&
                 node->type != AST_LAYOUT_TYPE_INPUT)) {
                bool has_content = false;

                if (node->block->text_content != NULL) {
                    if (node->block->children->length == 0 &&
                        strchr(node->block->text_content, '\n') == NULL) {
                        string_append_str(layout_block_str,
                                          node->block->text_content);
                    } else {
                        string_append_char(layout_block_str, '\n');
                        string_append_str(layout_block_str,
                                          node->block->text_content);
                        string_append_char(layout_block_str, '\n');

                        has_content = true;
                    }
                }

                if (node->block->children->length > 0) {
                    string_t *layout_block_children =
                        generator_code_layout_block(generator,
                                                    node->block->children);

                    if (has_content == false) {
                        string_append_char(layout_block_str, '\n');
                    }

                    if (layout_block_children->length > 0) {
                        string_append(layout_block_str, layout_block_children);
                    }

                    if (layout_block_children != NULL) {
                        layout_block_children->destroy(layout_block_children);
                    }
                }
            }

            if (is_layout_node_a_single_tag(node->type) == false) {
                string_append_str(layout_block_str, "</");
                string_append_str(layout_block_str, node_name);
                string_append_str(layout_block_str, ">\n");
            } else {
                string_append_char(layout_block_str, '\n');
            }
        }
    }

    if (node_attrs_str != NULL) {
        node_attrs_str->destroy(node_attrs_str);
    }

    return layout_block_str;
}

/**
 *
 * @function generator_code_layout_block
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @params {array_t*} children - Children
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_block(generator_t *generator,
                                      array_t *children) {
    DEBUG_ME;
    string_t *html = string_create(1024);

    for (size_t i = 0; i < children->length; i++) {
        ast_layout_node_t *node = array_get(children, i);
        string_t *node_html = generator_code_layout_block_item(generator, node);

        string_append(html, node_html);

        string_destroy(node_html);
    }

    return html;
}

/**
 *
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 *
 */
void generator_code_layout_body(generator_t *generator,
                                ast_layout_block_t *layout_block,
                                string_t *body) {
    DEBUG_ME;
    if (layout_block != NULL) {
        validate_layout_mainbody(layout_block);
    }

    string_t *body_tag = string_create(1024);
    string_t *body_content = string_create(1024);

    string_append_str(body_tag, "<body");
    string_t *body_attrs =
        generator_code_layout_attributes(generator, layout_block);
    if (body_attrs->length > 0) {
        string_append_char(body_tag, ' ');
    }
    string_append(body_tag, body_attrs);
    body_attrs->destroy(body_attrs);
    string_append_str(body_tag, ">");

    char *body_text_content = layout_block->text_content;
    size_t body_text_content_length =
        body_text_content != NULL ? strlen(body_text_content) : 0;

    string_t *body_child =
        generator_code_layout_block(generator, layout_block->children);
    if (body_child->length > 0 || body_text_content_length > 0) {
        string_append_char(body_tag, '\n');
    }

    // text content
    if (body_text_content != NULL && body_text_content_length > 0) {
        string_append_str(body_content, body_text_content);
    }

    // node content
    if (body_child->length > 0 && body_text_content != NULL &&
        body_text_content_length > 0) {
        string_append_char(body_content, '\n');
    }
    if (body_child->length > 0) {
        string_append(body_content, body_child);
    }

    if (body_child != NULL) body_child->destroy(body_child);

    string_append(body_tag, body_content);

    string_set(body, body_tag);
    body_tag->destroy(body_tag);
    body_content->destroy(body_content);
}

/**
 *
 * @function generator_code_head_item
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head_item(ast_layout_attribute_t *attribute,
                              string_t *head) {
    DEBUG_ME;
    if (head == NULL) {
        return;
    }

    char *value = NULL;

    switch (attribute->type) {
        case AST_LAYOUT_ATTRIBUTE_TYPE_TITLE:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<title>");
            string_append_str(head, value);
            string_append_str(head, "</title>");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta name=\"author\" content=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta name=\"description\" content=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta name=\"keywords\" content=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta charset=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta name=\"viewport\" content=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        case AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH:
            value = array_value_stringify(attribute->values, ", ");

            string_append_str(head, "<meta http-equiv=\"refresh\" content=\"");
            string_append_str(head, value);
            string_append_str(head, "\">");
            string_append_char(head, '\n');
            break;

        default:
            break;
    }

    if (value != NULL) {
        memory_destroy(value);
    }
}

/**
 *
 * @function generator_code_head_meta_children
 * @brief Generate the CSS/HTML code for meta children in the layout
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head_meta_children(generator_t *generator,
                                       ast_layout_block_t *block,
                                       string_t *head) {
    DEBUG_ME;
    if (head == NULL) {
        return;
    } else if (block == NULL) {
        return;
    }

    if (block->meta_children != NULL) {
        size_t meta_children_length = block->meta_children->length;
        for (size_t i = 0; i < meta_children_length; i++) {
            ast_layout_node_t *node = array_get(block->meta_children, i);
            ast_layout_block_t *node_block = node->block;

            if (node->type == AST_LAYOUT_TYPE_FONT) {
                string_append_str(generator->css, "@font-face{");
                hashmap_t *attributes =
                    cast(hashmap_t *, node_block->attributes);

                size_t attributes_length = attributes->length;
                size_t attributes_append_length = 0;

                size_t attributes_capacity = attributes->capacity;

                for (size_t i = 0; i < attributes_capacity; i++) {
                    hashmap_entry_t *entry = attributes->data[i];

                    while (entry) {
                        ast_layout_attribute_t *attribute =
                            cast(ast_layout_attribute_t *, entry->value);

                        if (attribute->final_key == NULL) {
                            attribute->final_key = string_strdup(entry->key);
                        }

                        string_append_str(
                            generator->css,
                            attribute->final_key);  // TODO: Why name lowercase
                                                    // entry->key?
                        string_append_char(generator->css, ':');

                        if (attribute->final_value == NULL) {
                            attribute->final_value =
                                array_value_stringify(attribute->values, ", ");
                        }

                        string_append_str(generator->css,
                                          attribute->final_value);

                        if (attributes_append_length != attributes_length - 1) {
                            string_append_char(generator->css, ';');
                        }

                        entry = cast(hashmap_entry_t *, entry->next);
                        attributes_append_length++;
                    }
                }

                string_append_char(generator->css, '}');
            }
        }
    }
}

/**
 *
 * @function generator_code_head
 * @brief Generate the code for the head
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head(generator_t *generator, ast_layout_block_t *block,
                         string_t *head) {
    DEBUG_ME;
    if (head == NULL) {
        return;
    } else if (block == NULL) {
        return;
    }

    // size_t html_tags_length = 0;

    if (block->attributes != NULL) {
        hashmap_t *attributes = cast(hashmap_t *, block->attributes);
        size_t attribute_capacity = attributes->capacity;

        for (size_t i = 0; i < attribute_capacity; i++) {
            hashmap_entry_t *entry = attributes->data[i];

            while (entry) {
                ast_layout_attribute_t *attribute =
                    cast(ast_layout_attribute_t *, entry->value);

                if (attribute->isStyle == true ||
                    attribute->isContent == true) {
                } else {
                    generator_code_head_item(attribute, head);

                    // html_tags_length++;
                }

                entry = cast(hashmap_entry_t *, entry->next);
            }
        }
    }

    if (block->meta_children != NULL) {
        generator_code_head_meta_children(generator, block, head);
    }
}

/**
 *
 * @function generator_code_layout
 * @brief Generate code for AST layout
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code_layout(generator_t *generator) {
    DEBUG_ME;
    if (generator->ast->layout != NULL) {
        if (generator->ast->layout->block != NULL) {
            string_t *head = string_create(1024);
            string_t *body = string_create(1024);
            string_t *html = string_create(1024);

            // Process the layout block
            generator_code_layout_body(generator, generator->ast->layout->block,
                                       body);

            // Process the head block
            generator_code_head(generator, generator->ast->layout->block, head);

            // Generate the HTML code
            string_append_str(html, "<!doctype html>\n");
            string_append_str(html, "<html");

            generator_code_layout_html(generator->ast->layout->block, html);

            string_append_str(html, ">\n");

            string_append_str(html, "<head>\n");
            string_append_str(html, "<meta charset=\"UTF-8\">\n");

            string_append(html, head);

            if ((generator->css != NULL && generator->css->length > 0) ||
                (generator->media_css != NULL &&
                 generator->media_css->length > 0)) {
                if (generator->inlineCSS == true) {
                    string_append_str(html, "<style>\n");
                    if (generator->css != NULL && generator->css->length > 0) {
                        string_append(html, generator->css);
                        string_append_char(html, '\n');
                    }
                    if (generator->media_css != NULL &&
                        generator->media_css->length > 0) {
                        string_append(html, generator->media_css);
                        string_append_char(html, '\n');
                    }
                    string_append_str(html, "</style>\n");
                } else {
                    string_append_str(
                        html, "<link rel=\"stylesheet\" href=\"style.css\">\n");
                }
            }

            string_append_str(html, "</head>\n");

            string_append(html, body);

            if (generator->js != NULL && generator->js->length > 0) {
                if (generator->inlineJS == true) {
                    string_append_str(html, "<script>\n");
                    string_append(html, generator->js);
                    string_append_str(html, "</script>\n");
                } else {
                    string_append_str(html,
                                      "<script src=\"script.js\"></script>\n");
                }
            }

            string_append_str(html, "</body>\n");
            string_append_str(html, "</html>");

            string_set(generator->html, html);

            string_destroy(head);
            string_destroy(body);
            string_destroy(html);
        }
    }
}

/**
 *
 * @function generator_code_layout_html
 * @brief Generate the HTML code for the layout block
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} html - HTML string
 * @returns {void}
 *
 */
void generator_code_layout_html(ast_layout_block_t *layout_block,
                                string_t *html) {
    DEBUG_ME;
    // LANG
    ast_layout_attribute_t *html_lang =
        hashmap_get(cast(hashmap_t *, layout_block->attributes), "lang");
    char *html_lang_value = NULL;
    string_append_str(html, " lang=\"");
    if (html_lang != NULL) {
        char *values = array_value_stringify(html_lang->values, ", ");
        html_lang_value = string_lower_str(values);

        if (values != NULL) {
            memory_destroy(values);
        }
    }
    if (html_lang_value == NULL || strcmp(html_lang_value, "") == 0) {
        string_append_str(html, "fa-IR");
    } else if (strcmp(html_lang_value, "fa") == 0 ||
               strcmp(html_lang_value, "fa-ir") == 0 ||
               strcmp(html_lang_value, "fa_ir") == 0) {
        string_append_str(html, "fa-IR");
    } else if (strcmp(html_lang_value, "en") == 0 ||
               strcmp(html_lang_value, "en-us") == 0 ||
               strcmp(html_lang_value, "en_us") == 0) {
        string_append_str(html, "en-US");
    } else {
        if (html_lang_value != NULL) {
            memory_destroy(html_lang_value);
        }

        error_generator(2, "Invalid value for lang attribute in layout block!");
    }
    string_append_str(html, "\"");

    // DIR
    ast_layout_attribute_t *html_dir =
        hashmap_get(cast(hashmap_t *, layout_block->attributes), "dir");
    char *html_dir_value = NULL;
    string_append_str(html, " dir=\"");
    if (html_dir != NULL) {
        char *values = array_value_stringify(html_dir->values, ", ");
        html_dir_value = string_lower_str(values);

        if (values != NULL) {
            memory_destroy(values);
        }
    }
    if (html_dir_value == NULL || strcmp(html_dir_value, "") == 0) {
        string_append_str(html, "rtl");
    } else if (strcmp(html_dir_value, "ltr") == 0) {
        string_append_str(html, "ltr");
    } else if (strcmp(html_dir_value, "rtl") == 0) {
        string_append_str(html, "rtl");
    } else {
        if (html_lang_value != NULL) {
            memory_destroy(html_lang_value);
        }
        if (html_dir_value != NULL) {
            memory_destroy(html_dir_value);
        }

        error_generator(2, "Invalid value for dir attribute in layout block!");
    }
    string_append_str(html, "\"");

    if (html_lang_value != NULL) {
        memory_destroy(html_lang_value);
    }
    if (html_dir_value != NULL) {
        memory_destroy(html_dir_value);
    }
}

/**
 *
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {generator_t} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_attributes(generator_t *generator,
                                           ast_layout_block_t *block) {
    DEBUG_ME;
    size_t html_attributes_length = 0;
    size_t css_attributes_length = 0;

    string_t *html_attributes = string_create(1024);
    string_t *css_attributes = string_create(1024);

    if (block != NULL) {
        if (block->attributes != NULL) {
            hashmap_t *attributes = cast(hashmap_t *, block->attributes);
            size_t attributes_capacity = attributes->capacity;

            for (size_t i = 0; i < attributes_capacity; i++) {
                hashmap_entry_t *entry = attributes->data[i];

                while (entry) {
                    ast_layout_attribute_t *attribute =
                        cast(ast_layout_attribute_t *, entry->value);
                    if (attribute == NULL) {
                        entry = cast(hashmap_entry_t *, entry->next);
                        continue;
                    }

                    if (attribute->ignoreMe == true ||
                        attribute->isContent == true ||
                        attribute->isStyle == true) {
                    } else {
                        if (attribute->final_value == NULL) {
                            attribute->final_value =
                                array_value_stringify(attribute->values, ", ");
                        }

                        size_t attribute_value_length =
                            attribute->final_value == NULL
                                ? 0
                                : strlen(attribute->final_value);

                        if (html_attributes_length != 0) {
                            string_append_char(html_attributes, ' ');
                        }

                        if (attribute->final_key == NULL) {
                            attribute->final_key = string_strdup(entry->key);
                        }

                        string_append_str(
                            html_attributes,
                            attribute->final_key);  // TODO: Why name lowercase
                                                    // entry->key?
                        string_append_str(html_attributes, "=");

                        if (attribute_value_length > 1) {
                            string_append_str(html_attributes, "\"");
                        }
                        string_append_str(html_attributes,
                                          attribute->final_value);
                        if (attribute_value_length > 1) {
                            string_append_str(html_attributes, "\"");
                        }

                        html_attributes_length++;
                    }

                    entry = cast(hashmap_entry_t *, entry->next);
                }
            }
        }

        string_t *this_style = generator_code_layout_styles(
            block->styles->normal, block, &css_attributes_length);
        string_append(css_attributes, this_style);
        string_destroy(this_style);

        // New styles
        size_t styles_new_capacity = block->styles->new->capacity;

        for (size_t i = 0; i < styles_new_capacity; i++) {
            hashmap_entry_t *entry = block->styles->new->data[i];

            while (entry) {
                ast_layout_attribute_t *attribute =
                    cast(ast_layout_attribute_t *, entry->value);
                if (attribute == NULL) {
                } else if (attribute->isStyle == false ||
                           attribute->ignoreMe == true) {
                } else {
                    if (css_attributes_length != 0) {
                        string_append_char(css_attributes, ';');
                    }
                    string_append_str(css_attributes, attribute->final_key);
                    string_append_str(css_attributes, ":");
                    string_append_str(css_attributes, attribute->final_value);

                    css_attributes_length++;
                }

                entry = cast(hashmap_entry_t *, entry->next);
            }
        }
    }

    bool has_substate = false;

    if (hashmap_has_any_sub_value_layout_attribute_style_state(block->states) ==
        true) {
        has_substate = true;
    }

    bool first_load = true;

    if (block->tag != NULL) {
        first_load = false;
    }

    if ((block->meta_children != NULL && block->meta_children->length > 0) ||
        block->styles->normal->length > 0 || block->styles->new->length > 0 ||
        has_substate == true) {
        if (block->tag == NULL) {
            block->tag = generator_identifier_get(
                generator->identifier);  // We will free its memory in the
                                         // block_layout_destroy function
        }
    }

    size_t media_queries_length = 0;
    size_t meta_children_length = block->meta_children->length;

    if (block->meta_children != NULL) {
        for (size_t i = 0; i < meta_children_length; i++) {
            ast_layout_node_t *node = block->meta_children->data[i];
            ast_layout_block_t *node_block = node->block;

            if (node->type != AST_LAYOUT_TYPE_MEDIA) {
                continue;
            }
            ast_layout_attribute_t *media_max_width =
                hashmap_get(node_block->attributes, "responsive_max_width");
            ast_layout_attribute_t *media_min_width =
                hashmap_get(node_block->attributes, "responsive_min_width");
            ast_layout_attribute_t *media_max_height =
                hashmap_get(node_block->attributes, "responsive_max_height");
            ast_layout_attribute_t *media_min_height =
                hashmap_get(node_block->attributes, "responsive_min_height");

            string_append_str(
                generator->media_css,
                "@media only screen and (");  // NOTE: WE HAVE TO HAVE A SPACE
                                              // AFTER `AND`

            size_t conditions = 0;
            size_t media_queries_styles_length = 0;

            if (media_max_width != NULL) {
                if (media_max_width->values->length > 1) {
                    error_generator(2,
                                    "The responsive_max_width attribute can "
                                    "only have one value!");

                    return NULL;
                } else if (validate_style_value_size(node_block->styles->normal,
                                                     node_block->styles->new,
                                                     media_max_width, NULL,
                                                     NULL) == false) {
                    error_generator(2,
                                    "Invalid value for responsive_max_width "
                                    "attribute in layout block!");

                    return NULL;
                }

                if (conditions > 0) {
                    string_append_str(generator->media_css, " and ");
                }

                string_append_str(generator->media_css, "max-width: ");
                string_append_str(generator->media_css,
                                  media_max_width->final_value);

                conditions++;
            }

            if (media_min_width != NULL) {
                if (media_min_width->values->length > 1) {
                    error_generator(2,
                                    "The responsive_max_width attribute can "
                                    "only have one value!");

                    return NULL;
                } else if (validate_style_value_size(node_block->styles->normal,
                                                     node_block->styles->new,
                                                     media_min_width, NULL,
                                                     NULL) == false) {
                    error_generator(2,
                                    "Invalid value for responsive_max_width "
                                    "attribute in layout block!");

                    return NULL;
                } else if (conditions > 0) {
                    string_append_str(generator->media_css, " and ");
                }

                string_append_str(generator->media_css, "min-width: ");
                string_append_str(generator->media_css,
                                  media_min_width->final_value);

                conditions++;
            }

            if (media_max_height != NULL) {
                if (media_max_height->values->length > 1) {
                    error_generator(2,
                                    "The responsive_max_width attribute can "
                                    "only have one value!");

                    return NULL;
                } else if (validate_style_value_size(node_block->styles->normal,
                                                     node_block->styles->new,
                                                     media_max_height, NULL,
                                                     NULL) == false) {
                    error_generator(2,
                                    "Invalid value for responsive_max_width "
                                    "attribute in layout block!");

                    return NULL;
                } else if (conditions > 0) {
                    string_append_str(generator->media_css, " and ");
                }

                string_append_str(generator->media_css, "max-height: ");
                string_append_str(generator->media_css,
                                  media_max_height->final_value);

                conditions++;
            }

            if (media_min_height != NULL) {
                if (media_min_height->values->length > 1) {
                    error_generator(2,
                                    "The responsive_max_width attribute can "
                                    "only have one value!");

                    return NULL;
                } else if (validate_style_value_size(node_block->styles->normal,
                                                     node_block->styles->new,
                                                     media_min_height, NULL,
                                                     NULL) == false) {
                    error_generator(2,
                                    "Invalid value for responsive_max_width "
                                    "attribute in layout block!");

                    return NULL;
                } else if (conditions > 0) {
                    string_append_str(generator->media_css, " and ");
                }

                string_append_str(generator->media_css, "min-height: ");
                string_append_str(generator->media_css,
                                  media_min_height->final_value);

                conditions++;
            }

            string_append_char(generator->media_css, ')');
            string_append_char(generator->media_css, '{');
            string_append_char(generator->media_css, STYLE_STYLE_LINKING);
            string_append_str(generator->media_css, block->tag);
            string_append_char(generator->media_css, '{');

            // Media styles
            size_t styles_normal_capacity =
                node_block->styles->normal->capacity;
            for (size_t i = 0; i < styles_normal_capacity; i++) {
                hashmap_entry_t *entry = node_block->styles->normal->data[i];

                while (entry) {
                    ast_layout_attribute_t *attribute =
                        cast(ast_layout_attribute_t *, entry->value);

                    generator_code_layout_style_value(
                        node_block->styles->normal, node_block->styles->new,
                        attribute);

                    if (attribute == NULL) {
                    } else if (attribute->isStyle == false ||
                               attribute->ignoreMe == true) {
                    } else {
                        if (media_queries_styles_length != 0) {
                            string_append_char(generator->media_css, ';');
                        }

                        if (attribute->final_key == NULL) {
                            attribute->final_key =
                                string_strdup(attribute->key);
                        }

                        string_append_str(generator->media_css,
                                          attribute->final_key);
                        string_append_str(generator->media_css, ":");

                        if (attribute->final_value == NULL) {
                            attribute->final_value =
                                array_value_stringify(attribute->values, ", ");
                        }

                        string_append_str(generator->media_css,
                                          attribute->final_value);

                        media_queries_styles_length++;
                    }

                    entry = cast(hashmap_entry_t *, entry->next);
                }
            }

            // Media new styles
            size_t styles_new_capacity = node_block->styles->new->capacity;

            for (size_t i = 0; i < styles_new_capacity; i++) {
                hashmap_entry_t *entry = node_block->styles->new->data[i];

                while (entry) {
                    ast_layout_attribute_t *attribute =
                        cast(ast_layout_attribute_t *, entry->value);
                    if (attribute == NULL) {
                    } else if (attribute->isStyle == false ||
                               attribute->ignoreMe == true) {
                    } else {
                        if (media_queries_styles_length != 0) {
                            string_append_char(generator->media_css, ';');
                        }

                        string_append_str(generator->media_css,
                                          attribute->final_key);
                        string_append_str(generator->media_css, ":");
                        string_append_str(generator->media_css,
                                          attribute->final_value);

                        media_queries_styles_length++;
                    }

                    entry = cast(hashmap_entry_t *, entry->next);
                }
            }

            string_append_char(generator->media_css, '}');
            string_append_char(generator->media_css, '}');

            media_queries_length++;
        }
    }

    if (((media_queries_length > 0 || css_attributes_length > 0) &&
         css_attributes->length > 0) ||
        has_substate == true) {
        if (html_attributes_length > 0 && html_attributes->length > 0) {
            string_append_char(html_attributes, ' ');
        }

        if (css_attributes->length > 0) {
            if (generator->inlineCSS == true &&
                (media_queries_length == 0 && has_substate == false)) {
                string_append_str(html_attributes, "style=\"");
                string_append(html_attributes, css_attributes);
                string_append_str(html_attributes, "\"");

                html_attributes_length++;
            } else {
                if (first_load) {
                    string_append_char(generator->css, STYLE_STYLE_LINKING);
                    string_append_str(generator->css, block->tag);
                    string_append_char(generator->css, '{');
                    string_append(generator->css, css_attributes);
                    string_append_char(generator->css, '}');
                }
            }
        }

        if (block->tag != NULL) {
            if (html_attributes_length > 0) {
                string_append_char(html_attributes, ' ');
            }

            size_t tag_length = strlen(block->tag);

            // string_append_str(html_attributes, "id=");
            string_append_str(html_attributes, "class=");
            if (tag_length > 1) {
                string_append_char(html_attributes, '\"');
            }

            string_append_str(html_attributes, block->tag);

            if (tag_length > 1) {
                string_append_char(html_attributes, '\"');
            }
            html_attributes_length++;
        }

        if (has_substate == true && first_load == true) {
            string_t *pseudo_elements = generator_code_layout_pseudo_elements(
                generator, block, &css_attributes_length);

            if (pseudo_elements != NULL) {
                string_append(generator->css, pseudo_elements);
                string_destroy(pseudo_elements);
            }
        }
    }

    if (css_attributes != NULL) {
        css_attributes->destroy(css_attributes);
    }

    return html_attributes;
}

/**
 *
 * @function generator_code_layout_node_type
 * @brief Convert AST layout node type to HTML node name
 * @params {ast_layout_node_type_t} type - Layout Node Type
 * @returns {char*} name - Name
 *
 */
char *generator_code_layout_node_type(ast_layout_node_type_t type) {
    DEBUG_ME;
    switch (type) {
#undef ADD_LAYOUT_TYPE
#undef ADD_LAYOUT_TYPE_HIDE
#undef ADD_LAYOUT_TYPE_REPEAT

#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, \
                        IS_MOTHER)                                            \
    case TYPE:                                                                \
        return GENERATED_NAME;
#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                             ENDUSER_NAME, IS_MOTHER)                \
    case TYPE:                                                       \
        return GENERATED_NAME;
#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                               ENDUSER_NAME, IS_MOTHER)

#include "ast_layout_type.h"
    }

    return "error?";
}

/**
 *
 * @function generator_code_layout_attribute_name
 * @brief Convert AST layout attribute type to HTML attribute name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 *
 */
char *generator_code_layout_attribute_name(ast_layout_attribute_type_t type) {
    DEBUG_ME;
    switch (type) {
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                        GENERATED_NAME, FILTER,               \
                                        ALLOWED_VALUES, SUBTAGS)              \
    case TYPE:                                                                \
        return "ERROR";
#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER,       \
                                             ENDUSER_NAME, GENERATED_NAME, \
                                             FILTER, ALLOWED_VALUES, SUBTAGS)

#include "ast_layout_attribute_style_type.h"

#undef ADD_LAYOUT_ATTRIBUTE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                                  ENDUSER_NAME)                           \
    case TYPE:                                                            \
        return GENERATED_NAME;
#define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, \
                                         GENERATED_NAME, ENDUSER_NAME)

#include "ast_layout_attribute_type.h"
    }

    return "error2????";
}
