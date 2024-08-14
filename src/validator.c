#include "validator.h"

/**
 *
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void validate_layout_block(ast_layout_block_t* block)
{
    hashmap_t* attributes = cast(hashmap_t*, block->attributes);

    ast_layout_attribute_t* attribute_content = hashmap_get(attributes, "content");
    if (attribute_content != NULL) {
        attribute_content->isContent = true;

        array_t* values = cast(array_t*, attribute_content->values);
        values->print(values);
        char* content = array_string_token(values, ", ");

        if (values->length > 0 && strlen(content) > 0) {
            printf("content: %s\n", content);
            block->text_content = content;
        }
        else {
            if (content != NULL) memory_destroy(content);
        }

        // ast_layout_node_t* text = ast_layout_node_create(AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW);
        // hashmap_put(cast(hashmap_t*, text->block->attributes), "content", values);
        // array_push(block->children, text);
    }
}

/**
 * 
 * @function is_attribute_type_in_array
 * @brief Check if the attribute type is in the array
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @params {ast_layout_attribute_type_t*} array - Array of attribute types
 * @params {size_t} array_size - Array size
 * @returns {bool} - True if the attribute type is in the array, false otherwise
 * 
 */
bool is_attribute_type_in_array(ast_layout_attribute_type_t type, ast_layout_attribute_type_t* array, size_t array_size)
{
    for (size_t i = 0; i < array_size; i++) {
        if (array[i] == type) {
            return true;
        }
    }
    
    return false;
}

/**
 * 
 * @function token_belongs_to_ast_layout_node
 * @brief Check if the token belongs to the AST layout node
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @params {ast_layout_attribute_t*} attribute - AST layout attribute
 * @returns {bool} - True if the token belongs to the AST layout node, false otherwise
 * 
 */
bool token_belongs_to_ast_layout_node(ast_layout_block_t* block, ast_layout_attribute_type_t attribute_key_type, ast_layout_attribute_t* attribute)
{
    if (block->parent_type != AST_NODE_TYPE_LAYOUT) {
        error(2, "Block parent type is not a layout node");
    }

    if (is_layout_node_a_single_tag(block->parent_node_type) && attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return false;
    }
    else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return true;
    }
    else if (is_style_attribute(attribute_key_type)) {
        attribute->isStyle = true;
        return true;
    }
    else if (is_attribute_type_a_style(attribute_key_type)) {
        // attribute->isStyle = false;
        return true;
    }

    // LAYOUT
    if (block->parent_node_type == AST_LAYOUT_NODE_TYPE_NONE) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
            AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
            AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
            AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
            AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
            AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
            AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
            AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
        };

        size_t valid_attributes_size = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_size)) {
            return true;
        }
    }
    // IMG
    else if (block->parent_node_type == AST_LAYOUT_NODE_TYPE_IMG) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
        };

        size_t valid_attributes_size = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_size)) {
            return true;
        }
    }

    return false;
}

/**
 * 
 * @function is_layout_node_a_single_tag
 * @brief Check if the layout node is a single tag
 * @params {ast_layout_node_type_t} type - Layout node type
 * @returns {bool} - True if the layout node is a single tag, false otherwise
 * 
 */
bool is_layout_node_a_single_tag(ast_layout_node_type_t type)
{
    switch (type) {
        case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW:
        case AST_LAYOUT_NODE_TYPE_INPUT:
        case AST_LAYOUT_NODE_TYPE_BR:
        case AST_LAYOUT_NODE_TYPE_HR:
            return true;

        default:
            return false;
    }
}

/**
 * 
 * @function validate_layout_mainbody
 * @brief Validate the layout mainbody
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_mainbody(ast_layout_block_t* block)
{
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
        AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
        AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
        AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
        AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
        AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
        AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
        AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
    };
    size_t valid_attributes_size = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

    if (block->attributes != NULL) {
		if (block->attributes->data != NULL) {

            size_t attributes_capacity = block->attributes->capacity;

			for (size_t i = 0; i < attributes_capacity; i++) {
				hashmap_entry_t *entry = block->attributes->data[i];

				while (entry) {
                    char* attribute_key = entry->key;
                    ast_layout_attribute_type_t attribute_key_type = name_to_ast_layout_attribute_type(attribute_key);
                    ast_layout_attribute_t* attribute_value = entry->value;
                    
                    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_size)) {
                        attribute_value->ignoreMe = true;
                    }

        			entry = entry->next;
				}
			}
		}
    }
}

/**
 * 
 * @function is_style_attribute
 * @brief Check if the attribute type is a CSS attribute
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 * 
 */
bool is_style_attribute(ast_layout_attribute_type_t type)
{
    switch (type) {
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_FAMILY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_STYLE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_WEIGHT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_HEIGHT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WIDTH:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_LEFT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_RIGHT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_TOP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MARGIN_BOTTOM:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_LEFT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_RIGHT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_TOP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_PADDING_BOTTOM:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_ALIGN:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_DECORATION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_TRANSFORM:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VERTICAL_ALIGN:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VISIBILITY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_Z_INDEX:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_POSITION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TOP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_RIGHT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOTTOM:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_LEFT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_COLOR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_STYLE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_WIDTH:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BORDER_RADIUS:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLOAT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLEAR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OVERFLOW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_OVERFLOW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_WHITE_SPACE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CURSOR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILTER:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OPACITY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSFORM:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TRANSITION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ANIMATION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BOX_SHADOW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_SHADOW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_COLOR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_STYLE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_OUTLINE_WIDTH:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_COLOR:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_IMAGE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_REPEAT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_POSITION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_SIZE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ATTACHMENT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_CLIP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_ORIGIN:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_BACKGROUND_BLEND_MODE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_DISPLAY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_DIRECTION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_WRAP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_FLOW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_GROW:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_SHRINK:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FLEX_BASIS:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_JUSTIFY_CONTENT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_ITEMS:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_SELF:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ALIGN_CONTENT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_ORDER:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_GRID:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_WIDTH:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINECAP:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_LINEJOIN:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHARRAY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_DASHOFFSET:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FILL_OPACITY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_STROKE_OPACITY:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_SHAPE_RENDERING:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_TEXT_RENDERING:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_IMAGE_RENDERING:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_INTERPOLATION:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_COLOR_RENDERING:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_VECTOR_EFFECT:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_MASK:
        case AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_CLIP_PATH:
            return true;

        default:
            return false;
    }
}

/**
 * 
 * @function is_attribute_type_a_style
 * @brief Check if the attribute type is a style
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 * 
 */
bool is_attribute_type_a_style(ast_layout_attribute_type_t type)
{
    switch (type) {
        // general attributes for naming and styling
        case AST_LAYOUT_ATTRIBUTE_TYPE_CLASS:
        case AST_LAYOUT_ATTRIBUTE_TYPE_ID:
            return true;

        default:
            return is_style_attribute(type);
    }
}
