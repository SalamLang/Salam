import yaml


def convert_yaml(input_file, output_file):
    with open(input_file, "r", encoding="utf-8") as file:
        data = yaml.safe_load(file)

    result = []

    for key, items in data.get("items", {}).items():
        print(key)
        print(items)
        childrens = {"id": key, "childrens": []}
        if items is not None:
            for item in items:
                childrens["childrens"].append(item)

        result.append(childrens)

    with open(output_file, "w", encoding="utf-8") as file:
        yaml.dump(result, file, allow_unicode=True, sort_keys=False)

    print(f"Converted YAML saved to {output_file}")


input_yaml = "value.yaml"
output_yaml = "value.yaml"

convert_yaml(input_yaml, output_yaml)
