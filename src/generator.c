#include "generator.h"

/**
 *
 * @function generator_create
 * @brief Create a generator
 * @params {ast_t*} ast - AST tree
 * @returns {generator_t*}
 *
 */
generator_t* generator_create(ast_t* ast)
{
    if (ast == NULL) {
        error(2, "AST tree is NULL and not correct!");
    }

    generator_t* generator = memory_allocate(sizeof(generator_t));
    generator->ast = ast;
    generator->html = string_create(4096);
    generator->css = string_create(4096);
    generator->js = string_create(4096);

    return generator;
}

/**
 *
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 *
 */
void generator_destroy(generator_t* generator)
{
    if (generator != NULL) {
        if (generator->html != NULL) {
            generator->html->destroy(generator->html);
        }

        if (generator->css != NULL) {
            generator->css->destroy(generator->css);
        }

        if (generator->js != NULL) {
            generator->js->destroy(generator->js);
        }

        memory_destroy(generator);
    }
}

/**
 *
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_debug(generator_t* generator)
{
    if (generator == NULL) {
        printf("generator is NULL\n");
        return;
    }

    printf("generator->html: ");
    if (generator->html != NULL) generator->html->print(generator->html);
    else printf("NULL\n");

    printf("generator->css: ");
    if (generator->css != NULL) generator->css->print(generator->css);
    else printf("NULL\n");

    printf("generator->js: ");
    if (generator->js != NULL) generator->js->print(generator->js);
    else printf("NULL\n");

}

/**
 *
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_save(generator_t* generator)
{
    if (generator == NULL) {
        return;
    }

    if (generator->html != NULL) {
        file_writes("index.html", generator->html->data);
    }

    if (generator->css != NULL) {
        file_writes("style.css", generator->css->data);
    }

    if (generator->js != NULL) {
        file_writes("script.js", generator->js->data);
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
void generator_code_layout_html(ast_layout_block_t* layout_block, string_t* html)
{
    // LANG
    array_t* html_lang = hashmap_get(cast(hashmap_t*, layout_block->attributes), "lang");
    char* html_lang_value = NULL;
    string_append_str(html, " lang=\"");
    if (html_lang != NULL) html_lang_value = string_lower_str(array_string(html_lang, 0));
    if (html_lang_value == NULL || strcmp(html_lang_value, "") == 0) {
        string_append_str(html, "fa-IR"); // default
    }
    else if (strcmp(html_lang_value, "fa") == 0 || strcmp(html_lang_value, "fa-ir") == 0 || strcmp(html_lang_value, "fa_ir") == 0) {
        string_append_str(html, "fa-IR");
    }
    else if (strcmp(html_lang_value, "en") == 0 || strcmp(html_lang_value, "en-us") == 0 || strcmp(html_lang_value, "en_us") == 0) {
        string_append_str(html, "en-US");
    }
    else {
        error(2, "Invalid value for lang attribute in layout block!");
    }
    string_append_str(html, "\"");

    // DIR
    array_t* html_dir = hashmap_get(cast(hashmap_t*, layout_block->attributes), "dir");
    char* html_dir_value = NULL;
    string_append_str(html, " dir=\"");
    if (html_dir != NULL) html_dir_value = string_lower_str(array_string(html_dir, 0));
    if (html_dir_value == NULL || strcmp(html_dir_value, "") == 0) {
        string_append_str(html, "rtl"); // default
    }
    else if (strcmp(html_dir_value, "ltr") == 0) {
        string_append_str(html, "ltr");
    }
    else if (strcmp(html_dir_value, "rtl") == 0) {
        string_append_str(html, "rtl");
    }
    else {
        error(2, "Invalid value for dir attribute in layout block!");
    }
    string_append_str(html, "\"");

    if (html_lang_value != NULL) memory_destroy(html_lang_value);
    if (html_dir_value != NULL) memory_destroy(html_dir_value);

    if (html_dir != NULL) html_dir->destroy(html_dir);
    if (html_lang != NULL) html_lang->destroy(html_lang);
}

/**
 *
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {string_t*}
 *
 */
