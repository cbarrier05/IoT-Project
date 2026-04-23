from flask import Flask, request, render_template

app = Flask(__name__)

set_led = 0

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update')
def update():
    return str(set_led)

@app.route('/toggle_LED/<int:led>')
def toggle_LED(led):
    global set_led
    set_led = led
    return "OK"

@app.route('/data')
def data():
    return {"led": set_led}

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)