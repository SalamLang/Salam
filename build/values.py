import itertools
from value import Value

class Values:
    @staticmethod
    def And(*value_groups):
        """
        Combines arrays of Value objects to generate new Value objects for all combinations.

        Args:
            value_groups: Lists of Value objects to combine.

        Returns:
            A list of Value objects representing all combinations.
        """

        combinations = itertools.product(*value_groups)

        result = []
        for combo in combinations:
            permutations = itertools.permutations(combo)

            for perm in permutations:
                combined_name = []
                for value in perm:
                    combined_name.append(value.generate_name)

                print(combined_name)

                print(combo)
                combined_value = Value()
                combined_value.set_generate_name(" ".join(combined_name))

                print(combined_value.generate_name)
                result.append(combined_value)

        return result