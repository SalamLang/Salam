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
    ast_layout_attribute_t* html_lang = hashmap_get(cast(hashmap_t*, layout_block->attributes), "lang");
    char* html_lang_value = NULL;
    string_append_str(html, " lang=\"");
    if (html_lang != NULL) {
        char* values = array_string_token(html_lang->values, 0);
        html_lang_value = string_lower_str(values);
        
        if (values != NULL) memory_destroy(values);
    }
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
        if (html_lang_value != NULL) memory_destroy(html_lang_value);

        error(2, "Invalid value for lang attribute in layout block!");
    }
    string_append_str(html, "\"");

    // DIR
    ast_layout_attribute_t* html_dir = hashmap_get(cast(hashmap_t*, layout_block->attributes), "dir");
    char* html_dir_value = NULL;
    string_append_str(html, " dir=\"");
    if (html_dir != NULL) {
        char* values = array_string_token(html_dir->values, 0);
        html_dir_value = string_lower_str(values);

        if (values != NULL) memory_destroy(values);
    }
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
        if (html_lang_value != NULL) memory_destroy(html_lang_value);
        if (html_dir_value != NULL) memory_destroy(html_dir_value);

        error(2, "Invalid value for dir attribute in layout block!");
    }
    string_append_str(html, "\"");

    if (html_lang_value != NULL) memory_destroy(html_lang_value);
    if (html_dir_value != NULL) memory_destroy(html_dir_value);
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
    size_t html_attributes_length = 0;
    size_t css_attributes_length = 0;

    string_t* html_attributes = string_create(1024);
    string_t* css_attributes = string_create(1024);

    if (block != NULL) {
        if (block->attributes != NULL) {
            hashmap_t* attributes = cast(hashmap_t*, block->attributes);

            for (size_t i = 0; i < attributes->capacity; i++) {
                hashmap_entry_t *entry = attributes->data[i];

                while (entry) {
                    ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);
                    if (attribute == NULL) {
                        entry = entry->next;
                        continue;
                    }

                    if (attribute->ignoreMe == true || attribute->isContent == true || attribute->isStyle == true) {}
                    else {
                        array_t* attribute_values = cast(array_t*, attribute->values);
                        char* attribute_values_str = array_string_token(attribute_values, ", ");
                        size_t attribute_value_length = strlen(attribute_values_str);

                        if (html_attributes_length != 0) string_append_char(html_attributes, ' ');

                        string_append_str(html_attributes, entry->key);
                        string_append_str(html_attributes, "=");

                        if (attribute_value_length > 1) string_append_str(html_attributes, "\"");
                        string_append_str(html_attributes, attribute_values_str);
                        if (attribute_value_length > 1) string_append_str(html_attributes, "\"");

                        if (attribute_values_str != NULL) memory_destroy(attribute_values_str);

                        html_attributes_length++;
                    }
                    
                    entry = entry->next;
                }
            }
        }

        if (block->styles != NULL) {
            hashmap_t* styles = cast(hashmap_t*, block->styles);

            for (size_t i = 0; i < styles->capacity; i++) {
                hashmap_entry_t *entry = styles->data[i];

                while (entry) {
                    ast_layout_attribute_t* attribute = cast(ast_layout_attribute_t*, entry->value);

                    if (attribute->isStyle == false) {}
                    else {
                        char* attribute_css_name = generator_code_layout_style_name(attribute->type);
                        char* attribute_css_value = generator_code_layout_style_value(attribute, block->parent_node_type);

                        if (css_attributes_length != 0) string_append_char(css_attributes, ';');
                        string_append_str(css_attributes, attribute_css_name);
                        string_append_str(css_attributes, ":");
                        string_append_str(css_attributes, attribute_css_value);

                        if (attribute_css_value != NULL) memory_destroy(attribute_css_value);

                        css_attributes_length++;
                    }
                    
                    entry = entry->next;
                }
            }
        }
    }

    if (css_attributes_length > 0 && css_attributes->length > 0) {
        if (html_attributes_length > 0 && html_attributes->length > 0) string_append_char(html_attributes, ' ');

        string_append_str(html_attributes, "style=\"");
        string_append(html_attributes, css_attributes);
        string_append_str(html_attributes, "\"");
    }

    if (css_attributes != NULL) css_attributes->destroy(css_attributes);

    return html_attributes;
}

/**
 * 
 * @function generator_code_layout_style_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @params {ast_layout_node_type_t} parent_node_type - Parent Node Type
 * @returns {char*} values_str - Values
 * 
 */
