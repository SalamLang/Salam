from flask import Flask, render_template, request, redirect, url_for, jsonify, session
import yaml
import os

app = Flask(__name__)
app.secret_key = 'your_secret_key'

YAML_DIR = '../'
YAML_FILES = []
for root, dirs, files in os.walk(YAML_DIR):
    for file in files:
        if file.endswith('.yaml'):
            YAML_FILES.append(os.path.relpath(os.path.join(root, file), start=YAML_DIR))


def read_yaml(file_path):
    with open(file_path, 'r', encoding='utf-8') as file:
        return yaml.safe_load(file)


def write_yaml(file_path, data):
    with open(file_path, 'w', encoding='utf-8') as file:
        yaml.dump(data, file, allow_unicode=True, sort_keys=True)


@app.route('/')
def index():
    return render_template('index.html', files=YAML_FILES)


@app.route('/edit/<path:filepath>', methods=['GET', 'POST'])
def edit_file(filepath):
    file_path = os.path.join(YAML_DIR, filepath)

    if not os.path.exists(file_path):
        return f"File {filepath} not found.", 404

    if request.method == 'POST':
        items = request.json.get('items', [])
        data = {'items': items}
        write_yaml(file_path, data)
        
        return jsonify({'success': True})
    
    data = read_yaml(file_path)
    return render_template('edit.html', filename=filepath, items=data['items'])


@app.route('/add-file', methods=['POST'])
def add_file():
    new_file = request.form.get('filename')

    if not new_file:
        session['error'] = 'Filename is required.'
        return redirect(url_for('index'))

    new_file = new_file.strip()
    
    if new_file == "":
        session['error'] = 'Filename is required.'
        return redirect(url_for('index'))        

    new_file = new_file.lstrip('/')

    if not new_file.endswith('.yaml'):
        new_file += '.yaml'

    path = os.path.join(YAML_DIR, new_file)

    if not path.startswith(os.path.abspath(YAML_DIR)):
        session['error'] = 'Invalid filename or directory traversal attempt.'
        return redirect(url_for('index'))

    if os.path.exists(path):
        session['error'] = 'File already exists.'
        return redirect(url_for('index'))

    try:
        write_yaml(path, {'items': []})

        return redirect(url_for('index'), code=200)
    except Exception as e:
        session['error'] = f'Failed to create file: {e}'
        
        return redirect(url_for('index'), code=302)


@app.route('/delete-file/<path:filepath>', methods=['POST'])
def delete_file(filepath):
    full_path = os.path.join(YAML_DIR, filepath)

    if os.path.exists(full_path) and os.path.abspath(full_path).startswith(os.path.abspath(YAML_DIR)):
        try:
            os.remove(full_path)
            session['error'] = f"File '{filepath}' has been deleted successfully."
        except Exception as e:
            session['error'] = f"Failed to delete file: {e}"
    else:
        session['error'] = f"File '{filepath}' not found or invalid path."

    return redirect(url_for('index'))


if __name__ == '__main__':
    os.makedirs(YAML_DIR, exist_ok=True)
    app.run(debug=True, port=5000)
