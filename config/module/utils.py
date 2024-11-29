from typing import Dict, List, Any
import yaml

def load_yaml(file_path: str) -> List[Dict[str, Any]]:
    """
    Loads a YAML file and returns its contents as a list of dictionaries.
    Args:
        file_path (str): Path to the YAML file.
    Returns:
        List[Dict[str, Any]]: Parsed contents of the YAML file.
    """
    try:
        with open(file_path, "r", encoding="utf-8") as file:
            return list(yaml.safe_load_all(file))
    except FileNotFoundError:
        error(f"File not found: {file_path}")
    except yaml.YAMLError as e:
        error(f"Error parsing YAML file: {file_path}\n{e}")

def error(msg: str) -> None:
    """
    Prints an error message and exits the program.
    Args:
        msg (str): The error message to display.
    """
    print("Error: " + msg)
    exit(1)
