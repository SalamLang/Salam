import itertools
from value import Value
from typing import Dict, List, Tuple, Any


class Values:
    @staticmethod
    def generate_ordered_strings(
        data: Dict[str, List[List[str]]],
    ) -> Dict[str, List[str]]:
        result: Dict[str, List[str]] = {}
        for lang, text_groups in data.items():
            combinations = itertools.product(*text_groups)
            result[lang] = [
                " ".join("".join(item) for item in combo) for combo in combinations
            ]
        return result

    @staticmethod
    def AndOrder(*value_groups: Tuple[Any, ...]) -> List[Value]:
        combinations = itertools.product(*value_groups)

        result: List[Value] = []
        for combo in combinations:
            combined_name: List[str] = []
            combined_text: Dict[str, List[List[str]]] = {}  # Adjusted type
            for value in combo:
                combined_name.append(value.generate_name)
                for lang, text in value.text.translations.items():
                    if lang not in combined_text:
                        combined_text[lang] = []
                    # Wrap the text into a list so we have List[List[str]] structure
                    combined_text[lang].append([text])

            combined_value = Value()
            combined_value.set_generate_name(" ".join(combined_name))

            output = Values.generate_ordered_strings(combined_text)
            for lang, strings in output.items():
                combined_value.add_text(lang, strings[0])

            result.append(combined_value)

        return result

    @staticmethod
    def And(*value_groups: Tuple[Any, ...]) -> List[Value]:
        combinations = itertools.product(*value_groups)

        result: List[Value] = []
        for combo in combinations:
            permutations = itertools.permutations(combo)

            for perm in permutations:
                combined_name: List[str] = []
                combined_text: Dict[str, List[List[str]]] = {}  # Adjusted type
                for value in perm:
                    combined_name.append(value.generate_name)
                    for lang, text in value.text.translations.items():
                        if lang not in combined_text:
                            combined_text[lang] = []
                        # Wrap the text into a list so we have List[List[str]] structure
                        combined_text[lang].append([text])

                combined_value = Value()
                combined_value.set_generate_name(" ".join(combined_name))

                output = Values.generate_ordered_strings(combined_text)
                for lang, strings in output.items():
                    combined_value.add_text(lang, strings[0])

                result.append(combined_value)

        return result
