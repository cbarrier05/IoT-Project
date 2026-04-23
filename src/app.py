from flask import Flask, request, render_template

app = Flask(__name__)

pattern = 0
temperature = 0
custom = []

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update')
def update():
    global temperature
    temp = request.args.get('temp')
    if temp:
        temperature = temp
    return str(pattern)

@app.route('/set_pattern/<int:p>')
def set_pattern(p):
    global pattern
    pattern = p
    return "OK"

@app.route('/set_custom/<int:c>')
def set_custom(c):
    global custom
    if c in custom:
        custom.remove(c)
    else:
        custom.append(c)
    return "OK"

@app.route('/get_custom')
def get_custom():
    custom_string = ""
    for led in custom:
        custom_string += str(led)
    return custom_string

@app.route('/data')
def data():
    match pattern:
        case 0:
            pattern_name = "Blink"
        case 1:
            pattern_name = "Chase"
        case 2:
            pattern_name = "Random"
        case 3:
            pattern_name = "Rainbow"
        case 4:
            pattern_name = "Custom"
        case _:
            pattern_name = "N/A"
    return {"temperature": temperature, "pattern": pattern_name}

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)