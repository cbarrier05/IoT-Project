from flask import Flask, request, render_template

app = Flask(__name__)

pattern = 0

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update')
def update():
    return str(pattern)

@app.route('/set_pattern/<int:p>')
def set_pattern(p):
    global pattern
    pattern = p
    return "OK"

@app.route('/data')
def data():
    return {"pattern": pattern}

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)