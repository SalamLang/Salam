def process_file(input_file, output_file):
    semi_space_char = '\u200c'

    with open(input_file, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    new_lines = []
    for line in lines:
        new_lines.append(line)
        if semi_space_char in line:
            modified_line = line
            modified_line = modified_line.replace("ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(", "ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(")
            modified_line = modified_line.replace(semi_space_char, " ")
            new_lines.append(modified_line)

    with open(output_file, 'w', encoding='utf-8') as file:
        file.writelines(new_lines)

input_file = 'ast_layout_attribute_style_type.h'
output_file = 'output.txt'

process_file(input_file, output_file)
