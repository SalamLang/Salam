// ----------- BEGIN AUTO GENERATED ----------- //
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_INCLUDE) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_PARAGRAPH_RAW) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_ERROR) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_NONE) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
        AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
        AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
        AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
        AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
        AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
        AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
        AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
        AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH,
        AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_DIV) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_PARAGRAPH) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_BUTTON) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TEXTAREA) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
        AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT,
        AST_LAYOUT_ATTRIBUTE_TYPE_PLACEHOLDER,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LABEL) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SELECT) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FIELDSET) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LEGEND) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FORM) {
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
        AST_LAYOUT_ATTRIBUTE_TYPE_TYPE,
        AST_LAYOUT_ATTRIBUTE_TYPE_KIND,
    };
    const size_t valid_attributes_length = sizeof(valid_attributes) / sizeof(valid_attributes[0]);
    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_length)) {
        return true;
    }
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_OPTGROUP) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_DATALIST) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SPAN) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_STRONG) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_EM) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_ITALIC) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_BOLD) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_UNDERLINE) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SUB) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SUP) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_S) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_HEADER) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FOOTER) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_NAV) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SECTION) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_ARTICLE) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_ASIDE) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_MAIN) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FIGURE) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FIGCAPTION) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_VIDEO) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_AUDIO) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_IFRAME) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_CANVAS) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_UL) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LINK) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_OL) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_OPTION) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LI) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE_HEADER) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE_FOOTER) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE_TR) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE_TD) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TABLE_TH) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_CAPTION) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_COL) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_BR) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_HR) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_INPUT) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_IMG) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FONT) {
}
else if (attribute->parent_node_type == AST_LAYOUT_TYPE_MEDIA) {
}
// ----------- END AUTO GENERATED ----------- //