string_t* generator_code_layout_attributes(ast_layout_block_t* block)
{
    size_t html_attributes_capacity = 0;
    string_t* html_attributes = string_create(1024);

    if (block != NULL) {
        if (block->attributes != NULL) {
            hashmap_t* attributes = cast(hashmap_t*, block->attributes);

            for (size_t i = 0; i < attributes->capacity; i++) {
                hashmap_entry_t *entry = attributes->data[i];

                while (entry) {
                    ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);

                    if (attribute->isContent != true && attribute->isStyle != true) {
                        array_t* attribute_values = cast(array_t*, attribute->values);
                        char* attribute_values_str = array_string_token(attribute_values, ", ");
                        size_t attribute_value_length = strlen(attribute_values_str);

                        if (html_attributes_capacity != 0) string_append_char(html_attributes, ' ');

                        string_append_str(html_attributes, entry->key);
                        string_append_str(html_attributes, "=");

                        if (attribute_value_length > 1) string_append_str(html_attributes, "\"");
                        string_append_str(html_attributes, attribute_values_str);
                        if (attribute_value_length > 1) string_append_str(html_attributes, "\"");

                        if (attribute_values_str != NULL) memory_destroy(attribute_values_str);

                        html_attributes_capacity++;
                    }
                    entry = entry->next;
                }
            }

        }
    }

    return html_attributes;
}