char* generator_code_layout_style_value(ast_layout_attribute_t* attribute, ast_layout_node_type_t parent_node_type)
{
    char* values_str = array_string_token(attribute->values, ", ");

    if (parent_node_type == AST_LAYOUT_NODE_TYPE_TABLE) {}

    return values_str;
}

/**
 * 
 * @function generator_code_layout_style_name
 * @brief Convert AST layout attribute type to CSS attribute name
 * @params {ast_layout_attribute_type_t} type - Layout Attribute Type
 * @returns {char*} name - Name
 * 
 */
char* generator_code_layout_style_name(ast_layout_attribute_type_t type)
{
	switch (type) {
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
			return  "background";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
			return  "color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
			return  "font";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
			return  "font-family";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
			return  "font-size";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
			return  "font-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
			return  "font-weight";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
			return  "height";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
			return  "width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
			return  "margin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
			return  "margin-left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
			return  "margin-right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
			return  "margin-top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
			return  "margin-bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
			return  "padding";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
			return  "padding-left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
			return  "padding-right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
			return  "padding-top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
			return  "padding-bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN:
			return  "text-align";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
			return  "text-decoration";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM:
			return  "text-transform";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN:
			return  "vertical-align";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY:
			return  "visibility";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX:
			return  "z-index";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION:
			return  "position";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
			return  "top";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
			return  "right";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
			return  "bottom";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
			return  "left";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER:
			return  "border";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR:
			return  "border-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE:
			return  "border-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH:
			return  "border-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS:
			return  "border-radius";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT:
			return  "float";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR:
			return  "clear";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW:
			return  "overflow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW:
			return  "text-overflow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE:
			return  "white-space";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR:
			return  "cursor";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER:
			return  "filter";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY:
			return  "opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM:
			return  "transform";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION:
			return  "transition";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION:
			return  "animation";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW:
			return  "box-shadow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW:
			return  "text-shadow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE:
			return  "outline";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR:
			return  "outline-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE:
			return  "outline-style";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH:
			return  "outline-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
			return  "background-color";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE:
			return  "background-image";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT:
			return  "background-repeat";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION:
			return  "background-position";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE:
			return  "background-size";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT:
			return  "background-attachment";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP:
			return  "background-clip";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN:
			return  "background-origin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE:
			return  "background-blend-mode";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY:
			return  "display";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX:
			return  "flex";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION:
			return  "flex-direction";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP:
			return  "flex-wrap";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW:
			return  "flex-flow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW:
			return  "flex-grow";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK:
			return  "flex-shrink";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS:
			return  "flex-basis";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT:
			return  "justify-content";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS:
			return  "align-items";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF:
			return  "align-self";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT:
			return  "align-content";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER:
			return  "order";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID:
			return  "grid";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL:
			return  "fill";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE:
			return  "stroke";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH:
			return  "stroke-width";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP:
			return  "stroke-linecap";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN:
			return  "stroke-linejoin";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY:
			return  "stroke-dasharray";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET:
			return  "stroke-dashoffset";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY:
			return  "fill-opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY:
			return  "stroke-opacity";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING:
			return  "shape-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING:
			return  "text-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING:
			return  "image-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION:
			return  "color-interpolation";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING:
			return  "color-rendering";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT:
			return  "vector-effect";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK:
			return  "mask";
		case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH:
			return  "clip-path";
		case AST_LAYOUT_ATTRIBUTE_TYPE_ERROR:
			return "error";
        
        default:
            return "error????";
	}
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
        case AST_LAYOUT_NODE_TYPE_NONE:
        case AST_LAYOUT_NODE_TYPE_ERROR:
            return "";
    }

    return "error?";
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

/**
 * 
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 * 
 */
void generator_code_layout_body(generator_t* generator, ast_layout_block_t* layout_block, string_t* body)
{
    if (layout_block == NULL) {
        validate_layout_mainbody(layout_block);
    }
    
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

        if (layout->block != NULL) {
            string_t* head = string_create(1024);
            string_t* body = string_create(1024);
            string_t* html = string_create(1024);

            string_append_str(html, "<!doctype html>\n");
            string_append_str(html, "<html");

            generator_code_layout_html(layout->block, html);

            string_append_str(html, ">\n");

            string_append_str(html, "<head>\n");
            string_append_str(html, "<meta charset=\"UTF-8\">\n");

            string_append(html, head);
            string_append_str(html, "</head>\n");

            generator_code_layout_body(generator, layout->block, body);

            string_append(html, body);

            string_append_str(html, "</html>");

            string_set(generator->html, html);

            string_destroy(head);
            string_destroy(body);
            string_destroy(html);
        }
    }
}
