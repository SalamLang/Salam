from flask import Flask, render_template, request, redirect, url_for, jsonify, session
import yaml
import os

app = Flask(__name__)
app.secret_key = 'your_secret_key'

PORT = 5000
DEBUG = False
DEBUG = True
YAML_DIR = '../'
LANGUAEG_FILE = 'language.yaml'

def get_dynamic_columns(data):
    """ Extract unique keys from the YAML structure for dynamic columns """
    columns = set()
    
    for item in data.get('items', []):
        for key in item:
            columns.add(key)
    
    return list(columns)


def get_language_keys() -> list[str]:
    global LANGUAEG_FILE
    
    keys = []

    file_path = os.path.join(YAML_DIR, LANGUAEG_FILE)
    data = read_yaml(file_path)
    
    for item in data.get("items", []):
        if item.has("id"):
            keys.append(item.get("id"))
    
    return keys

    
def get_yaml_files() -> list[str]:
    """
    Scans the YAML_DIR directory and returns a list of all YAML file paths.
    
    Returns:
        list[str]: A list of file paths relative to the YAML_DIR for all .yaml files found.
    """
    files = []
    for root, _, filenames in os.walk(YAML_DIR):
        for file in filenames:
            if file.endswith('.yaml'):
                files.append(os.path.relpath(os.path.join(root, file), start=YAML_DIR))
    
    return files


def read_yaml(file_path: str) -> dict:
    """
    Reads a YAML file and returns its contents as a dictionary.
    
    Args:
        file_path (str): The path to the YAML file to read.
    
    Returns:
        dict: The contents of the YAML file.
    """
    with open(file_path, 'r', encoding='utf-8') as file:
        return yaml.safe_load(file)
    
    return None


def write_yaml(file_path: str, data: dict) -> None:
    """
    Writes data to a YAML file, overwriting the file contents.
    
    Args:
        file_path (str): The path to the YAML file to write.
        data (dict): The data to write to the file.
    
    Returns:
        None
    """
    with open(file_path, 'w', encoding='utf-8') as file:
        yaml.dump(data, file, allow_unicode=True, sort_keys=True)


@app.route('/')
def index() -> str:
    """
    Displays the main admin panel page with a list of YAML files and error messages if present.
    
    Returns:
        str: The rendered HTML template for the admin panel.
    """
    message = session.pop('message', None)
    message_type = session.pop('message_type', None)
    
    return render_template('index.html', message=message, message_type=message_type, files=get_yaml_files())


@app.route('/edit/<path:filepath>', methods=['POST'])
def edit_file_action(filepath: str) -> jsonify:
    """
    Handles editing an existing YAML file by receiving new data via POST request.
    
    Args:
        filepath (str): The relative path of the YAML file to edit.
    
    Returns:
        jsonify: JSON response indicating success or failure.
    """
    file_path = os.path.join(YAML_DIR, filepath)

    if not os.path.exists(file_path):
        return jsonify({'success': False, 'error': 'File not exists'})

    try:
        data = {'items': items}
        write_yaml(file_path, data)
        
        return jsonify({'success': True})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)})


@app.route('/edit/<path:filepath>', methods=['GET'])
def edit_file(filepath: str) -> str:
    """
    Displays the editing page for a given YAML file, showing the current contents.
    
    Args:
        filepath (str): The relative path of the YAML file to edit.
    
    Returns:
        str: The rendered HTML template for editing the YAML file.
    """
    file_path = os.path.join(YAML_DIR, filepath)

    if not os.path.exists(file_path):
        return f"File {filepath} not found.", 404

    data = read_yaml(file_path)
    columns = get_dynamic_columns(data)
    
    languages = get_language_keys()
    
    message = session.pop('message', None)
    message_type = session.pop('message_type', None)
    
    return render_template('edit.html', message=message, message_type=message_type, filename=filepath, data=data.get("items", []), columns=columns, languages=languages)


@app.route('/add-file', methods=['POST'])
def add_file_action() -> str:
    """
    Handles the action to add a new YAML file via POST request.
    
    Returns:
        str: A redirect URL back to the main admin panel with success or error messages stored in the session.
    """
    new_file = request.form.get('filename')

    if not new_file:
        session['message'] = 'Filename is required.'
        session['message_type'] = 'error'
        
        return redirect(url_for('index'))

    new_file = new_file.strip()
    
    if new_file == "":
        session['message'] = 'Filename is required.'
        session['message_type'] = 'error'
        
        return redirect(url_for('index'))        

    new_file = new_file.lstrip('/')

    if not new_file.endswith('.yaml'):
        new_file += '.yaml'

    path = os.path.join(YAML_DIR, new_file)
    
    if path.startswith(YAML_DIR + ".."):
        session['message'] = 'Invalid filename or directory traversal attempt.'
        session['message_type'] = 'error'
        
        return redirect(url_for('index'))

    if os.path.exists(path):
        session['message'] = 'File already exists.'
        session['message_type'] = 'error'
        
        return redirect(url_for('index'))

    try:
        write_yaml(path, {'items': []})

        session['message'] = 'File created.'
        session['message_type'] = 'ok'
        
        return redirect(url_for('index'))
    except Exception as e:
        session['message'] = f'Failed to create file: {e}'
        session['message_type'] = 'error'
        
        return redirect(url_for('index'))


@app.route('/delete-file/<path:filepath>', methods=['POST'])
def delete_file_action(filepath: str) -> str:
    """
    Handles the deletion of a YAML file based on the provided file path.
    
    Args:
        filepath (str): The relative path of the YAML file to delete.
    
    Returns:
        str: A redirect URL back to the main admin panel with success or error messages stored in the session.
    """
    full_path = os.path.join(YAML_DIR, filepath)
    
    if os.path.exists(full_path) and os.path.abspath(full_path).startswith(os.path.abspath(YAML_DIR)):
        try:
            os.remove(full_path)
            session['message'] = f"File '{filepath}' has been deleted successfully."
            session['message_type'] = 'ok'
        except Exception as e:
            session['message'] = f"Failed to delete file: {e}"
            session['message_type'] = 'error'
    else:
        session['message'] = f"File '{filepath}' not found or invalid path."
        session['message_type'] = 'error'

    return redirect(url_for('index'))


if __name__ == '__main__':
    os.makedirs(YAML_DIR, exist_ok=True)
    
    app.run(debug=DEBUG, port=PORT)
