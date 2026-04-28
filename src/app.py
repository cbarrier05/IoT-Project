from flask import Flask, request, render_template

app = Flask(__name__)

pattern = 0
temperature = "N/A"
temp_data = [0.0 for i in range(20)]

custom_frames = []
custom_delay = 200

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/update')
def update():
    global temperature
    temp = request.args.get('temp')
    if temp:
        update_graph(float(temp))
        temperature = temp
    return str(pattern)

@app.route('/set_pattern/<int:p>')
def set_pattern(p):
    global pattern
    pattern = p
    return "OK"

@app.route('/set_custom', methods=['POST'])
def set_custom():
    global custom_frames, custom_delay
    data = request.get_json()
    if data:
        custom_frames = data.get('frames', [])
        custom_delay = data.get('delay', 200)
    return "OK"

# Returns the current position of custom LEDs
@app.route('/get_custom')
def get_custom():
    custom_string = ""
    for frame in custom_frames:
        for led in frame:
            custom_string += "1" if led else "0"

    return f"{custom_delay},{custom_string}"

# Forwards the data from the web server to the web page
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

@app.route('/set_graph')
def set_graph():
    global temp_data
    time_data = [i * 2 for i in range(len(temp_data))]
    return {
        "temp_history": temp_data,
        "time_history": time_data
    }

def update_graph(t: float):
    global temp_data
    temp_data.insert(0, t)
    temp_data.pop()

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)