char* generator_code_layout_node_type(ast_layout_node_type_t type)
{
    switch (type) {
        case AST_LAYOUT_NODE_TYPE_LINK: return "link";
        case AST_LAYOUT_NODE_TYPE_SCRIPT: return "script";
        case AST_LAYOUT_NODE_TYPE_STYLE: return "style";
        case AST_LAYOUT_NODE_TYPE_DIV: return "div";
        case AST_LAYOUT_NODE_TYPE_HEADER: return "header";
        case AST_LAYOUT_NODE_TYPE_FOOTER: return "footer";
        case AST_LAYOUT_NODE_TYPE_NAV: return "nav";
        case AST_LAYOUT_NODE_TYPE_MAIN: return "main";
        case AST_LAYOUT_NODE_TYPE_SECTION: return "section";
        case AST_LAYOUT_NODE_TYPE_ARTICLE: return "article";
        case AST_LAYOUT_NODE_TYPE_ASIDE: return "aside";
        case AST_LAYOUT_NODE_TYPE_PARAGRAPH: return "p";
        case AST_LAYOUT_NODE_TYPE_UL: return "ul";
        case AST_LAYOUT_NODE_TYPE_OL: return "ol";
        case AST_LAYOUT_NODE_TYPE_LI: return "li";
        case AST_LAYOUT_NODE_TYPE_IMG: return "img";
        case AST_LAYOUT_NODE_TYPE_TABLE: return "table";
        case AST_LAYOUT_NODE_TYPE_TABLE_TR: return "tr";
        case AST_LAYOUT_NODE_TYPE_TABLE_TD: return "td";
        case AST_LAYOUT_NODE_TYPE_TABLE_TH: return "th";
        case AST_LAYOUT_NODE_TYPE_FORM: return "form";
        case AST_LAYOUT_NODE_TYPE_INPUT: return "input";
        case AST_LAYOUT_NODE_TYPE_BUTTON: return "button";
        case AST_LAYOUT_NODE_TYPE_TEXTAREA: return "textarea";
        case AST_LAYOUT_NODE_TYPE_SELECT: return "select";
        case AST_LAYOUT_NODE_TYPE_OPTION: return "option";
        case AST_LAYOUT_NODE_TYPE_LABEL: return "label";
        case AST_LAYOUT_NODE_TYPE_IFRAME: return "iframe";
        case AST_LAYOUT_NODE_TYPE_CANVAS: return "canvas";
        case AST_LAYOUT_NODE_TYPE_AUDIO: return "audio";
        case AST_LAYOUT_NODE_TYPE_VIDEO: return "video";
        case AST_LAYOUT_NODE_TYPE_BLOCKQUOTE: return "blockquote";
        case AST_LAYOUT_NODE_TYPE_PRE: return "pre";
        case AST_LAYOUT_NODE_TYPE_CODE: return "code";
        case AST_LAYOUT_NODE_TYPE_BR: return "br";
        case AST_LAYOUT_NODE_TYPE_HR: return "hr";
        case AST_LAYOUT_NODE_TYPE_SPAN: return "span";
        case AST_LAYOUT_NODE_TYPE_STRONG: return "strong";
        case AST_LAYOUT_NODE_TYPE_EM: return "em";
        case AST_LAYOUT_NODE_TYPE_ITALIC: return "i";
        case AST_LAYOUT_NODE_TYPE_BOLD: return "b";
        case AST_LAYOUT_NODE_TYPE_UNDERLINE: return "u";
        case AST_LAYOUT_NODE_TYPE_S: return "s";
        case AST_LAYOUT_NODE_TYPE_SMALL: return "small";
        case AST_LAYOUT_NODE_TYPE_BIG: return "big";
        case AST_LAYOUT_NODE_TYPE_SUB: return "sub";
        case AST_LAYOUT_NODE_TYPE_SUP: return "sup";
        case AST_LAYOUT_NODE_TYPE_CENTER: return "center";
        case AST_LAYOUT_NODE_TYPE_DEL: return "del";
        case AST_LAYOUT_NODE_TYPE_INS: return "ins";
        case AST_LAYOUT_NODE_TYPE_MARK: return "mark";
        case AST_LAYOUT_NODE_TYPE_Q: return "q";
        case AST_LAYOUT_NODE_TYPE_CITE: return "cite";
        case AST_LAYOUT_NODE_TYPE_DFN: return "dfn";
        case AST_LAYOUT_NODE_TYPE_ADDRESS: return "address";
        case AST_LAYOUT_NODE_TYPE_TIME: return "time";
        case AST_LAYOUT_NODE_TYPE_PROGRESS: return "progress";
        case AST_LAYOUT_NODE_TYPE_METER: return "meter";
        case AST_LAYOUT_NODE_TYPE_DETAILS: return "details";
        case AST_LAYOUT_NODE_TYPE_SUMMARY: return "summary";
        case AST_LAYOUT_NODE_TYPE_DIALOG: return "dialog";
        case AST_LAYOUT_NODE_TYPE_MENU: return "menu";
        case AST_LAYOUT_NODE_TYPE_MENUITEM: return "menuitem";
        case AST_LAYOUT_NODE_TYPE_COMMAND: return "command";
        case AST_LAYOUT_NODE_TYPE_LEGEND: return "legend";
        case AST_LAYOUT_NODE_TYPE_FIELDSET: return "fieldset";
        case AST_LAYOUT_NODE_TYPE_CAPTION: return "caption";
        case AST_LAYOUT_NODE_TYPE_COL: return "col";
        case AST_LAYOUT_NODE_TYPE_COLGROUP: return "colgroup";
        case AST_LAYOUT_NODE_TYPE_TABLE_HEADER: return "thead";
        case AST_LAYOUT_NODE_TYPE_TABLE_BODY: return "tbody";
        case AST_LAYOUT_NODE_TYPE_TABLE_FOOTER: return "tfoot";
        
        case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW:
        case AST_LAYOUT_NODE_TYPE_ERROR: return "";
    }
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
string_t* generator_code_layout_block(generator_t* generator, array_t* children)
{
    string_t* html = string_create(1024);

    for (size_t i = 0; i < children->length; i++) {
        ast_layout_node_t* node = array_get(children, i);

        string_t* layout_block_str = string_create(1024);
        string_t* node_attrs_str = generator_code_layout_attributes(node->block);
        char* node_name = ast_layout_node_type_to_name(node->type);

        string_append_char(layout_block_str, '<');
        string_append_str(layout_block_str, node_name);
        if (node_attrs_str->length > 0) {
            string_append_char(layout_block_str, ' ');
            string_append(layout_block_str, node_attrs_str);
        }
        if (node_attrs_str != NULL) node_attrs_str->destroy(node_attrs_str);
        string_append_str(layout_block_str, ">");

        if (node->block->children->length > 0 || node->block->text_content != NULL) {
            if (node->block->text_content != NULL) {
                // string_append_char(layout_block_str, '{');
                if (node->block->children->length == 0 && strchr(node->block->text_content, '\n') == NULL) {
                    string_append_str(layout_block_str, node->block->text_content);
                    // string_append_char(layout_block_str, '}');
                }
                else {
                    string_append_char(layout_block_str, '\n');
                    string_append_str(layout_block_str, node->block->text_content);
                    // string_append_char(layout_block_str, '}');
                    string_append_char(layout_block_str, '\n');
                }
            }

            if (node->block->children->length > 0) {
                string_t* layout_block_children = generator_code_layout_block(generator, node->block->children);

                if (layout_block_children->length > 0) string_append(layout_block_str, layout_block_children);
                if (layout_block_children != NULL) layout_block_children->destroy(layout_block_children);
            }

        }

        string_append_str(layout_block_str, "</");
        string_append_str(layout_block_str, node_name);
        string_append_str(layout_block_str, ">\n");

        string_append(html, layout_block_str);

        if (layout_block_str != NULL) layout_block_str->destroy(layout_block_str);
    }

    return html;
}

void generator_code_layout_body(generator_t* generator, ast_layout_block_t* layout_block, string_t* body)
{
    string_t* body_tag = string_create(1024);
    string_t* body_content = string_create(1024);

    string_append_str(body_tag, "<body");
    string_t* body_attrs = generator_code_layout_attributes(layout_block);
    if (body_attrs->length > 0) string_append_char(body_tag, ' ');
    string_append(body_tag, body_attrs);
    body_attrs->destroy(body_attrs);
    string_append_str(body_tag, ">");

    string_t* body_child = generator_code_layout_block(generator, layout_block->children);
    if (body_child->length > 0) string_append_char(body_tag, '\n');
    string_append(body_content, body_child);
    body_child->destroy(body_child);

    string_append(body_tag, body_content);
    string_append_str(body_tag, "</body>\n");

    string_set(body, body_tag);
    body_tag->destroy(body_tag);
    body_content->destroy(body_content);
}

/**
 *
 * @function generator_code
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code(generator_t* generator)
{
    if (generator->ast == NULL) {
        error(2, "AST tree is NULL and is not valid!");
    }

    ast_t* ast = generator->ast;

    if (generator->ast->layout != NULL) {
        ast_layout_t* layout = ast->layout;
        ast_layout_block_t* layout_block = NULL;

        if (layout->block != NULL) {
            layout_block = layout->block;

            string_t* head = string_create(1024);
            string_t* body = string_create(1024);
            string_t* html = string_create(1024);

            string_append_str(html, "<!doctype html>\n");
            string_append_str(html, "<html");

            generator_code_layout_html(layout_block, html);

            string_append_str(html, ">\n");

            string_append_str(html, "<head>\n");
            string_append_str(html, "<meta charset=\"UTF-8\">\n");

            string_append(html, head);
            string_append_str(html, "</head>\n");

            generator_code_layout_body(generator, layout_block, body);

            string_append(html, body);

            string_append_str(html, "</html>");

            string_set(generator->html, html);

            string_destroy(head);
            string_destroy(body);
            string_destroy(html);
        }
    }
}
